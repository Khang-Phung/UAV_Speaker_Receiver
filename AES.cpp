#include "AES.hpp"
#include <openssl/evp.h>

static EVP_CIPHER_CTX* aesCtx = nullptr;

bool AES::init()
{
    aesCtx = EVP_CIPHER_CTX_new();

    if (aesCtx == nullptr)
    {
        printf("AES context creation failed\n");
        return false;
    }

    return true;
}

bool AES::decrypt(uint8_t* data, int len)
{
    if (data == nullptr )
    {
        return false;
    }

    if (aesCtx == nullptr)
    {
        printf("AES is not initialized\n");
        return false;
    }

    if (len < AES_NONCE_SIZE + AES_TAG_SIZE)
    {
        printf("Invalid AES packet\n");
        return false;
    }

    const uint8_t* nonce =
        data;

    const uint8_t* cipher =
        data + AES_NONCE_SIZE;

    int cipherLen =
        len - AES_NONCE_SIZE - AES_TAG_SIZE;

    const uint8_t* tag =
        data + len - AES_TAG_SIZE;

    if (EVP_CIPHER_CTX_reset(aesCtx) != 1)
    {
        printf("AES context reset failed\n");
        return false;
    }

    if (EVP_DecryptInit_ex(
            aesCtx,
            EVP_aes_256_gcm(),
            nullptr,
            AES_KEY,
            nonce) != 1)
    {
        printf("AES decrypt init failed\n");
        return false;
    }

    int outLen = 0;
    int finalLen = 0;

    if (EVP_DecryptUpdate(
            aesCtx,
            _plainBuffer,
            &outLen,
            cipher,
            cipherLen) != 1)
    {
        printf("AES decrypt failed\n");
        return false;
    }

    if (EVP_CIPHER_CTX_ctrl(
            aesCtx,
            EVP_CTRL_GCM_SET_TAG,
            AES_TAG_SIZE,
            const_cast<uint8_t*>(tag)) != 1)
    {
        printf("AES set tag failed\n");
        return false;
    }

    int ret = EVP_DecryptFinal_ex(
        aesCtx,
        _plainBuffer + outLen,
        &finalLen
    );

    if (ret != 1)
    {
        printf("AES authentication failed\n");
        return false;
    }

    plainLen = outLen + finalLen;

    return true;
}

uint8_t* AES::getBuffer()
{
    return _plainBuffer;
}

int AES::getLen()
{
    return plainLen;
}
