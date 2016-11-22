#include "stdafx.h"
#include "m4DumpCL.h"

//=================================== dump() ===========================================================
#ifdef _UNICODE
#define dump_mem(n,f) Tree->m_Definition.AppendFormat(L#n L"=" f L"\r\n",n)
#define dump_var(n,v) Tree->m_Definition.AppendFormat(n  L"\r\n",v)
#define dump_flag(v,b) Tree->m_Definition.AppendFormat(L" " L#b L"= %d\r\n",v&b ? 1 : 0)
#define dump_enum(n,e) Tree->m_Definition.AppendFormat(L#n L"= %u (%s)\r\n",n,enumerate(n,e))
#define dump_link_(f,n) Tree->m_Tree.append(new m4TreeLink(L#n,getLink(n)))
#define dump_link(f,n)  \
  { \
    m4TreeItem *t=Tree->m_Tree.append(new m4TreeLink(L#n,getLink(n))); \
    if (0==getLink(n)) { t->m_Definition=_T("-NIL-"); } \
    else { m4Block *b=f.LoadLink(*this,n); if (b==NULL) t->m_Definition=_T("?FAILED?"); \
    else { b->dump(f,t); delete b; }}}
#else
M_CHAR szGlobalBuffer[16384];
#define dump_mem(n,f)   { sprintf(szGlobalBuffer,#n "=" f "\r\n",n); strcat(Tree->m_Definition, szGlobalBuffer);}
#define dump_var(n,v)   { sprintf(szGlobalBuffer,n  "\r\n",v); strcat(Tree->m_Definition, szGlobalBuffer);}
#define dump_flag(v,b)  { sprintf(szGlobalBuffer," " #b "= %d\r\n",v&b ? 1 : 0); strcat(Tree->m_Definition, szGlobalBuffer);}
#define dump_enum(n,e)  { sprintf(szGlobalBuffer,#n "= %u (%s)\r\n",n,enumerate(n,e)); strcat(Tree->m_Definition, szGlobalBuffer);}
#define dump_link_(f,n) Tree->m_Tree.append(new m4TreeLink(#n,getLink(n)))
#define dump_link(f,n)  \
  { \
    m4TreeItem *t=Tree->m_Tree.append(new m4TreeLink(#n,getLink(n))); \
    if (0==getLink(n)) { strcpy(t->m_Definition,"-NIL-"); } \
    else { m4Block *b=f.LoadLink(*this,n); if (b==NULL) strcpy(t->m_Definition,"?FAILED?"); \
    else { b->dump(f,t); delete b; }}}
#endif


typedef struct
{
  LPCTSTR Name;
  M_UINT32    Value;
} m4Enumeration;
#ifdef _UNICODE
#define _e(x) { L#x, x },
#else
#define _e(x) { #x, x },
#endif

LPCTSTR enumerate(M_UINT32 v,const m4Enumeration *e)
{
  for( ; e->Name; e++)
    if (v==e->Value)
      return e->Name;
  return _T("");
}

void m4Block::dump(MDF4File &f,m4TreeItem *Tree)
{
	M_CHAR szBuffer[128];
	sprintf(szBuffer,"%s BLOCK @ %016llX\r\n",&m_Hdr.hdr_hdr,m_At);
	strcat(Tree->m_Definition,szBuffer);
}

void M4HDBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);

  // memory
  dump_mem(hd_start_time.time_ns,_T("%lld"));
  dump_mem(hd_start_time.tz_offset_min,_T("%d"));
  dump_mem(hd_start_time.dst_offset_min,_T("%d"));
  dump_mem(hd_start_time.time_flags,_T("%u"));
  dump_flag(hd_start_time.time_flags,HD_TF_LOCALTIME);
  dump_flag(hd_start_time.time_flags,HD_TF_TIMEOFFS);
#if _MDF4_WIN
  SYSTEMTIME st;
  M_DATE::get(hd_start_time,st);
	Tree->m_Definition.AppendFormat(_T(" (converted) %04d %02d. %02d. %02d:%02d:%02d.%d \r\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
#endif

  dump_mem(hd_time_class,_T("%u"));
  dump_mem(hd_flags,_T("%02X"));
  dump_mem(hd_reserved,_T("%u"));
  dump_mem(hd_start_angle_rad,_T("%g"));
  dump_mem(hd_start_distance_m,_T("%g"));
  // links
  dump_link(f,hd_dg_first);
  dump_link(f,hd_fh_first);
  dump_link(f,hd_ch_first);
  dump_link(f,hd_at_first);
  dump_link(f,hd_ev_first);
  dump_link(f,hd_md_comment);

}
//-----------------------------------------------------
// ##TX Text
//-----------------------------------------------------
void M4TXBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
  const M_UINT8 *t=m_utf8.data();

  strcat(Tree->m_Definition,(const char*)m_utf8.data());
}

#define FH_TF_LOCALTIME  BIT0 // Local time flag If set, the start time stamp in nanoseconds represents the local time instead of the UTC time, In this case, time zone and DST offset must not be considered (time offsets flag must not be set). Should only be used if UTC time is unknown. If the bit is not set (default), the start time stamp represents the UTC time.
#define FH_TF_TIMEOFFS   BIT1 // Time offsets valid flag If set, the time zone and DST offsets are valid. Must not be set together with "local time" flag (mutually exclusive). If the offsets are valid, the locally displayed time at start of recording can be determined (after conversion of offsets to ns) by Local time = UTC time + time zone offset + DST offset.

//-----------------------------------------------------
// ##FH FileHistory
//-----------------------------------------------------
void M4FHBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
  // memory
  dump_mem(fh_time.time_ns,_T("%lld"));
  dump_mem(fh_time.tz_offset_min,_T("%d"));
  dump_mem(fh_time.dst_offset_min,_T("%d"));
  dump_mem(fh_time.time_flags,_T("%u"));
  dump_flag(fh_time.time_flags,FH_TF_LOCALTIME);
  dump_flag(fh_time.time_flags,FH_TF_TIMEOFFS);
#if _MDF4_WIN
  SYSTEMTIME st;
  M_DATE::get(fh_time,st);
	Tree->m_Definition.AppendFormat(_T(" (converted) %04d %02d. %02d. %02d:%02d:%02d.%d \r\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
#endif
  // links
  dump_link(f,fh_fh_next);
  dump_link(f,fh_md_comment);

}


//-----------------------------------------------------
// ##EV Events
//-----------------------------------------------------
void M4EVBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  static const m4Enumeration enum_ev_type[]=
  {
    _e(EV_T_RECORDING)   // Recording
    _e(EV_T_RECINT)      // Recording Interrupt
    _e(EV_T_ACQINT)      // Acquisition interrupt
    _e(EV_T_STARTTRG)    // Start Recording Trigger
    _e(EV_T_STOPTRG)     // Stop Recording Trigger
    _e(EV_T_TRIGGER)     // Trigger
    _e(EV_T_MARKER)      // Marker
    { NULL,0}
  };
  static const m4Enumeration enum_ev_sync_type[]=
  {
    _e(EV_S_TIME)         // calculated synchronization value represents time in seconds
    _e(EV_S_ANGLE)        // calculated synchronization value represents angle in radians
    _e(EV_S_DISTANCE)     // calculated synchronization value represents distance in meter
    _e(EV_S_INDEX)        // calculated synchronization value represents zero-based record index
    { NULL,0}
  };
  static const m4Enumeration enum_ev_range_type[]=
  {
    _e(EV_R_POINT)         // event defines a point
    _e(EV_R_START)         // event defines the beginning of a range
    _e(EV_R_END)           // event defines the end of a range
    { NULL,0}
  };
  static const m4Enumeration enum_ev_cause[]=
  {
    _e(EV_C_OTHER)        // OTHER cause of event is not known or does not fit into given categories.
    _e(EV_C_ERROR)        // ERROR event was caused by some error.
    _e(EV_C_TOOL)         // TOOL event was caused by tool-internal condition, e.g. trigger condition or re-configuration.
    _e(EV_C_SCRIPT)       // SCRIPT event was caused by a scripting command.
    _e(EV_C_USER)         // USER event was caused directly by user, e.g. user input or some other interaction with GUI.
    { NULL,0}
  };
  m4Block::dump(f,Tree);
  // memory
  dump_enum(ev_type,enum_ev_type);
  dump_enum(ev_sync_type,enum_ev_sync_type);
  dump_enum(ev_range_type,enum_ev_range_type);
  dump_enum(ev_cause,enum_ev_cause);
  dump_mem(ev_flags,_T("%u"));
  dump_flag(ev_flags,EV_F_POST);
  dump_mem(ev_scope_count,_T("%u"));
  dump_mem(ev_attachment_count,_T("%u"));
  dump_mem(ev_creator_index,_T("%u"));
  dump_mem(ev_sync_base_value,_T("%lld"));
  dump_mem(ev_sync_factor,_T("%g"));

  // links
  dump_link(f,ev_ev_next);
  //dump_link(f,ev_ev_parent); wg. Rekursion
  //dump_link(f,ev_ev_range);
  dump_link(f,ev_tx_name);
  dump_link(f,ev_md_comment);
  dump_link(f,ev_scope);
  dump_link(f,ev_at_reference);
}
//-------------------------------------------------------------------------------------------------------
// ##DG Data Group Block
//-------------------------------------------------------------------------------------------------------
void M4DGBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
  // memory
  dump_mem(dg_rec_id_size,_T("%u"));

  // links
  dump_link(f,dg_dg_next);
  dump_link(f,dg_cg_first);
  dump_link(f,dg_data);
  dump_link(f,dg_md_comment);

  // Test m4DataStream
  m4DataStream *s=ReadStream();
  if (s && s->getSize())
  {
		M_CHAR szBuffer[128];
		sprintf(szBuffer,"\r\nStream Size=%llu\r\n",s->getSize());
    M_BYTE Buffer[16];
    s->Seek(s->getSize()/2);
    s->Read(16,Buffer);
  }
  delete s;
}

//-------------------------------------------------------------------------------------------------------
// ##CG Channel Group Block
//-------------------------------------------------------------------------------------------------------
void M4CGBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
  // memory
  dump_mem(cg_record_id,_T("%llu"));
  dump_mem(cg_cycle_count,_T("%llu"));
  dump_mem(cg_flags,_T("%u"));
  dump_flag(cg_flags,CG_FL_VLSD);
  dump_mem(cg_data_bytes,_T("%u"));
  dump_mem(cg_inval_bytes,_T("%u"));
  // links
  dump_link(f,cg_cg_next);
  dump_link(f,cg_cn_first);
  dump_link(f,cg_tx_acq_name);
  dump_link(f,cg_si_acq_source);
  dump_link(f,cg_sr_first);
  dump_link(f,cg_md_comment);
}

//-------------------------------------------------------------------------------------------------------
// ##SI Source Information Block
//-------------------------------------------------------------------------------------------------------

void M4SIBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  static const m4Enumeration enum_si_type[]=
  {
    _e(SI_T_OTHER)   // OTHER source type does not fit into given categories or is unknown
    _e(SI_T_ECU)     // ECU source is an ECU
    _e(SI_T_BUS)     // BUS source is a bus (e.g. for bus monitoring)
    _e(SI_T_DEVICE)  // I/O source is an I/O device (e.g. analog I/O)
    _e(SI_T_TOOL)    // TOOL source is a software tool (e.g. for tool generated signals/events)
    _e(SI_T_USER)    //  USER source is a user interaction/input (e.g. for user generated events)
    { NULL,0}
  };
  static const m4Enumeration enum_si_bus_type[]=
  {
    _e(SI_BUS_NONE)    // NONE no bus
    _e(SI_BUS_OTHER)   // OTHER bus type does not fit into given categories or is unknown
    _e(SI_BUS_CAN)     // CAN
    _e(SI_BUS_LIN)     // LIN
    _e(SI_BUS_MOST)    // MOST
    _e(SI_BUS_FLEXR)   // FLEXRAY
    _e(SI_BUS_KLINE)   // K_LINE
    _e(SI_BUS_NET)     // ETHERNET
    _e(SI_BUS_USB)     // USB
    { NULL,0}
  };
  m4Block::dump(f,Tree);
  // memory
  dump_enum(si_type,enum_si_type);
  dump_enum(si_bus_type,enum_si_bus_type);
  dump_mem(si_flags,_T("%u"));
  dump_flag(si_flags,SI_FL_SIMULATED);
  // links
  dump_link(f,si_tx_name);
  dump_link(f,si_tx_path);
  dump_link(f,si_md_comment);
}

