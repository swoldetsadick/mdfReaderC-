/*********************************************************************************************
  Copyright 2011 Michael Bührer & Bernd Sparrer. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice, this list of
        conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright notice, this list
        of conditions and the following disclaimer in the documentation and/or other materials
        provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Michael Bührer  & Bernd Sparrer ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Michael Bührer OR Bernd Sparrer
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed
  or implied, of Michael Bührer & Bernd Sparrer.
***********************************************************************************************/
#include "stdafx.h"
#include "md5.h"

#if _MDF4_WIN
void *mymemcpy(void *pDst, const void *pSrc, size_t nSize);
#else
#define mymemcpy memcpy
#endif

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
MD5Context::MD5Context()
{
  buf[0] = 0x67452301;
  buf[1] = 0xefcdab89;
  buf[2] = 0x98badcfe;
  buf[3] = 0x10325476;
  
  bits[0] = 0;
  bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Context::Update(unsigned char *buf, unsigned long len)
{
    unsigned long t;

    /* Update bitcount */
    t = bits[0];
    if ((bits[0] = t + (len << 3)) < t)
	    bits[1]++; 	/* Carry from low to high */
    bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) 
    {
	    unsigned char *p = (unsigned char *) in + t;
      t = 64 - t;
      if (len < t) 
      {
	       mymemcpy(p, buf, len);
	       return;
	    }
	    mymemcpy(p, buf, t);
      Transform();
      buf += t;
      len -= t;
    }
    /* Process data in 64-byte chunks */
    while (len >= 64) 
    {
      mymemcpy(in, buf, 64);
      Transform();
      buf += 64;
      len -= 64;
    }

    /* Handle any remaining bytes of data. */
    mymemcpy(in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Context::Final(unsigned char *digest)
{
  unsigned long count;
  unsigned char *p;

  /* Compute number of bytes mod 64 */
  count = (bits[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
  p = in + count;
  *p++ = 0x80;

  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;

  /* Pad out to 56 mod 64 */
  if (count < 8) 
  {
    /* Two lots of padding:  Pad the first block to 64 bytes */
    memset(p, 0, count);
    Transform();

    /* Now fill the next block with 56 bytes */
    memset(in, 0, 56);
  }
  else
  {
    /* Pad block to 56 bytes */
    memset(p, 0, count - 8);
  }
  /* Append length in bits and transform */
  ((unsigned long *)in)[14] = bits[0];
  ((unsigned long *)in)[15] = bits[1];

   Transform();
   mymemcpy(digest, buf, 16);
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD5Context::Transform()
{
  unsigned long *_in=(unsigned long *)in;
  register unsigned long a, b, c, d;

  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];

  MD5STEP(F1, a, b, c, d, _in[0] + 0xd76aa478, 7);
  MD5STEP(F1, d, a, b, c, _in[1] + 0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, _in[2] + 0x242070db, 17);
  MD5STEP(F1, b, c, d, a, _in[3] + 0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, _in[4] + 0xf57c0faf, 7);
  MD5STEP(F1, d, a, b, c, _in[5] + 0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, _in[6] + 0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, _in[7] + 0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, _in[8] + 0x698098d8, 7);
  MD5STEP(F1, d, a, b, c, _in[9] + 0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, _in[10] + 0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, _in[11] + 0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, _in[12] + 0x6b901122, 7);
  MD5STEP(F1, d, a, b, c, _in[13] + 0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, _in[14] + 0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, _in[15] + 0x49b40821, 22);

  MD5STEP(F2, a, b, c, d, _in[1] + 0xf61e2562, 5);
  MD5STEP(F2, d, a, b, c, _in[6] + 0xc040b340, 9);
  MD5STEP(F2, c, d, a, b, _in[11] + 0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, _in[0] + 0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, _in[5] + 0xd62f105d, 5);
  MD5STEP(F2, d, a, b, c, _in[10] + 0x02441453, 9);
  MD5STEP(F2, c, d, a, b, _in[15] + 0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, _in[4] + 0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, _in[9] + 0x21e1cde6, 5);
  MD5STEP(F2, d, a, b, c, _in[14] + 0xc33707d6, 9);
  MD5STEP(F2, c, d, a, b, _in[3] + 0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, _in[8] + 0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, _in[13] + 0xa9e3e905, 5);
  MD5STEP(F2, d, a, b, c, _in[2] + 0xfcefa3f8, 9);
  MD5STEP(F2, c, d, a, b, _in[7] + 0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, _in[12] + 0x8d2a4c8a, 20);

  MD5STEP(F3, a, b, c, d, _in[5] + 0xfffa3942, 4);
  MD5STEP(F3, d, a, b, c, _in[8] + 0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, _in[11] + 0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, _in[14] + 0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, _in[1] + 0xa4beea44, 4);
  MD5STEP(F3, d, a, b, c, _in[4] + 0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, _in[7] + 0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, _in[10] + 0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, _in[13] + 0x289b7ec6, 4);
  MD5STEP(F3, d, a, b, c, _in[0] + 0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, _in[3] + 0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, _in[6] + 0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, _in[9] + 0xd9d4d039, 4);
  MD5STEP(F3, d, a, b, c, _in[12] + 0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, _in[15] + 0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, _in[2] + 0xc4ac5665, 23);

  MD5STEP(F4, a, b, c, d, _in[0] + 0xf4292244, 6);
  MD5STEP(F4, d, a, b, c, _in[7] + 0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, _in[14] + 0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, _in[5] + 0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, _in[12] + 0x655b59c3, 6);
  MD5STEP(F4, d, a, b, c, _in[3] + 0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, _in[10] + 0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, _in[1] + 0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, _in[8] + 0x6fa87e4f, 6);
  MD5STEP(F4, d, a, b, c, _in[15] + 0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, _in[6] + 0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, _in[13] + 0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, _in[4] + 0xf7537e82, 6);
  MD5STEP(F4, d, a, b, c, _in[11] + 0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, _in[2] + 0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, _in[9] + 0xeb86d391, 21);

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
