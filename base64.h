#ifndef _BASE64_H
#define _BASE64_H

#include <ctype.h>
#include "buffer.h"

#define  HRC_BASE64_OK					0x0000
#define  HRC_BASE64_DECODE_FAILED		0x0001
#define  HRC_BASE64_MALLOC_FAILED		0x0002
#define  HRC_BASE64_INVALID_PARAM		0x0003

class cBase64
{
public:
	cBase64();
	~cBase64();

	int Decode(cBuffer *pBuffer);
	int Encode(cBuffer *pBuffer);

	int GetStatusCode() { return m_status; }
private:
	int m_status;
	cBuffer m_InternalBuffer;
    static char base64[65];

    inline bool is_base64(unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    inline int find(unsigned char c)
    {
        for (int i=0; i<65; i++)
        {
            if (base64[i] == c)
                return i;
        }
        return -1;
    }
};

#endif // _BASE64_H

/* TEST CASES
//http://www.adp-gmbh.ch/cpp/common/base64.html
//http://rumkin.com/tools/compression/base64.php
    cBuffer buf;
    cBase64 b64;

    //buf.ReplaceWith("y"); // eQ==
    //buf.ReplaceWith("cluster"); // Y2x1c3Rlcg==
    //buf.ReplaceWith("help"); // aGVscA==
    //buf.ReplaceWith("WtF.,?%^&*@#$!<>|{}[]`~-=+_\"':"); // V3RGLiw/JV4mKkAjJCE8Pnx7fVtdYH4tPStfIic6
    //buf.ReplaceWith("<html><body></body></html>"); // PGh0bWw+PGJvZHk+PC9ib2R5PjwvaHRtbD4=
    //buf.ReplaceWith("Be0wu1f!"); // QmUwd3UxZiE=
    //buf.ReplaceWith("The quick brown fox jumped over the lazy brown dogs."); // VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wZWQgb3ZlciB0aGUgbGF6eSBicm93biBkb2dzLg==

    //buf.ReplaceWith(""eQ==");
    //buf.ReplaceWith("Y2x1c3Rlcg==");
    //buf.ReplaceWith("aGVscA==");
    //buf.ReplaceWith("V3RGLiw/JV4mKkAjJCE8Pnx7fVtdYH4tPStfIic6");
    //buf.ReplaceWith("PGh0bWw+PGJvZHk+PC9ib2R5PjwvaHRtbD4=");
    //buf.ReplaceWith("QmUwd3UxZiE=");
    //buf.ReplaceWith("VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wZWQgb3ZlciB0aGUgbGF6eSBicm93biBkb2dzLg==");
    printf("[%s]\n", buf.c_str());
    //b64.Encode(&buf);
    b64.Decode(&buf);
    printf("[%s]\n", buf.c_str());
    abort();
*/
