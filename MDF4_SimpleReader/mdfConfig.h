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

//-----------------------------------------------------------------------------
// CONFIGuration-Options for MDFlib
// _MDF4_DUMP  - include dump support (needed for mdf4Check.exe only)
//               add m4Dump.cpp to the project
// _MDF4_XML   - add MSXML2-support for MD Records
// _MDF4_WONLY - remove support to read blocks
// _MDF4_UTF8  - add support for utf8 conversion (only with Unicode)
// _MDF4_MD5   - add support for MD5 checksums
// _MDF4_WIN   - Windows Funktionen: Files,FILETIME etc.
//-----------------------------------------------------------------------------
// Linux Settings:
#define _MDF4_WONLY 0
#define _MDF4_DUMP  0
#define _MDF4_XML   0
#define _MDF4_UTF8  1
#define _MDF4_MD5   1
#define _MDF4_WIN   0
#define _MDF4_1     1
