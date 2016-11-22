#include "stdafx.h"
#include "ImportMdf4.h"
#include <math.h>
#include <cstring>

  /* Signed-Unsigned conversion tables */
  static M_UINT32 signed_max[33] = {
    0x00000000L, /* Gibts nicht */
    0x00000001UL-1UL,  0x00000002UL-1UL,  0x00000004UL-1UL,  0x00000008UL-1UL,  //lint !e778 (Info -- Constant expression evaluates to 0 in operation '-')
    0x00000010UL-1UL,  0x00000020UL-1UL,  0x00000040UL-1UL,  0x00000080UL-1UL,
    0x00000100UL-1UL,  0x00000200UL-1UL,  0x00000400UL-1UL,  0x00000800UL-1UL,
    0x00001000UL-1UL,  0x00002000UL-1UL,  0x00004000UL-1UL,  0x00008000UL-1UL,
    0x00010000UL-1UL,  0x00020000UL-1UL,  0x00040000UL-1UL,  0x00080000UL-1UL,
    0x00100000UL-1UL,  0x00200000UL-1UL,  0x00400000UL-1UL,  0x00800000UL-1UL,
    0x01000000UL-1UL,  0x02000000UL-1UL,  0x04000000UL-1UL,  0x08000000UL-1UL,
    0x10000000UL-1UL,  0x20000000UL-1UL,  0x40000000UL-1UL,  0x80000000UL-1UL
  };
	M_UINT32 get_signed_max(int idx)
	{
		if (idx >= 0 && idx < 33)
			return signed_max[idx];
		return 0;
	}
  static M_UINT32 signed_sub[33] = {
    0x00000000UL, /* Gibts nicht */
    0x00000002UL,  0x00000004UL,  0x00000008UL,  0x00000010UL,
    0x00000020UL,  0x00000040UL,  0x00000080UL,  0x00000100UL,
    0x00000200UL,  0x00000400UL,  0x00000800UL,  0x00001000UL,
    0x00002000UL,  0x00004000UL,  0x00008000UL,  0x00010000UL,
    0x00020000UL,  0x00040000UL,  0x00080000UL,  0x00100000UL,
    0x00200000UL,  0x00400000UL,  0x00800000UL,  0x01000000UL,
    0x02000000UL,  0x04000000UL,  0x08000000UL,  0x10000000UL,
    0x20000000UL,  0x40000000UL,  0x80000000UL,  0x00000000UL
  };
	M_UINT32 get_signed_sub(int idx)
	{
		if (idx >= 0 && idx < 33)
			return signed_sub[idx];
		return 0;
	}
/*
  static BYTE byte_mask[9] = {
    0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,
    0xff
  };
*/
  static M_UINT32 dword_mask[33] = {
    0x00000000L,0x00000001L,0x00000003L,0x00000007L,0x0000000fL,0x0000001fL,0x0000003fL,0x0000007fL,
    0x000000ffL,0x000001ffL,0x000003ffL,0x000007ffL,0x00000fffL,0x00001fffL,0x00003fffL,0x00007fffL,
    0x0000ffffL,0x0001ffffL,0x0003ffffL,0x0007ffffL,0x000fffffL,0x001fffffL,0x003fffffL,0x007fffffL,
    0x00ffffffL,0x01ffffffL,0x03ffffffL,0x07ffffffL,0x0fffffffL,0x1fffffffL,0x3fffffffL,0x7fffffffL,
    0xffffffffL
  };
	M_UINT32 get_dword_mask(int idx)
{
	if (idx >= 0 && idx < 33)
		return dword_mask[idx];
	return 0;
}
//lint -e799 (Info -- numerical constant '0x00ffffffffffffff' larger than unsigned long)

  static M_UINT64 i64_mask[65] = {
    0x0000000000000000,0x0000000000000001,0x0000000000000003,0x0000000000000007,
    0x000000000000000f,0x000000000000001f,0x000000000000003f,0x000000000000007f,
    0x00000000000000ff,0x00000000000001ff,0x00000000000003ff,0x00000000000007ff,
    0x0000000000000fff,0x0000000000001fff,0x0000000000003fff,0x0000000000007fff,
    0x000000000000ffff,0x000000000001ffff,0x000000000003ffff,0x000000000007ffff,
    0x00000000000fffff,0x00000000001fffff,0x00000000003fffff,0x00000000007fffff,
    0x0000000000ffffff,0x0000000001ffffff,0x0000000003ffffff,0x0000000007ffffff,
    0x000000000fffffff,0x000000001fffffff,0x000000003fffffff,0x000000007fffffff,
    0x00000000ffffffff,0x00000001ffffffff,0x00000003ffffffff,0x00000007ffffffff,
    0x0000000fffffffff,0x0000001fffffffff,0x0000003fffffffff,0x0000007fffffffff,
    0x000000ffffffffff,0x000001ffffffffff,0x000003ffffffffff,0x000007ffffffffff,
    0x00000fffffffffff,0x00001fffffffffff,0x00003fffffffffff,0x00007fffffffffff,
    0x0000ffffffffffff,0x0001ffffffffffff,0x0003ffffffffffff,0x0007ffffffffffff,
    0x000fffffffffffff,0x001fffffffffffff,0x003fffffffffffff,0x007fffffffffffff,
    0x00ffffffffffffff,0x01ffffffffffffff,0x03ffffffffffffff,0x07ffffffffffffff,
    0x0fffffffffffffff,0x1fffffffffffffff,0x3fffffffffffffff,0x7fffffffffffffff,
    0xffffffffffffffff
  };