//-------------------------------------------------------------------------------------------------------
// ##CN Channel Block
//-------------------------------------------------------------------------------------------------------
void M4CNBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  static const m4Enumeration enum_cn_type[]=
  {
    _e(CN_T_FIXEDLEN)   // fixed length data channel
    _e(CN_T_VLSD)       // variable length data channel
    _e(CN_T_MASTER)     // master channel for all signals of this group
    _e(CN_T_VMASTER)    // virtual master channel
    _e(CN_T_SYNC)       // synchronization channel
    { NULL,0}
  };
  static const m4Enumeration enum_cn_sync_type[]=
  {
    _e(CN_S_NONE)      // None (to be used for normal data channels)
    _e(CN_S_TIME)      // Time (physical values must be seconds)
    _e(CN_S_ANGLE)     // Angle (physical values must be radians)
    _e(CN_S_DISTANCE)  // Distance (physical values must be meters)
    _e(CN_S_INDEX)     // Index (physical values must be zero-based index values)
    { NULL,0}
  };
  static const m4Enumeration enum_cn_data_type[]=
  {
    _e(CN_D_UINT_LE)    // Unsigned Integer LE Byte Order
    _e(CN_D_UINT_BE)    // Unsigned Integer BE Byte Order
    _e(CN_D_SINT_LE)    // Signed Integer LE Byte Order
    _e(CN_D_SINT_BE)    // Snsigned Integer BE Byte Order
    _e(CN_D_FLOAT_LE)   // Float (IEEE 754) LE Byte Order
    _e(CN_D_FLOAT_BE)   // Float (IEEE 754) BE Byte Order
    _e(CN_D_STRING)     // String (ISO-8859-1 Latin), NULL terminated)
    _e(CN_D_UTF8)       // String (UTF8-encoded), NULL terminated)
    _e(CN_D_UTF16_LE)   // String (UTF16-LE Byte order), NULL terminated)
    _e(CN_D_UTF16_BE)   // String (UTF16-BE Byte order), NULL terminated)
    _e(CN_D_BYTES)      // Byte array
    _e(CN_D_SAMPLE)     // MIME sample
    _e(CN_D_STREAM)     // MIME stream
    _e(CN_D_CODATE)     // CANOpen Date
    _e(CN_D_COTIME)     // CANOpen Time
    { NULL,0}
  };
  m4Block::dump(f,Tree);
  // memory
  dump_enum(cn_type,enum_cn_type);
  dump_enum(cn_sync_type,enum_cn_sync_type);
  dump_enum(cn_data_type,enum_cn_data_type);
  dump_mem(cn_flags,_T("%u"));
  dump_flag(cn_flags,CN_F_ALLINVALID);
  dump_flag(cn_flags,CN_F_INVALID);
  dump_flag(cn_flags,CN_F_PRECISION);
  dump_flag(cn_flags,CN_F_RANGE);
  dump_flag(cn_flags,CN_F_LIMIT);
  dump_flag(cn_flags,CN_F_EXTLIMIT);
  dump_flag(cn_flags,CN_F_DISCRETE);
  dump_flag(cn_flags,CN_F_CALIBRATION);
  dump_flag(cn_flags,CN_F_CALCULATED);
  dump_flag(cn_flags,CN_F_VIRTUAL);
  dump_mem(cn_inval_bit_pos,_T("%u"));
  dump_mem(cn_precision,_T("%u"));
  dump_mem(cn_val_range_min,_T("%g"));
  dump_mem(cn_val_range_max,_T("%g"));
  dump_mem(cn_limit_min,_T("%g"));
  dump_mem(cn_limit_max,_T("%g"));
  dump_mem(cn_limit_ext_min,_T("%g"));
  dump_mem(cn_limit_ext_max,_T("%g"));

  // links
  dump_link(f,cn_cn_next);
  dump_link(f,cn_composition);
  dump_link(f,cn_tx_name);
  dump_link(f,cn_si_source);
  dump_link(f,cn_cc_conversion);
  dump_link(f,cn_data);
  dump_link(f,cn_md_unit);
  dump_link(f,cn_md_comment);

}

