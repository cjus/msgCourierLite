#include <stdio.h>
#include "base64.h"

char cBase64::base64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

cBase64::cBase64()
: m_status(HRC_BASE64_OK)
{
}

cBase64::~cBase64()
{
}

int cBase64::Decode(cBuffer *pBuffer)
{
    int in_len = pBuffer->GetBufferCount();
    int i = 0;
    int j = 0;
    int in_ = 0;
    int cp = 0;
    unsigned char char_array_4[4], char_array_3[3];
    unsigned char *encoded_string = (unsigned char*)pBuffer->GetRawBuffer();

    m_InternalBuffer.Reset();
    m_InternalBuffer.SetBufferSize(in_len);
    unsigned char* buffer = (unsigned char*)m_InternalBuffer.GetRawBuffer();

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i ==4 )
        {
            for (i = 0; i <4; i++)
                char_array_4[i] = find(char_array_4[i]);
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            {
                buffer[cp] = char_array_3[i];
                cp++;
            }
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;
        for (j = 0; j <4; j++)
            char_array_4[j] = find(char_array_4[j]);
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; (j < i - 1); j++)
        {
            buffer[cp] = char_array_3[j];
            cp++;
        }
    }
    pBuffer->ReplaceWith((char*)buffer, cp);
    return (m_status = HRC_BASE64_OK);
}


int cBase64::Encode(cBuffer *pBuffer)
{
    unsigned char const* bytes_to_encode = (unsigned char const*)pBuffer->GetRawBuffer();
    unsigned int in_len = pBuffer->GetBufferCount();

    m_InternalBuffer.Reset();
    m_InternalBuffer.SetBufferSize(in_len*2);
    unsigned char* buffer = (unsigned char*)m_InternalBuffer.GetRawBuffer();

    int i = 0;
    int j = 0;
    int cp = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4 ; i++)
            {
                buffer[cp] = base64[char_array_4[i]];
                cp++;
            }
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++)
        {
            buffer[cp] = base64[char_array_4[j]];
            cp++;
        }
        while ((i++ < 3))
        {
            buffer[cp] = '=';
            cp++;
        }
    }

    pBuffer->ReplaceWith((char*)buffer, cp);
    return (m_status = HRC_BASE64_OK);
}

