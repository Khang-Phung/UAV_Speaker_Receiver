#include "Jitter.hpp"

void JitterBuffer::init(int startPackets)
{
    // mutex lock

    std::lock_guard<std::mutex> lock(jbMutex);

    _count = 0;
    _started = false;

    _startThreshold = startPackets;

    _expectedSeq = 0;
}

int JitterBuffer::push(
    const uint8_t *data,
    int len)
{
    // mutex lock
    std::lock_guard<std::mutex> lock(jbMutex);

    if (data == nullptr)
        return false;

    if (len < static_cast<int>(sizeof(uint32_t)))
        return false;

    if (len > JB_MAX_PACKET_SIZE)
        return false;

    // =========================================
    // First 4 bytes = sequence number
    // =========================================
    uint32_t seq;
    uint8_t type;
    std::memcpy(
        &seq,
        data,
        sizeof(seq));
    
    std::memcpy(
        &type,
        data + 11,
        sizeof(type)
    );

    if (type)
    {
        return 2;
    }


    // =========================================
    // Sender restart
    // =========================================
    if (
        _started &&
        seq < _expectedSeq)
    {
        if (seq == 0)
        {
            std::printf(
                "Sender restart\n");

            // Clear jitter buffer
            std::memset(
                _packetBuffer,
                0,
                sizeof(_packetBuffer));

            // Reset state
            _count = 0;

            _started = false;

            _expectedSeq = 0;
        }
        else
        {
            // Packet cu
            std::printf(
                "Old packet seq=%u\n",
                seq);

            return false;
        }
    }

    // =========================================
    // RESYNC
    //
    // Packet moi di qua xa expectedSeq
    // =========================================
    if (_started && seq > _expectedSeq)
    {
        uint32_t gap =
            seq - _expectedSeq;

        if (gap > JB_SIZE)
        {
            std::printf(
                "JB RESYNC: expected=%u, "
                "new seq=%u, gap=%u\n",
                _expectedSeq,
                seq,
                gap);

            // Clear toàn bộ buffer
            std::memset(
                _packetBuffer,
                0,
                sizeof(_packetBuffer));

            // Reset buffer count
            _count = 0;

            // Resync expected sequence
            _expectedSeq = seq;

            // Restart trạng thái started
            _started = false;
        }
    }

    // =========================================
    // Jitter Buffer đầy
    // =========================================
    if (_count >= JB_SIZE)
    {
        std::printf(
            "JB full, drop seq=%u\n",
            seq);

        return false;
    }

    // =========================================
    // Tìm vị trí chèn
    // =========================================
    int pos = _count;

    while (
        pos > 0 &&
        _packetBuffer[pos - 1].seq > seq)
    {
        _packetBuffer[pos] =
            _packetBuffer[pos - 1];

        pos--;
    }

    // =========================================
    // Packet trùng
    // =========================================
    if (
        pos < _count &&
        _packetBuffer[pos].seq == seq)
    {
        std::printf(
            "Duplicate seq=%u\n",
            seq);

        return false;
    }

    // =========================================
    // Insert packet
    // =========================================
    _packetBuffer[pos].seq =
        seq;

    _packetBuffer[pos].len =
        len;

    std::memcpy(
        _packetBuffer[pos].data,
        data,
        len);

    _count++;

    // =========================================
    // Đủ packet để bắt đầu phát
    // =========================================
    if (
        !_started &&
        _count >= _startThreshold)
    {
        _started = true;

        _expectedSeq =
            _packetBuffer[0].seq;

        std::printf(
            "Jitter started, "
            "expectedSeq=%u\n",
            _expectedSeq);
    }

    return true;
}

int JitterBuffer::pop(
    JitterPacket *pkt)
{
    // =========================================
    // Return 0: mute âm thanh
    // Return 1: giải mã âm thanh gốc
    // Return 2: giải mã plc
    // =========================================

    std::lock_guard<std::mutex> lock(jbMutex);

    if (pkt == nullptr)
        return 0;

    // =========================================
    // Chua du packet de bat dau
    // =========================================
    if (!_started)
    {

        return 0;
    }
    // =========================================
    // Buffer khong co packet
    // =========================================
    if (_count == 0)
    {
        std::printf(
            "Empty buffer\n");

        return 0;
    }

    // =========================================
    // Packet dung thu tu
    // =========================================
    if (
        _packetBuffer[0].seq ==
        _expectedSeq)
    {
        *pkt =
            _packetBuffer[0];

        // Shift buffer
        for (
            int i = 1;
            i < _count;
            i++)
        {
            _packetBuffer[i - 1] =
                _packetBuffer[i];
        }

        _count--;

        _expectedSeq++;

        return 1;
    }

    // =========================================
    // Packet dau tien lon hon expectedSeq
    // =========================================
    if (
        _packetBuffer[0].seq >
        _expectedSeq)
    {
        uint32_t gap =
            _packetBuffer[0].seq -
            _expectedSeq;

        // =====================================
        // khoang cach qua lon
        // => Bo qua cac packet cu
        // => Phat packet hien tai
        // =====================================
        if (gap > JB_SIZE - 20)
        {
            uint32_t currentSeq =
                _packetBuffer[0].seq;

            std::printf(
                "Large gap: expected=%u, "
                "got=%u, gap=%u -> skip\n",
                _expectedSeq,
                currentSeq,
                gap);

            // Tra packet hien tai
            *pkt =
                _packetBuffer[0];

            // Shift buffer
            for (
                int i = 1;
                i < _count;
                i++)
            {
                _packetBuffer[i - 1] =
                    _packetBuffer[i];
            }

            _count--;

            // Dong bo sequence tiep theo
            _expectedSeq =
                currentSeq + 1;

            return 1;
        }

        // =====================================
        // Khoang cach nho
        // => Packet bi mat
        // => Dung PLC
        // =====================================
        std::printf(
            "Missing packet seq=%u\n",
            _expectedSeq);

        _expectedSeq++;

        // AudioTask se dung PLC
        return 2;
    }

    return 0;
}

int JitterBuffer:: getCount()
{
        return _count;
}