//-------------------------------------------------------------------------------------------------------
// ##CC Channel Conversion Block
//-------------------------------------------------------------------------------------------------------
void M4CCBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  static const m4Enumeration enum_cc_type[]=
  {
    _e(CC_T_IDENTITY)       // 1:1 conversion (in this case, the CCBLOCK simply can be omitted)
    _e(CC_T_LINEAR)         // linear conversion
    _e(CC_T_RATIONAL)       // rational conversion
    _e(CC_T_FORMULA)        // algebraic conversion (MCD-2 MC text formula)
    _e(CC_T_N2N_INTERPOL)   // value to value tabular look-up with interpolation
    _e(CC_T_N2N)            // value to value tabular look-up without interpolation
    _e(CC_T_R2N)            // value range to value tabular look-up
    _e(CC_T_N2T)            // value to text/scale conversion tabular look-up
    _e(CC_T_R2T)            // value range to text/scale conversion tabular look-up
    _e(CC_T_T2N)            // text to value tabular look-up
    _e(CC_T_T2T)            // text to text tabular look-up (translation)
    { NULL,0}
  };
  m4Block::dump(f,Tree);
  // memory
  dump_enum(cc_type,enum_cc_type);
  dump_mem(cc_flags,_T("%u"));
  dump_flag(cc_flags,CC_F_PRECISION);
  dump_flag(cc_flags,CC_F_RANGE);
  dump_flag(cc_flags,CC_F_STATUS);
  dump_mem(cc_ref_count,_T("%u"));
  dump_mem(cc_val_count,_T("%u"));
  dump_mem(cc_phy_range_min,_T("%g"));
  dump_mem(cc_phy_range_max,_T("%g"));
  for(M_UINT32 i=0; i<cc_val_count; i++)
  {
    M_REAL v=*m_var.get(i);
		M_CHAR szBuffer[128];
		sprintf(szBuffer,"val[%u]=%g\r\n",i,v);
    strcat(Tree->m_Definition,szBuffer);
  }
  // links
  dump_link(f,cc_tx_name);
  dump_link(f,cc_md_unit);
  dump_link(f,cc_md_comment);
  dump_link_(f,cc_cc_inverse);
  M_UINT32 lno=(M_UINT32)cc_ref;
  for(M_UINT32 i=0; i<cc_ref_count;i++)
  {
		M_CHAR szBuffer[40];
    M_LINK at=getLink(lno+i);
    sprintf(szBuffer,"ref[%u]",i);
    m4TreeItem *t=Tree->m_Tree.append(new m4TreeLink(szBuffer,at));
    if (at)
    {
      m4Block *b=f.LoadBlock(at);
      b->dump(f,t);
      delete b;
    }
    else
    {
      strcpy(t->m_Definition,"-NIL-");
    }
  }
}