M_UINT64 get_i64_mask(int idx)
{
	if (idx >= 0 && idx < 65)
		return i64_mask[idx];
	return 0;
}

void MovAndSwap(M_BYTE *pVal, M_BYTE *pDst,int nBytes)
{
  pDst+=nBytes;
  while (nBytes--) *--pDst=*pVal++;
}

///////////////////////////////////////////////////////////////////////////
// CMdf4Calc
///////////////////////////////////////////////////////////////////////////

double CMdf4Calc::MdfCalc(double val)
{
  if (m_pCC==NULL)
    return val;
	try
	{
		switch (m_pCC->cc_type)
		{
		case CC_T_IDENTITY			: return val;
		case CC_T_LINEAR				: return val * *m_pCC->m_var.get(1) + *m_pCC->m_var.get(0);
		case CC_T_RATIONAL			: // @@@@ Division durch NULL nicht abgefangen !!
															return (*m_pCC->m_var.get(0) * val* val + *m_pCC->m_var.get(1) * val + *m_pCC->m_var.get(2)) /
																		 (*m_pCC->m_var.get(3) * val * val + *m_pCC->m_var.get(4) * val + *m_pCC->m_var.get(5));
		case CC_T_FORMULA				: // ?????
															return val;
		case CC_T_N2N_INTERPOL : return do_tabi(val);
		case CC_T_N2N          : return do_tab(val);
		case CC_T_R2N          : return do_RangeToValue(val, (m_cn_data_type > 3 && m_cn_data_type<= 5));
		case CC_T_N2T          : return val;
		case CC_T_R2T          : return val;
		case CC_T_T2N          : return val;
		case CC_T_T2T          : return val;
		}
	}
	catch(...)
	{
		return 0;
	}
	return val;
}

double CMdf4Calc::do_RangeToValue(double val, BOOL bIsInt)
{
  int i1,i2, i, m = 0;
	double vmin,vmax;
	double defval = *m_pCC->m_var.get(m_pCC->cc_val_count-1);

  i1 = 0;                     // Untere Intervallgrenze
  i2 = m_pCC->cc_val_count/3-1;  // Obere Intervalgrenze
#ifdef WIN32
	if (!_finite(*m_pCC->m_var.get(0)))
#else
	if (!finite(*m_pCC->m_var.get(0)))
#endif
		m |= 1;
#ifdef WIN32
	if (!_finite(*m_pCC->m_var.get(m_pCC->cc_val_count-2)))
#else
	if (!finite(*m_pCC->m_var.get(m_pCC->cc_val_count-2)))
#endif
		m |= 2;
	if (m) // Fälle darunter und darüber behandeln:
	{
		if ((m&1) && val < *m_pCC->m_var.get(1))
			return defval;
		if ((m&2) && val > *m_pCC->m_var.get(m_pCC->cc_val_count-3))
			return defval;
		if (m & 1) i1++;
		if (m & 2) i2--;
	}
	for (i=i1; i<i2; i++)
	{
		vmin = *m_pCC->m_var.get(i*3);
		vmax = *m_pCC->m_var.get(i*3+1);
		if (vmin <= val)
		{
			if (bIsInt)
			{
				if (val <= vmax)
					return *m_pCC->m_var.get(i*3+2);
			}
			else
			{
				if (val < vmax)
					return *m_pCC->m_var.get(i*3+2);
			}
		}
	}
	return defval;
}

