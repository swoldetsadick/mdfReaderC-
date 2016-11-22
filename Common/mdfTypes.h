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
#pragma once

#include "mdfConfig.h"
#ifndef WIN32
#define _STDINT
#endif

// type definition for MDF4 data types
#ifndef _STDINT
typedef unsigned char  M_BYTE;    // used for "raw data" (e.g. reserved fields)
typedef unsigned char  M_UINT8;   // used for small (0-255) integer units
typedef unsigned short M_UINT16;  // used for medium (0-32767) integer units
typedef signed short   M_INT16;   // used for signed medium (0-32767) integer units
typedef unsigned long  M_UINT32;  // used for normal (32-bit) integer units
typedef signed long    M_INT32;   // used for signed normal (32-bit) integer units
typedef unsigned __int64 M_UINT64;  // used for quantities>2 GByte
typedef __int64        M_INT64;   // used for quantities>2 GByte
typedef double         M_REAL;    // used for floating point numbers
typedef char           M_CHAR;    // used in ID-block only: ASCII chars
#else
#include <stdint.h>
typedef uint8_t  M_BYTE;    // used for "raw data" (e.g. reserved fields)
typedef uint8_t  M_UINT8;   // used for small (0-255) integer units
typedef uint16_t M_UINT16;  // used for medium (0-32767) integer units
typedef int16_t  M_INT16;   // used for signed medium (0-32767) integer units
typedef uint32_t M_UINT32;  // used for normal (32-bit) integer units
typedef int32_t  M_INT32;   // used for normal (32-bit) integer units
typedef uint64_t M_UINT64;  // used for quantities>2 GByte
typedef int64_t  M_INT64;   // used for quantities>2 GByte
typedef double   M_REAL;    // used for floating point numbers
typedef char     M_CHAR;    // used in ID-block only: ASCII chars
#endif

// MDF 3.xx
typedef M_UINT16      M3SIZE;     // MDF3.xx max. size of a block
typedef M_UINT32      M3LINK;     // MDF3.xx address of block in file

// MDF 4.00
typedef M_UINT64       M_LINK;    // .. like links /size/count etc. (MDF4 only)
typedef M_UINT64       M_SIZE;    //    (MDF4 only)

// UNICODE and UTF8 pointer
typedef const char    *M_UTF8;
typedef const wchar_t *M_UNICODE;
#ifdef _UNICODE
typedef M_UNICODE      M_FILENAME;
#else 
#ifdef WIN32
typedef LPCTSTR        M_FILENAME;
#else
typedef const char    *M_FILENAME;
#endif
#endif

#ifndef WIN32
#define _T(s) s
#endif

#ifndef PVOID
typedef void *PVOID;
#endif
#ifndef BOOL
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#endif
#ifndef WIN32
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (-1)
#endif
#endif


#ifndef BIT
#define BIT(x) (1<<(x))
#define BIT0   BIT(0)
#define BIT1   BIT(1)
#define BIT2   BIT(2)
#define BIT3   BIT(3)
#define BIT4   BIT(4)
#define BIT5   BIT(5)
#define BIT6   BIT(6)
#define BIT7   BIT(7)
#define BIT8   BIT(8)
#define BIT9   BIT(9)
#define BIT10  BIT(10)
#define BIT11  BIT(11)
#define BIT12  BIT(12)
#define BIT13  BIT(13)
#define BIT14  BIT(14)
#define BIT15  BIT(15)
#endif

#define MDF_ID(a,b) (((b)<<8)|(a))  // 2-char Block Header

#pragma pack( push, 1)  // Enter BYTE packing

//-------------------------------------------------------------------------------------------------------
// The IDBLOCK always begins at file position 0 and has a constant length of 64 Bytes.
// It contains information to identify the file. This includes information about the source of the file
// and general format specifications. 
// To be compliant with older MDF formats in this section each CHAR must be a 1-Byte ASCII character.
// The IDBLOCK is the only block without a Header section and without a Link section.
//-------------------------------------------------------------------------------------------------------

struct mdfFileId
{
  M_CHAR   id_file[8];  // File identifier always contains “MDF “. (“MDF” followed by five spaces, no zero termination!).
  M_CHAR   id_vers[8];  // Format identifier, a textual representation of the format version for display, e.g. “4.00 ” (no zero termination required).
  M_CHAR   id_prog[8];  // Program identifier, to identify the program which generated the MDF file (no zero termination required). This program identifier serves only for compatibility with previous MDF format versions. Detailed information about the generating application must be written to the first FHBLOCK referenced by the HDBLOCK. As a recommendation, the program identifier inserted into the 8 characters should be the base name (first 8 characters) of the EXE/DLL of the writing application. Alternatively, also version information of the application can be appended (e.g. "MyApp45" for version 4.5 of MyApp.exe).
  M_UINT16 id_order;    // Byte Order: 0=Intel (MDF4: always 0, MDF3: only 0 supported)
  M_UINT16 id_float;    // Float Format: 0=IEEE 754 (MDF4: always 0, MDF3: only 0 supported)
  M_UINT16 id_ver;      // Version number of the MDF format, i.e. 400 for this version
  M_BYTE   id_res2[34]; // Reserved
};

// structure for CANOpenDATE
typedef struct
{
  M_UINT16 ms;   // Bit 0 .. Bit 15: Milliseconds (0 .. 59999)
  M_UINT8  min;  // Bit 0 .. Bit 5: Minutes (0 .. 59) Bit 6 .. Bit 7: Reserved
  M_UINT8  hour; // Bit 0 .. Bit 4: Hours (0 .. 23) Bit 5 .. Bit 6: Reserved Bit 7: 0 = Standard time, 1 = Summer time
  M_UINT8  day;  // Bit 0 .. Bit 4: Day (1 .. 31) Bit 5 .. Bit 7: Day of week (1 = Monday ... 7 = Sunday)
  M_UINT8  month; // Bit 0 .. Bit 5: Month (1 = January .. 12 = December) Bit 6 .. Bit 7: Reserved
  M_UINT8  year; // Bit 0 .. Bit 6: Year (0 .. 99) Bit 7: Reserved
} CANOpenDATE;

// structure for CANOpenTIME
typedef struct
{
  M_UINT32 ms;   // Bit 0 .. Bit 27: Number of Milliseconds since midnight of 01. Jan.1984 Bit 28 .. Bit 31: Reserved
  M_UINT16 days; // Bit 0 .. Bit 15: Number of days since 01. Jan.1984 (Can be 0)
} CANOpenTIME;

#pragma pack(pop)  // Leave Byte packing


//-----------------------------------------------------------------
// helper : safe character copy
//-----------------------------------------------------------------
inline void m3ccopy(char *dst,int dlen,const char *src,bool bFill)
{
  for( ; *src && dlen; dlen--)
    *dst++=*src++;
  if (bFill)
    while (dlen--) *dst++=' ';
  else
    if (dlen) *dst++=0;
}
#define m3acopy(a,src,fill) m3ccopy(a,sizeof(a),src,fill)