//-------------------------------------------------------------------------------------------------------
// ##DL Data List Block
//-------------------------------------------------------------------------------------------------------
void M4DLBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
  // memory
  dump_mem(dl_flags,_T("%u"));
  dump_flag(dl_flags,DL_F_EQUAL);
  dump_mem(dl_count,_T("%u"));
  if (dl_flags&DL_F_EQUAL)
  {
    M_UINT64 eql=*get(0);
    dump_var(_T("dl_equal_length=%llu"),eql);
  }
  else
  {
    for(M_UINT32 i=0; i<dl_count; i++)
    {
			M_CHAR szBuffer[128];
      M_UINT64 ofs=*get(i);
      sprintf(szBuffer,"dl_offset[%u]=%llu\r\n",i,ofs);
      strcat(Tree->m_Definition,szBuffer);
    }
  }
  // links
  dump_link(f,dl_dl_next);
  M_UINT32 lno=(M_UINT32)dl_data;
  for(M_UINT32 i=0; i<dl_count;i++)
  {
		M_CHAR szBuffer[40];
    M_LINK at=getLink(lno+i);
    sprintf(szBuffer,"data[%u]",i);
    m4TreeItem *t=Tree->m_Tree.append(new m4TreeLink(szBuffer,at));
    if (at)
    {
      m4Block *b=f.LoadBlock(at);
      b->dump(f,t);
      delete b;
    }
    else
    {
      strcpy(t->m_Definition,"-NIL-");
    }
  }
}

