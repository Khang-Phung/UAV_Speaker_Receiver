#include "Receiver.hpp"
// receive UDP + decrypt AES + jitter push
void Receiver::receiveTask()
{
    while (true)
    {
        while (true)
        {
            // Receive UDP packet
            int len = udp.receive(UDP_BUFFER_SIZE);

            // Timeout or error
            if (len <= 0)
            {
                continue;
            }

            // AES decrypt
            bool decryptOk = aes.decrypt(
                udp.getBuffer(),
                len);

            if (!decryptOk)
            {
                continue;
            }

            // Push decrypted packet
            int pushOk = jitterBuffer.push(
                aes.getBuffer(),
                aes.getLen());

            if (pushOk == 2)
                udp.setAlive();
        }
    }
}

//  jitter pop + decode Opus + UART send
void Receiver::audioTask()
{
    // Run every 20 ms
    auto nextWake =
        std::chrono::steady_clock::now();

    static int64_t lastPeriod = 0;

    JitterPacket pkt;

    int samples = 0;

    while (true)
    {
        // =================================
        // Wait until next 20 ms period
        // =================================

        static int intervalMs = 20;

        int count = jitterBuffer.getCount();

        if (count < 17)
        {
            intervalMs = 21;
        }
        else if (count > 20)
        {
            intervalMs = 20;
        }

        nextWake += std::chrono::milliseconds(intervalMs);

        std::this_thread::sleep_until(nextWake);

        // =================================
        // Period timing
        // =================================

        auto now =
            std::chrono::steady_clock::now();

        int64_t now_us =
            std::chrono::duration_cast<
                std::chrono::microseconds>(
                now.time_since_epoch())
                .count();

        if (lastPeriod != 0)
        {
            // printf(
            //     "period=%ld us\n",
            //     now_us - lastPeriod
            // );
        }

        lastPeriod = now_us;

        // =================================
        // Jitter Pop
        // =================================

        int popResult =
            jitterBuffer.pop(&pkt);

        // =================================
        // Opus Decode
        // =================================

        if (popResult == 1)
        {
            // Packet received
            //
            // pkt.data:
            //
            // [4-byte seq]
            // [8-byte timestamp]
            // [Opus]

            samples =
                opus.decode(
                    pkt.data + 12,
                    pkt.len - 12);
        }
        else if (popResult == 2)
        {
            // Packet lost
            // Use Opus PLC

            samples =
                opus.decode(
                    nullptr,
                    0);
        }
        else
        {
            // Empty + Reset
            // Play silence

            std::memset(
                opus.getBuffer(),
                0,
                opus.getBufferSize());

            samples = FRAME_SIZE;
        }

        if (samples > 0)
        {

            // Giam am luong con 50%
            opus.low_volume();

            // dem byte
            uint16_t pcmBytes =
                samples * sizeof(int16_t);
            // ghep cac byte chua thong tin gia tri seq
            uint32_t pcmSeq =

                static_cast<uint32_t>(pkt.data[0]) |
                (static_cast<uint32_t>(pkt.data[1]) << 8) |
                (static_cast<uint32_t>(pkt.data[2]) << 16) |
                (static_cast<uint32_t>(pkt.data[3]) << 24);

            // Truyen UART
            uart.uartSend(
                FRAME_TYPE_AUDIO,
                pcmSeq,
                reinterpret_cast<const uint8_t *>(opus.getBuffer()),
                pcmBytes);
        }
       // std::cout << "    Count: " << jitterBuffer.getCount() << "\n";
    }
}

void Receiver::tcpTask()
{
    std::cout << "TCP task started\n";

    auto lastHeartbeat =
        std::chrono::steady_clock::now();

    while (true)
    {
        // =========================
        // Accept TCP client
        // =========================

        if (!tcp.isConnected())
        {
            std::cout << "Waiting for TCP client...\n";

            if (!tcp.acceptClient())
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100)
                );

                continue;
            }

            std::cout << "TCP client connected\n";
        }

        // =========================
        // Receive TCP
        // =========================

        int len = tcp.receive(128);

        if (len < 0)
        {
            std::cout << "TCP client disconnected\n";
            continue;
        }

        if (len > 0)
        {
            const uint8_t* data =
                tcp.getBuffer();

            std::cout
                << "TCP RX: ["
                << std::string(
                       reinterpret_cast<const char*>(data),
                       len)
                << "]\n";
        }

        // =========================
        // Heartbeat
        // =========================

        auto now =
            std::chrono::steady_clock::now();

        if (now - lastHeartbeat >=
            std::chrono::seconds(2))
        {
            lastHeartbeat = now;

const bool uartConnected =
    uart.isConnected();

const char* heartbeat =
    uartConnected
        ? "HEARTBEAT,UART=1\n"
        : "HEARTBEAT,UART=0\n";

bool success = tcp.send(
    reinterpret_cast<const uint8_t*>(heartbeat),
    strlen(heartbeat)
);

printf(
    "Heartbeat: UART=%d, TCP_SEND=%d\n",
    uartConnected,
    success
);
        }

        // =========================
        // Avoid CPU 100%
        // =========================

        std::this_thread::sleep_for(
            std::chrono::milliseconds(1)
        );
    }
}

void Receiver::start()
{

    std::cout << "    UAV Speaker Receiver\n";
    std::cout << "---------------------------\n";
    udp.init(UDP_PORT);
    tcp.init(TCP_PORT);
    opus.init();
    aes.init();
    jitterBuffer.init(JB_THRESHOLD);
    std::cout << "IP Address (Sender chọn IP VPN):\n";
    system("hostname -I | awk '{for(i=1;i<=NF;i++) if ($i ~ /^[0-9]+\\./) print \"- \" $i}'");
        if (uart.init())
    {
        printf(
            "UART initialized: %s\n",
            "Connected!!!");
    }
    std::cout << "---------------------------\n";
    std::cout << "WAITING FOR SENDER START\n";
    udpThread = std::thread(&Receiver::receiveTask, this);
    audioThread = std::thread(&Receiver::audioTask, this);
    tcpThread = std::thread(&Receiver::tcpTask, this);
    udpThread.join();
    audioThread.join();
    tcpThread.join();
}