double CMdf4Calc::do_tabi(double val)
{
  int i1,i2, i;
  i1 = 0;                     // Untere Intervallgrenze
  i2 = m_pCC->cc_val_count/2-1;  // Obere Intervalgrenze
  if (val <= *m_pCC->m_var.get(i1*2)) 
  {
    return *m_pCC->m_var.get(i1*2+1);
  }
  if (val >= *m_pCC->m_var.get(i2*2)) 
  {
    return *m_pCC->m_var.get(i2*2+1);
  }
  while (i2-i1>1) 
  {
    i = (i2+i1)/2;
    if (val < *m_pCC->m_var.get(i*2)) 
      i2 = i;
    else if (val > *m_pCC->m_var.get(i*2)) 
      i1 = i;
    else 
    {
      val = *m_pCC->m_var.get(i*2+1);
      break;
    }
  }
  // Zwischen den Stützstellen interpolieren 
	i2 = i2*2+1;
	i1 = i1*2+1;
  double m = (*m_pCC->m_var.get(i2) - *m_pCC->m_var.get(i1)) / (*m_pCC->m_var.get(i2) - *m_pCC->m_var.get(i1));
  val = m * (val - *m_pCC->m_var.get(i1)) + *m_pCC->m_var.get(i1);
  return val;
}

double CMdf4Calc::do_tab(double val)
{
  int i1,i2, i;
  i1 = 0;                     // Untere Intervallgrenze
  i2 = m_pCC->cc_val_count/2-1;  // Obere Intervalgrenze
  if (val <= *m_pCC->m_var.get(i1*2)) 
  {
    return *m_pCC->m_var.get(i1*2+1);
  }
  if (val >= *m_pCC->m_var.get(i2*2)) 
  {
    return *m_pCC->m_var.get(i2*2+1);
  }
  while (i2-i1>1) 
  {
    i = (i2+i1)/2;
    if (val < *m_pCC->m_var.get(i*2)) 
      i2 = i;
    else if (val > *m_pCC->m_var.get(i*2)) 
      i1 = i;
    else 
      return *m_pCC->m_var.get(i*2+1);
  }
	if (i2-val > val-i1) // Die nächst-untere Stützstelle nehmen
		return *m_pCC->m_var.get(i1*2+1);
	else // Die nächst-höhere Stützstelle nehmen
		return *m_pCC->m_var.get(i2*2+1);
}

/////////////////////////////////////////////////////////////////////////
// CMdf4DataGroup
/////////////////////////////////////////////////////////////////////////

BOOL CMdf4DataGroup::GetRecord(M4CGBlock *cg, M_BYTE *ptr, M_UINT64 ix1, M_UINT64 ix2)
{
  void *pOrgPtr = ptr;
	M_UINT32 RecLen = (M_UINT32)cg->getRecordSize() + m_dg->dg_rec_id_size; 
  M_UINT64 dwPos = ix1 * RecLen;
	M_UINT64 n = ix2 - ix1 + 1;

	M_UINT32 nRFAllocated = 0;
	if (n < 1000 && m_pDataStream)
	{
		nRFAllocated = 2;
	}
	else if (m_pRF == NULL)
	{
		m_pRF = new CReadFileMF4;
		while (n > 5000)
			n /= 2;
		if (!m_pRF->Open( m_dg, RecLen, n))
		{
			delete m_pRF;
			m_pRF = NULL;
			return FALSE;
		}
		nRFAllocated = 1;
	}

	if (nRFAllocated != 2)
	{
		if (!m_pRF->Seek(dwPos))
		{
			m_pRF->Close();
			if (nRFAllocated==1)
			{
				delete m_pRF;
				m_pRF = NULL;
			}
			return FALSE;
		}
	}
	else
	{
		if (m_pDataStream->Seek(dwPos) != dwPos)
		{
			return FALSE;
		}
	}

	if (nRFAllocated != 2)
	  m_pRF->Read( RecLen * n, ptr);
	else
	  m_pDataStream->Read( RecLen, ptr);
	if (nRFAllocated==1)
	{
		m_pRF->Close();
		delete m_pRF;
		m_pRF = NULL;
	}
	return TRUE;
}