//-------------------------------------------------------------------------------------------------------
// ##DT Data Block (use by ##SD ##RD ##SR also)
//-------------------------------------------------------------------------------------------------------
void M4DataBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
	M_CHAR szBuffer[128];
	M_CHAR strLine[2048];
  m4Block::dump(f,Tree);
  sprintf(szBuffer,"dl_data_size=%llu\r\n",m_wrtSize);
	strcat(Tree->m_Definition,szBuffer);
  // Add 5 lines of hexdump
  M_UINT32 szDump=5*30;
  M_UINT32 iDump=0;
  M_UINT32 *pDump=(M_UINT32 *)m_Buffer;
  if (szDump>m_wrtSize) szDump=(M_UINT32)m_wrtSize;
	*strLine=0;
  while (szDump--)
  {
		sprintf(szBuffer,"%02X ",*pDump++);
    strcat(strLine,szBuffer);
    if (++iDump==30)
    {
	    strcat(strLine,"\r\n");
      strcat(Tree->m_Definition,strLine);
      *strLine=0;
      iDump=0;
    }
  }
  if (iDump)
  {
    strcat(strLine,"\r\n");
    strcat(Tree->m_Definition,strLine);
  }
}
//-------------------------------------------------------------------------------------------------------
// ##DZ Data Block
//-------------------------------------------------------------------------------------------------------
#if _MDF4_1
void M4DZBlockWr::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
}
#endif
//-------------------------------------------------------------------------------------------------------
// ##AT Attachment Block
//-------------------------------------------------------------------------------------------------------
void M4ATBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  m4Block::dump(f,Tree);
  // memory
  dump_mem(at_flags,_T("%u"));
  dump_flag(at_flags,AT_FL_EMBEDDED);
  dump_flag(at_flags,AT_FL_COMPRESSED);
  dump_flag(at_flags,AT_FL_CHECKSUM);
  dump_mem(at_creator_index,_T("%u"));
  dump_mem(at_original_size,_T("%llu"));
  dump_mem(at_embedded_size,_T("%llu"));
  M_CHAR md5[128], szBuffer[128];
	strcpy(md5,"at_md5_checksum=");
  for(int i=0; i<16; i++)
	{
		sprintf(szBuffer,"%02X",at_md5_checksum[i]);
    strcat(md5,szBuffer);
	}
  strcat(md5,"\r\n");
  strcat(Tree->m_Definition,md5);
  // links
  dump_link(f,at_at_next);
  dump_link(f,at_tx_filename);
  dump_link(f,at_tx_mimetype);
  dump_link(f,at_md_comment);
}

//-------------------------------------------------------------------------------------------------------
// ##CH Channel Hierarchy Block
//-------------------------------------------------------------------------------------------------------
void M4CHBlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  static const m4Enumeration enum_ch_type[]=
  {
    _e(CH_T_GROUP)
    _e(CH_T_FUNCTION)
    _e(CH_T_STRUCTURE)
    _e(CH_T_MAPLIST)
    _e(CH_T_PARAMETER)
    _e(CH_T_RESULT)
    _e(CH_T_LOCAL)
    _e(CH_T_CALDEF)
    _e(CH_T_CALREF)
    { NULL,0}
  };
  m4Block::dump(f,Tree);
  // memory
  dump_mem(ch_element_count,_T("%u"));
  dump_enum(ch_type,enum_ch_type);
  // links
  dump_link(f,ch_ch_next);
  dump_link(f,ch_ch_first);
  dump_link(f,ch_tx_name);
  dump_link(f,ch_md_comment);
  M_UINT32 lno=(M_UINT32)ch_element;
  for(M_UINT32 i=0; i<3*ch_element_count;i++)
  {
    M_LINK at=getLink(lno+i);
		M_CHAR szBuffer[128];
    sprintf(szBuffer,"element[%u]",i);
    m4TreeItem *t=Tree->m_Tree.append(new m4TreeLink(szBuffer,at));
    if (at)
    {
      m4Block *b=f.LoadBlock(at);
      b->dump(f,t);
      delete b;
    }
    else
    {
      strcpy(t->m_Definition,"-NIL-");
    }
  }
}
//-------------------------------------------------------------------------------------------------------
// ##CA Channel Array Block
//-------------------------------------------------------------------------------------------------------
void M4CABlock::dump(MDF4File &f,m4TreeItem *Tree)
{
  static const m4Enumeration enum_ca_type[]=
  {
    _e(CA_T_ARRAY)
    _e(CA_T_AXIS)
    _e(CA_T_LOOKUP)
    { NULL,0}
  };
  static const m4Enumeration enum_ca_storage[]=
  {
    _e(CA_S_CN_TEMPLATE)
    _e(CA_S_CG_TEMPLATE)
    _e(CA_S_DG_TEMPLATE)
    { NULL,0}
  };
  m4Block::dump(f,Tree);
  // memory
  dump_enum(ca_type,enum_ca_type);
  dump_enum(ca_storage,enum_ca_storage);
  dump_mem(ca_ndim,_T("%u"));
  dump_mem(ca_flags,_T("%u"));
  dump_flag(ca_flags,CA_F_DYNAMIC);
  dump_flag(ca_flags,CA_F_INPUT);
  dump_flag(ca_flags,CA_F_OUTPUT);
  dump_flag(ca_flags,CA_F_COMPARISON);
  dump_flag(ca_flags,CA_F_AXIS);
  dump_flag(ca_flags,CA_F_FIXED);
  dump_flag(ca_flags,CA_F_INVERSE);
  // links
  dump_link(f,ca_composition);
  M_UINT32 i,lno=(M_UINT32)ca_data;
  for(i=0;  lno<m_Links.getSize();lno++)
  {
		M_CHAR szBuffer[128];
    M_LINK at=getLink(lno);
    sprintf(szBuffer,"element[%u]",i++);
    m4TreeItem *t=Tree->m_Tree.append(new m4TreeLink(szBuffer,at));
    if (at)
    {
      m4Block *b=f.LoadBlock(at);
      b->dump(f,t);
      delete b;
    }
    else
    {
      strcpy(t->m_Definition,"-NIL-");
    }
  }
}

void MDF4File::dump(m4TreeItem *Tree)
{
	M_CHAR szBuffer[256];
  char f[10],v[10],p[10];
  memset(f,0,sizeof(f));
  memset(v,0,sizeof(v));
  memset(p,0,sizeof(p));
  memcpy(f,m_Id.id_file,8);
  memcpy(v,m_Id.id_vers,8);
  memcpy(p,m_Id.id_prog,8);
  sprintf(szBuffer,"ID:\r\nid_file: %s\r\nid_vers: %s\r\nid_prog: %s\r\nid_order=%u\r\nid_float=%u\r\nid_ver=%u\r\n\r\n",
    f,v,p,m_Id.id_order,m_Id.id_float,m_Id.id_ver);
  strcpy(Tree->m_Definition,szBuffer);
  m_Hdr.dump(*this,Tree);
}

bool m4TreeFile::Open(LPCTSTR Path)
{
  if (m_m4.Open(Path))
  {
    m_m4.dump(this);
    return true;
  }
  return false;
}