double CMdf4DataGroup::GetValueFromRecord(M_BYTE *dptr, int sbit, int nbit, int nbytes, BOOL bInteger, BOOL bSigned, BOOL bMotorola, BOOL bFloat)
{
	double val;
	M_UINT64 dummy = 0;
	void *ptr = (void*)&dummy;
	if (bFloat)
	{
		if (nbit == 32)
		{
			if (bMotorola)
				MovAndSwap( (M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
			else
				*(long*)ptr = *((long*)dptr);
			val = (double)*((float*)ptr);
		}
		else
		{
			if (bMotorola)
				MovAndSwap( (M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
			else
				*(double*)ptr = *((double*)dptr);
			val = *((double*)ptr);
		}
	}
	else if (bInteger && nbit > 32)
	{
		if (bMotorola)
			MovAndSwap( (M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
		else
			memcpy(ptr, dptr, nbytes);
		M_UINT64 i64Val = *((M_UINT64*)ptr);
		i64Val = (i64Val>>sbit) & get_i64_mask(nbit);
		val = (double)i64Val;
	}
	else
	{
		if (bMotorola)
			MovAndSwap( (M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
		else
			memcpy(ptr, dptr, nbytes);
		M_UINT32 dwValue = *(M_UINT32*)ptr;
		if (nbit > 16)
		{
			M_UINT32 get_dword_mask(int idx);
			M_UINT32 get_signed_max(int idx);
			dwValue = (dwValue>>sbit) & get_dword_mask(nbit);
			if (bSigned && dwValue > get_signed_max(nbit)) 
				val = (double)(long)dwValue;
			else 
				val = (double)dwValue;
		}
		else 
		{
			M_UINT16 wValue = (dwValue>>sbit) & (M_UINT16)get_dword_mask(nbit);
			if (bSigned && dwValue > get_signed_max(nbit)) 
			{
				if (nbit <= 8)
					val = (double)(char)dwValue;
				else 
					val = (double)(short)dwValue;
			}
			else
				val = (double)dwValue;
		}
	}
	return val;
}

BOOL CMdf4DataGroup::GetRawValueFromRecord(M4CGBlock *cg, M4CNBlock *cn, M_BYTE *pRecord, double *pValue)
{
	BOOL bIsNoval = FALSE;
	M_BYTE *ibptr = NULL;
	M_BYTE *pInv=NULL;
	int iIBpos;
  M_UINT16 nbit,sbit;
	int nbytes;
	M_UINT64 buf;
	M_BYTE *ptr = (M_BYTE *)&buf;

  if (cn->cn_data_type>=CN_D_STRING)
    return TRUE;

  BOOL bInteger = cn->cn_data_type<=CN_D_SINT_BE;
  BOOL bSigned = cn->cn_data_type>=CN_D_SINT_LE;
  BOOL bMotorola = (cn->cn_data_type & 1);
  BOOL bFloat = cn->cn_data_type>CN_D_SINT_BE;

	ibptr = pRecord;
	sbit = cn->cn_bit_offset;
  pRecord += sbit >> 3; // Byte Offset
	pRecord += cn->cn_byte_offset; 
  if (m_dg->dg_rec_id_size)
    pRecord+=m_dg->dg_rec_id_size;
  sbit &= 7;
	nbit = cn->cn_bit_count;
  nbytes = (nbit+7) >> 3;

	if (cn->cn_flags & CN_F_INVALID)
	{
		M_UINT32 RecLen = cg->getRecordSize() + m_dg->dg_rec_id_size; 
		ibptr += RecLen - cg->cg_inval_bytes;
		iIBpos = cn->cn_inval_bit_pos;
		ibptr += iIBpos / 8;
		iIBpos = 1 << (iIBpos % 8);
		if (*ibptr & iIBpos)
			bIsNoval = TRUE;
	}
	*pValue = GetValueFromRecord(pRecord, sbit, nbit, nbytes, bInteger, bSigned, bMotorola, bFloat);
	if (!bIsNoval && bFloat)
	{
#ifdef WIN32
		if (!_finite(*pValue))
#else
		if (!finite(*pValue))
#endif
		{
			bIsNoval = TRUE;
			*pValue = 1E308;
		}
	}
	return bIsNoval;
}

/////////////////////////////////////////////////////////////////////////////////////
// CMdf4FileImport
/////////////////////////////////////////////////////////////////////////////////////

CMdf4FileImport::CMdf4FileImport(void)
{
	m_vDataGroups.clear();
	m_nDataGroupsInvalid = 0;
}


CMdf4FileImport::~CMdf4FileImport(void)
{
}

// Helpers
char* CMdf4FileImport::GetString(M4TXBlock *pTX)
{
	static char szBuffer[4096];
	memset(szBuffer,0,sizeof(szBuffer));
	if (pTX)
	{
		const unsigned char *t = pTX->m_utf8.data();
		//utf8in in(pTX->m_utf8.data());
		strncpy(szBuffer, (char*)t, 4095);
	}
	return szBuffer;
}

const char *StrID(M_UINT16 hdrID)
{
	static char szBuffer[8];
	for (int i=0; i<4; i++)
	{
		szBuffer[i] = hdrID & 0xFF;
		hdrID >>= 8;
	}
	szBuffer[4]=0;
	return szBuffer;
}

void CMdf4FileImport::DisplayGroup(CMdf4DataGroup* pGroup)
{
	M4DGBlock *pDG = pGroup->m_dg;
	printf("  1st Channel Group = 0x%llX\n", pDG->getLink(M4DGBlock::dg_cg_first));
	printf("  1st Data Block = ");
	m4Block *b = NULL;
	M_LINK l = pDG->getLink(M4DGBlock::dg_data);
	if (l)
		b = m_m4.LoadLink( *pDG, M4DGBlock::dg_data);
	if (b)
		printf("0x%I64X ID = %s\n", l, StrID(b->hdrID()));
	else
		printf("0x%I64X\n",  l);
	printf("  Record ID Size = %u\n",(unsigned int )pDG->dg_rec_id_size);
}

void CMdf4FileImport::DisplayChannelGroup(M4CGBlock *cg)
{
	char *p = " ";
	printf("  Channelgroup:\n");
	printf("    1st Channel        = 0x%llX\n",  cg->getLink(M4CGBlock::cg_cn_first));
	M4TXBlock *tx = (M4TXBlock *)m_m4.LoadLink( *cg, M4CGBlock::cg_tx_acq_name);
	if (tx)
	{
		p = GetString(tx);
		delete tx;
	}
	printf("    Acquisition Name   = %s\n",p);

	p = " ";
	M4SIBlock *si = (M4SIBlock *)m_m4.LoadLink( *cg, M4CGBlock::cg_si_acq_source);
	if (si)
	{
		tx = (M4TXBlock *)m_m4.LoadLink( *si, M4SIBlock::si_tx_name);
		if (tx)
		{
			p = GetString(tx);
			delete tx;
		}
		delete si;
	}
	printf("    Acquisition Source = %s\n",p);
	printf("    Record ID          = %llu\n", cg->cg_record_id);
	printf("    Cycle Count        = %llu\n", cg->cg_cycle_count);
	printf("    Data Bytes         = %lu\n", cg->cg_data_bytes);
	printf("    Invalid Bytes      = %lu\n", cg->cg_inval_bytes);
}

void CMdf4FileImport::DisplayChannel(M4CNBlock *cn, int n)
{
	char *p = " ";
	printf("    Channel %d\n",n);
	M4TXBlock *tx = (M4TXBlock *)m_m4.LoadLink( *cn, M4CNBlock::cn_tx_name);
	if (tx)
	{
		p = GetString(tx);
		delete tx;
	}
	printf("      Name = %s\n",p);

	p = " ";
	M4SIBlock *si = (M4SIBlock *)m_m4.LoadLink( *cn, M4CNBlock::cn_si_source);
	if (si)
	{
		tx = (M4TXBlock *)m_m4.LoadLink( *si, M4SIBlock::si_tx_name);
		if (tx)
		{
			p = GetString(tx);
			delete tx;
		}
		delete si;
	}
	printf("      Source = %s\n",p);

	p = " ";
	tx = (M4TXBlock *)m_m4.LoadLink( *cn, M4CNBlock::cn_md_unit);
	if (tx)
	{
		p = GetString(tx);
		delete tx;
	}
	printf("      Unit = %s\n",p);

	printf("      Channel Type = %lu\n",  (unsigned int)cn->cn_type);
	printf("      Data Type = %lu\n",  (unsigned int)cn->cn_data_type);
	printf("      Bit Offset = %lu\n",  (unsigned int)cn->cn_bit_offset);
	printf("      Byte Offset = %lu\n",  (unsigned int)cn->cn_byte_offset);
	printf("      Bit Count = %lu\n",  (unsigned int)cn->cn_bit_count);
	printf("      Flags = 0x%lX\n",  (unsigned int)cn->cn_flags);
	printf("      Inval. Bit Pos = %lu\n", cn->cn_inval_bit_pos);
	printf("      Min Range Raw = %lf\n", cn->cn_val_range_min);
	printf("      Max Range Raw = %lf\n", cn->cn_val_range_max);
}

void CMdf4FileImport::GetChannelName(M4CNBlock *cn, int n, char *channel_name)
{
  char *p = "";
	M4TXBlock *tx = (M4TXBlock *)m_m4.LoadLink( *cn, M4CNBlock::cn_tx_name);
	if (tx)
	{
		p = GetString(tx);
		delete tx;
	}
  strcpy(channel_name,p);
}

void CMdf4FileImport::DisplayConversion(M4CCBlock *cc)
{
	char *p = " ";
	switch (cc->cc_type)
	{
	case CC_T_IDENTITY    :  p = "1:1 conversion"; break;
	case CC_T_LINEAR      :  p = "linear conversion"; break;
  case CC_T_RATIONAL    :  p = "rational conversion"; break; 
  case CC_T_FORMULA     :  p = "algebraic conversion (MCD-2 MC text formula)"; break; 
	case CC_T_N2N_INTERPOL:  p = "value to value tabular look-up with interpolation"; break; 
  case CC_T_N2N         :  p = "value to value tabular look-up without interpolation"; break; 
  case CC_T_R2N         :  p = "value range to value tabular look-up"; break; 
  case CC_T_N2T         :  p = "value to text/scale conversion tabular look-up"; break; 
  case CC_T_R2T         :  p = "value range to text/scale conversion tabular look-up"; break; 
  case CC_T_T2N         :  p = "text to value tabular look-up"; break; 
  case CC_T_T2T         :  p = "text to text tabular look-up (translation)"; break;
	default               :  p = "unknown"; break;
	}
	printf("      Conversion Type = %s\n",p);

	if (cc->cc_type == CC_T_LINEAR)
	{
		double f = *cc->m_var.get(1);
		double o = *cc->m_var.get(0);
		printf("        Factor = %lf\n",f);
		printf("        Offset = %lf\n",o);
	}
}

void CMdf4FileImport::DisplayData(CMdf4DataGroup* pGroup, M4DGBlock *dg,M4CGBlock *cg,M4CNBlock *cn)
{
	int iScanSize = cg->cg_data_bytes + cg->cg_inval_bytes + dg->dg_rec_id_size;
  M_UINT8 *pScan = (M_UINT8*)calloc(1,iScanSize);
	int n = cg->cg_cycle_count;
  
	for (int i=0; i<n; i++)
	{
		BOOL bResult = pGroup->GetRecord(cg, pScan, i, i);
		if (!bResult)
			printf("        read error\n");
		else
		{
			double value;
			BOOL bNoval = pGroup->GetRawValueFromRecord(cg, cn, pScan, &value);
			if (bNoval)
				printf("novalue %.15lg\n",value);
			else
			{
				CMdf4Calc * pCalc = new CMdf4Calc( cn, m_m4);
				if (pCalc->m_pCC != NULL)
				{
					value = pCalc->MdfCalc(value);
				}
        printf("%20.15lg\n",value);

				delete pCalc;
			}
		}
	}
}


void CMdf4FileImport::MyDisplayData(CMdf4DataGroup* pGroup,
                                    M4DGBlock *dg,
                                    M4CGBlock *cg,
                                    M4CNBlock *cn,
                                    char *filename,
                                    char *channel_name,
                                    M4CNBlock *time_channel)
{
	int iScanSize = cg->cg_data_bytes + cg->cg_inval_bytes + dg->dg_rec_id_size;
  M_UINT8 *pScan = (M_UINT8*)calloc(1,iScanSize);
	int n = cg->cg_cycle_count;

	for (int i=0; i<n; i++)
	{
		BOOL bResult = pGroup->GetRecord(cg, pScan, i, i);
		if (!bResult)
 			printf("read error,read error,read error,read error\n");
 		else
		{
      double timestamp;
			double value;
      BOOL tNoval = pGroup->GetRawValueFromRecord(cg, time_channel, pScan, &timestamp);
			BOOL bNoval = pGroup->GetRawValueFromRecord(cg, cn, pScan, &value);

			if (!bNoval)
			{
				CMdf4Calc * pCalc = new CMdf4Calc( cn, m_m4);
				if (pCalc->m_pCC != NULL)
				{
					value = pCalc->MdfCalc(value);
				}

				delete pCalc;
			}

			if (!tNoval)
			{
				CMdf4Calc * pCalc = new CMdf4Calc( time_channel, m_m4);
				if (pCalc->m_pCC != NULL)
				{
					timestamp = pCalc->MdfCalc(timestamp);
				}

				delete pCalc;
			}

      if(!tNoval && !bNoval)
        printf("%s,%f,%s,%f\n",filename,timestamp,channel_name,value);
      else
        printf("read error,read error,read error,read error\n");

    }
	}
}

BOOL CMdf4FileImport::MyImportFile(char *szFileName)
{
  int i,j;
  char channel_name[4096];
  char *p_channel_name = channel_name;
  
#ifdef UNICODE
	wchar_t szFileBuffer[260];
	mbstowcs(szFileBuffer, szFileName, 260);
	if (!m_m4.Open(szFileBuffer))
#else
	if (!m_m4.Open(szFileName))
#endif
		return FALSE;

	M4HDBlock *pHdr = m_m4.GetHdr();

	// Count groups
  M_UINT32 nGrp=0;
	M4DGBlock *dg;
	M4CGBlock *cg;
  dg = (M4DGBlock *)m_m4.LoadLink(*pHdr,M4HDBlock::hd_dg_first);
	while (dg)
	{
		cg = (M4CGBlock *)m_m4.LoadLink(*dg, M4DGBlock::dg_cg_first);
		while (cg)
		{
			CMdf4DataGroup *p = new CMdf4DataGroup;
			m_vDataGroups.push_back(p);
			nGrp++;
			m_vDataGroups[nGrp-1]->m_iGrp = nGrp;
			m_vDataGroups[nGrp-1]->m_pMdf = this;
			m_vDataGroups[nGrp-1]->m_dg = dg;

			cg = (M4CGBlock *)LoadLink(cg, M4CGBlock::cg_cg_next); // would be unsorted -> not supported
		}
		dg = (M4DGBlock *)m_m4.LoadLink(*dg,M4DGBlock::dg_dg_next);
  }
	m_nDataGroups = nGrp;
	for (i=0; i<nGrp; i++)
	{
		CMdf4DataGroup* pGroup = m_vDataGroups[i];
		M4DGBlock *dg = pGroup->m_dg;
		/* printf("Group %d of %d:\n",i+1,m_nDataGroups); */
		/* DisplayGroup(pGroup); */
		cg = (M4CGBlock *)m_m4.LoadLink(*dg, M4DGBlock::dg_cg_first);
		/* DisplayChannelGroup(cg); */
		j = 0;
		M4CNBlock *tn = (M4CNBlock *)m_m4.LoadLink(*cg, M4CGBlock::cg_cn_first, M4ID_CN);
    M4CNBlock *cn = (M4CNBlock *)m_m4.LoadLink(*tn, M4CNBlock::cn_cn_next, M4ID_CN);
		while (cn)
		{
      GetChannelName(cn,++j,channel_name);
			MyDisplayData(pGroup, dg,cg,cn,szFileName,channel_name,tn);
			cn = (M4CNBlock *)m_m4.LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
		}
	}	

	return TRUE;
}


BOOL CMdf4FileImport::ImportFile(const char *szFileName)
{
  int i,j;
#ifdef UNICODE
	wchar_t szFileBuffer[260];
	mbstowcs(szFileBuffer, szFileName, 260);
	if (!m_m4.Open(szFileBuffer))
#else
	if (!m_m4.Open(szFileName))
#endif
		return FALSE;

	printf("File %s\n",szFileName);

	// ID Block
	mdfFileId *pId = m_m4.GetIdBlk();
	printf("id_file  = %s\n",pId->id_file);
	printf("id_vers  = %s\n",pId->id_vers);
	printf("id_prog  = %s\n",pId->id_prog);
	printf("id_order = %s\n",pId->id_order==0 ? "Intel" : "Motorola");
	printf("id_float = %s\n",pId->id_float==0 ? "IEEE 754" : "(unsupported)");
	printf("id_ver   = %d\n", (int)pId->id_ver);
	
	// Show time: don't know how to handle local/GMT time under Linux
	M4HDBlock *pHdr = m_m4.GetHdr();
	printf("Time: %d\n",(long)(pHdr->hd_start_time.time_ns/1000000000));

	// Count groups
  M_UINT32 nGrp=0;
	M4DGBlock *dg;
	M4CGBlock *cg;
  dg = (M4DGBlock *)m_m4.LoadLink(*pHdr,M4HDBlock::hd_dg_first);
	while (dg)
	{
		cg = (M4CGBlock *)m_m4.LoadLink(*dg, M4DGBlock::dg_cg_first);
		while (cg)
		{
			CMdf4DataGroup *p = new CMdf4DataGroup;
			m_vDataGroups.push_back(p);
			nGrp++;
			m_vDataGroups[nGrp-1]->m_iGrp = nGrp;
			m_vDataGroups[nGrp-1]->m_pMdf = this;
			m_vDataGroups[nGrp-1]->m_dg = dg;

			cg = (M4CGBlock *)LoadLink(cg, M4CGBlock::cg_cg_next); // would be unsorted -> not supported
		}
		dg = (M4DGBlock *)m_m4.LoadLink(*dg,M4DGBlock::dg_dg_next);
  }
	m_nDataGroups = nGrp;
	for (i=0; i<nGrp; i++)
	{
		CMdf4DataGroup* pGroup = m_vDataGroups[i];
		M4DGBlock *dg = pGroup->m_dg;
		printf("Group %d of %d:\n",i+1,m_nDataGroups);
		DisplayGroup(pGroup);
		cg = (M4CGBlock *)m_m4.LoadLink(*dg, M4DGBlock::dg_cg_first);
		DisplayChannelGroup(cg);
		j = 0;
		M4CNBlock *cn = (M4CNBlock *)m_m4.LoadLink(*cg, M4CGBlock::cg_cn_first, M4ID_CN);
		while (cn)
		{
			DisplayChannel(cn,++j);
			if (cn->hasLink(M4CNBlock::cn_cc_conversion))
			{
				M4CCBlock *cc = (M4CCBlock *)m_m4.LoadLink(*cn, M4CNBlock::cn_cc_conversion, M4ID_CC);
				if (cc)
				{
					DisplayConversion(cc);
					delete cc;
				}
			}
			DisplayData(pGroup, dg,cg,cn);
			cn = (M4CNBlock *)m_m4.LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
		}
	}	

	return TRUE;
}


m4Block* CMdf4FileImport::LoadLink(m4Block *pParent,int linkNo)
{
	m4Block* pResult = m_m4.LoadBlock(pParent->getLink(linkNo));
	delete pParent;
  return pResult;
}


void CMdf4FileImport::ReleaseFile(void)
{
	for ( std::vector<CMdf4DataGroup*>::iterator it = m_vDataGroups.begin(); it != m_vDataGroups.end(); it++ )
	{
		delete *it;
	}
	m_vDataGroups.clear();

	if (m_m4.isOpen())
		m_m4.Close();
}

