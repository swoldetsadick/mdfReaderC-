
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

/*********************************************************************************************
  Copyrights of ASAM e.V.

  This library is based on the ASAM MDF 4.1.0 standard from 2012-11-20.

  Most namings and comments are copied from this standard to allow a clear understanding of the 
  context.

  If you are not a member of ASAM e.V., you may buy the MDF Standard from ASAM e.V. (please 
  contact mdf@asam.net)
***********************************************************************************************/

#pragma once

#include <map>
#include "mdfConfig.h"
#include "dynArray.h"
#include "mdFile.h"
#include "utf8.h"
#include <assert.h>

#define MDF4LIB_VERSION L"2.020 2016/06/20"
extern M_UNICODE MDF4LibGetVersionString();

#if _MDF4_MD5       // [B]---- MD5 Support
#  include "md5.h"
#endif              // [E]---- MD5 Support
#if _MDF4_XML       // [B]---- MD5 MSXML Support
#  import <msxml6.dll> auto_rename 
   using namespace MSXML2;
#endif              // [E]---- MD5 MSXML Support

#if !_MDF4_WIN
	#define ATLTRACE
	#define TRACE
#endif

#if _MDF4_DUMP      // [B]---- DUMP Support
class m4TreeItem;
#define DECLARE_DUMP   virtual void dump(MDF4File &f,m4TreeItem *Tree);
#else
#define DECLARE_DUMP
#endif              // [E]---- DUMP Support

#define M4ID_ID MDF_ID('#','#')  // common ID prefix
#define M4ID_HD MDF_ID('H','D')  // ##HD HeaDer Block
#define M4ID_MD MDF_ID('M','D')  // ##MD Meta Data Block
#define M4ID_TX MDF_ID('T','X')  // ##TX TeXt Block
#define M4ID_FH MDF_ID('F','H')  // ##FH File History Block
#define M4ID_CH MDF_ID('C','H')  // ##CH Channel Hierarchy Block
#define M4ID_AT MDF_ID('A','T')  // ##AT ATtachment Block
#define M4ID_EV MDF_ID('E','V')  // ##EV EVent Block
#define M4ID_DG MDF_ID('D','G')  // ##DG Data Group Block
#define M4ID_CG MDF_ID('C','G')  // ##CG Channel Group Block
#define M4ID_SI MDF_ID('S','I')  // ##SI Source Information Block
#define M4ID_CN MDF_ID('C','N')  // ##CN ChaNnel Block
#define M4ID_CC MDF_ID('C','C')  // ##CC Channel Conversion Block
#define M4ID_CA MDF_ID('C','A')  // ##CA Channel Array Block
#define M4ID_DT MDF_ID('D','T')  // ##DT DaTa Block
#define M4ID_SR MDF_ID('S','R')  // ##SR Signal Reduction Block
#define M4ID_RD MDF_ID('R','D')  // ##RD Reduction Data Block
#define M4ID_SD MDF_ID('S','D')  // ##SD Signal Data Block
#define M4ID_DL MDF_ID('D','L')  // ##DL Data List Block
#define M4ID_DZ MDF_ID('D','Z')  // ##DZ Data Zipped Block
#define M4ID_HL MDF_ID('H','L')  // ##HL Header List Block



//-------------------------------------------------------------------------------------------------------
// MDF4 - common Header
//-------------------------------------------------------------------------------------------------------
struct m4BlockHdr
{
  M_UINT16 hdr_hdr;   // '##'
  M_UINT16 hdr_id;    // 'XX'
  M_UINT32 hdr_gap;   // reserved, must be 0
  M_UINT64 hdr_len;   // Length of block in bytes
  M_UINT64 hdr_links; // # of links 
};




//-------------------------------------------------------------------------------------------------------
// ##MD Meta Data  Block
//-------------------------------------------------------------------------------------------------------
// The MDBLOCK contains information encoded as XML string. For example this can be comments for the 
// measured data file, file history information or the identification of a channel. This information 
// is ruled by the parent block and follows specific XML schemas definitions.
//-------------------------------------------------------------------------------------------------------
// ##TX Text Block
//-------------------------------------------------------------------------------------------------------
// The TXBLOCK is very similar to the MDBLOCK but only contains a plain string encoded in UTF-8. 
// The text length results from the block size.
//-------------------------------------------------------------------------------------------------------
// Note:
// Both do NOT have any LINK section and the only data member is the embedded zero-terminated UTF-8 string
//-------------------------------------------------------------------------------------------------------

// ##HD Header Block
//-------------------------------------------------------------------------------------------------------
// The HDBLOCK always begins at file position 64. It contains general information about the contents
// of the measured data file and is the root for the block hierarchy
//-------------------------------------------------------------------------------------------------------

// Bit definition for member hd_time_flags
#define HD_TF_LOCALTIME  BIT0 // Local time flag If set, the start time stamp in nanoseconds represents the local time instead of the UTC time, In this case, time zone and DST offset must not be considered (time offsets flag must not be set). Should only be used if UTC time is unknown. If the bit is not set (default), the start time stamp represents the UTC time.
#define HD_TF_TIMEOFFS   BIT1 // Time offsets valid flag If set, the time zone and DST offsets are valid. Must not be set together with "local time" flag (mutually exclusive). If the offsets are valid, the locally displayed time at start of recording can be determined (after conversion of offsets to ns) by Local time = UTC time + time zone offset + DST offset.

// enumeration for member hd_time_class
#define HD_TC_LOCALPC    0    // local PC reference time (Default)
#define HD_TC_EXTERN    10    // external time source
#define HD_TC_EXTABS    16    // external absolute synchronized time

// Bit definition for member hd_flags
#define HD_FL_ANGLE     BIT0  // Start angle valid flag If set, the start angle value below is valid.
#define HD_FL_DISTANCE  BIT1  // Start distance valid flag If set, the start distance value below is valid.

// helper struct for HD and FH records
#pragma pack( push, 1)  // Enter BYTE packing
struct M_DATE
{
  M_UINT64 time_ns;  // Time stamp in nanoseconds elapsed since 00:00:00 01.01.1970 (UTC time or local time, depending on "local time" flag, see [UTC]).
  M_INT16  tz_offset_min;  // Time zone offset in minutes. The value must be in range [-720,720], i.e. it can be negative! For example a value of 60 (min) means UTC+1 time zone = Central European Time (CET). Only valid if "time offsets valid" flag is set in time flags.
  M_INT16  dst_offset_min; // Daylight saving time (DST) offset in minutes for start time stamp. During the summer months, most regions observe a DST offset of 60 min (1 hour). Only valid if "time offsets valid" flag is set in time flags.
  M_UINT8  time_flags;     // Time flags The value contains the following bit flags (see HD_TF_xxx)
#if _MDF4_WIN
  static void set(M_DATE &d,const FILETIME &ft);
  static void now(M_DATE &d);
  static void get(const M_DATE &d,FILETIME &ft);
  static void get(const M_DATE &d,SYSTEMTIME &st);
#endif
};
#pragma pack(pop)


struct  m4HDRecord
{
  // Block Header
  enum { RID=M4ID_HD };
  // enumeration of links
  enum
  {
    hd_dg_first, // Pointer to the first data group block (DGBLOCK) (can be NIL)
    hd_fh_first, // Pointer to first file history block (FHBLOCK) 
                 // There must be at least one FHBLOCK with information about the application which created the MDF file.
    hd_ch_first, // Pointer to first channel hierarchy block (CHBLOCK) (can be NIL).
    hd_at_first, // Pointer to first attachment block (ATBLOCK) (can be NIL)
    hd_ev_first, // Pointer to first event block (EVBLOCK) (can be NIL)
    hd_md_comment, // Pointer to the measurement file comment (TXBLOCK or MDBLOCK) (can be NIL) For MDBLOCK contents, see Table 14.
    LinkMax      // # of known links
  };
  // Data members
  M_DATE   hd_start_time;     // Time stamp at start of measurement in nanoseconds elapsed since 00:00:00 01.01.1970 (UTC time or local time, depending on "local time" flag, see [UTC]). All time stamps for time synchronized master channels or events are always relative to this start time stamp.
  M_UINT8  hd_time_class;     // Time quality class (see HD_TC_xxx)
  M_UINT8  hd_flags;          // Flags The value contains the following bit flags (see HD_FL_xxx):
  M_UINT8  hd_reserved;       // Reserved
  M_REAL   hd_start_angle_rad; // Start angle in radians at start of measurement (only for angle synchronous measurements) Only valid if "start angle valid" flag is set. All angle values for angle synchronized master channels or events are relative to this start angle.
  M_REAL   hd_start_distance_m; // Start distance in meters at start of measurement (only for distance synchronous measurements) Only valid if "start distance valid" flag is set. All distance values for distance synchronized master channels or events are relative to this start distance.
};

//-------------------------------------------------------------------------------------------------------
// ##FH File History
//-------------------------------------------------------------------------------------------------------
// The FHBLOCK describes who/which tool generated or changed the MDF file. 
// Each FHBLOCK contains a change log entry for the MDF file. 
// The first FHBLOCK referenced by the HDBLOCK must contain information about the tool which 
// created the MDF file. Starting from this FHBLOCK then a linked list of FHBLOCKs can be maintained 
// with a chronological change history, i.e. any other application that changes the file must append 
// a new FHBLOCK to the list. A zero-based index value is used to reference blocks within this linked list,
// thus the ordering of the blocks must not be changed.
//-------------------------------------------------------------------------------------------------------

// Bit definitions for member fh_time_flags
#define FH_TF_LOCALTIME  BIT0 // Local time flag If set, the start time stamp in nanoseconds represents the local time instead of the UTC time, In this case, time zone and DST offset must not be considered (time offsets flag must not be set). Should only be used if UTC time is unknown. If the bit is not set (default), the start time stamp represents the UTC time.
#define FH_TF_TIMEOFFS   BIT1 // Time offsets valid flag If set, the time zone and DST offsets are valid. Must not be set together with "local time" flag (mutually exclusive). If the offsets are valid, the locally displayed time at start of recording can be determined (after conversion of offsets to ns) by Local time = UTC time + time zone offset + DST offset.

typedef struct
{
  // Block Header
  enum { RID=M4ID_FH };
  // enumeration of links
  enum
  {
    fh_fh_next, // Link to next FHBLOCK (can be NIL if list finished)
    fh_md_comment, // Link to MDBLOCK containing comment about the creation or modification of the MDF file.
    LinkMax       // # of known links
  };
  // Data members
  M_DATE   fh_time;
  M_UINT8  fh_reserved[3]; // Reserved
} m4FHRecord;


//-------------------------------------------------------------------------------------------------------
// ##CH Channel Hierarchy Block
//-------------------------------------------------------------------------------------------------------
// The CHBLOCKs describe a logical ordering of the channels in a tree-like structure. This only serves to
// structure the channels and is totally independent to the data group and channel group structuring. 
// A channel even may not be referenced at all or more than one time. Each CHBLOCK can be seen as a node
// in a tree which has a number of channels as leafs and which has a reference to its next sibling and its
// first child node (both CHBLOCKs). 
// The reference to a channel is always a triple link to the CNBLOCK of the channel and its parent 
// CGBLOCK and DGBLOCK. Each CHBLOCK can have a name.
//-------------------------------------------------------------------------------------------------------

// enumeration for member ch_type
#define CH_T_GROUP     0  // All elements and children of this hierarchy level form a logical group (see [MCD-2 MC] keyword GROUP).
#define CH_T_FUNCTION  1  // All children of this hierarchy level form a functional group (see [MCD-2 MC] keyword FUNCTION) For this type, the hierarchy must not contain CNBLOCK references (ch_element_count must be 0).
#define CH_T_STRUCTURE 2  // All elements and children of this hierarchy level form a "fragmented" structure, see 4.18.1 Structures. Note: Do not use "fragmented" and "compact" structure in parallel. If possible prefer a "compact" structure.
#define CH_T_MAPLIST   3  // All elements of this hierarchy level form a map list (see [MCD-2 MC] keyword MAP_LIST): the first element represents the z axis (must be a curve, i.e. CNBLOCK with CABLOCK of type "axis") all other elements represent the maps (must be 2-dimensional map, i.e. CNBLOCK with CABLOCK of type "look-up")
#define CH_T_PARAMETER 4  // input variables of function (see [MCD-2 MC] keyword IN_MEASUREMENT) All referenced channels must be measurement objects ("calibration" flag (bit 7) not set in cn_flags)
#define CH_T_RESULT    5  // output variables of function (see [MCD-2 MC] keyword OUT_MEASUREMENT) All referenced channels must be measurement objects ("calibration" flag (bit 7) not set in cn_flags)
#define CH_T_LOCAL     6  // local variables of function (see [MCD-2 MC] keyword LOC_MEASUREMENT) All referenced channels must be measurement objects ("calibration" flag (bit 7) not set in cn_flags)
#define CH_T_CALDEF    7  // calibration objects defined in function (see [MCD-2 MC] keyword DEF_CHARACTERISTIC) All referenced channels must be calibration objects ("calibration" flag (bit 7) set in cn_flags)
#define CH_T_CALREF    8  // calibration objects referenced in function (see [MCD-2 MC] keyword REF_CHARACTERISTIC) All referenced channel must be calibration objects ("calibration" flag (bit 7) set in cn_flags)

typedef struct
{
  // Block Header
  enum { RID=M4ID_CH };
  // enumeration of links
  enum
  {
    ch_ch_next,  // Link to next sibling CHBLOCK (can be NIL)
    ch_ch_first, // Link to first child CHBLOCK (can be NIL, must be NIL for ch_type = 3 ("map list")).
    ch_tx_name,  // Link to TXBLOCK with the name of the hierarchy level. Must be NIL for ch_type ≥ 4, must not be NIL for all other types.
                 //  If specified, the name must be according to naming rules stated in 4.4.2 Naming Rules, and it must be unique within all sibling CHBLOCKs.
    ch_md_comment, // Link to TXBLOCK or MDBLOCK with comment and other information for the hierarchy level (can be NIL)
    LinkMax,      // # of known links
    ch_element=LinkMax   // N x 3 (DG,CG,CN)
  } ;
  // Data menbers
  M_UINT32  ch_element_count; // Number of channels N referenced by this CHBLOCK.
  M_UINT8   ch_type;          // Type of hierarchy level: (see CH_T_xxx))
  M_BYTE    ch_reserved[3];   // Reserved
} m4CHRecord;

//-------------------------------------------------------------------------------------------------------
// ##AT Attachment Block
//-------------------------------------------------------------------------------------------------------
// The ATBLOCK specifies attached data, either by referencing an external file or by embedding the data in 
// the MDF file. Embedding the data, it optionally can be compressed using the GNU zip algorithm
//-------------------------------------------------------------------------------------------------------

// Bit definitions for member at_flags
#define AT_FL_EMBEDDED   BIT0  // Embedded data flag If set, the attachment data is embedded, otherwise it is contained in an external file referenced by file path and name in at_tx_filename.
#define AT_FL_COMPRESSED BIT1  // Compressed embedded data flag If set, the embedded data is compressed using the gzip algorithm. Can only be set if "embedded data" flag (bit 0) is set.
#define AT_FL_CHECKSUM   BIT2  // at_md5_checksum is valid

typedef struct
{
  // Block Header
  enum { RID=M4ID_AT };
  // enumeration of links
  enum
  {
    at_at_next, // Link to next ATBLOCK (linked list) (can be NIL)
    at_tx_filename, // Link to TXBLOCK with the path and file name of the embedded or referenced file (can only be NIL if data is embedded). The path of the file can be relative or absolute. If relative, it is relative to the directory of the MDF file. If no path is given, the file must be in the same directory as the MDF file.
    at_tx_mimetype, // Link to TXBLOCK with MIME content-type text that gives information about the attached data. Can be NIL if the content-type is unknown, but should be specified whenever possible. The MIME content-type string must be written in lowercase.
    at_md_comment,   // Link to MDBLOCK with comment and additional information about the attachment (can be NIL).
    LinkMax      // # of known links
  };
  // Data menbers
  M_UINT16   at_flags;         // Flags The value contains the following bit flags (see AT_FL_xxx):
  M_UINT16   at_creator_index; // Creator index, i.e. zero-based index of FHBLOCK in global list of FHBLOCKs that specifies which application has created this attachment, or changed it most recently.
  M_BYTE     at_reserved[4];      // Reserved
  M_BYTE     at_md5_checksum[16];  // 128-bit value for MD5 check sum (of the uncompressed data if data is embedded and compressed). Only valid if "MD5 check sum valid" flag (bit 2) is set.
  M_UINT64   at_original_size; // Original data size in Bytes, i.e. either for external file or for uncompressed data.
  M_UINT64   at_embedded_size; // Embedded data size N, i.e. number of Bytes for binary embedded data following this element. Must be 0 if external file is referenced.
  // followed by embedded data ...
} m4ATRecord;


//-------------------------------------------------------------------------------------------------------
// ##EV Event  Block
//-------------------------------------------------------------------------------------------------------
// The EVBLOCK serves to describe an event. Each EVBLOCK stores a synchronization value to specify when 
// the event occurred. Usually this will be a time stamp, but the event also can be synchronized with some
// other master channel type or the record index of a channel group (see 4.4.5 Synchronization Domains).
// Generally an event defines a "point" in time or some other synchronization domain. 
// For some event types, two "points" can be used to define a "range".
//-------------------------------------------------------------------------------------------------------

// enumeration for member ev_type
#define EV_T_RECORDING    0   // Recording
#define EV_T_RECINT       1   // Recording Interrupt
#define EV_T_ACQINT       2   // Acquisition interrupt
#define EV_T_STARTTRG     3   // Start Recording Trigger
#define EV_T_STOPTRG      4   // Stop Recording Trigger
#define EV_T_TRIGGER      5   // Trigger
#define EV_T_MARKER       6   // Marker

// enumeration for member ev_sync_type
#define EV_S_TIME          1   // calculated synchronization value represents time in seconds
#define EV_S_ANGLE         2   // calculated synchronization value represents angle in radians
#define EV_S_DISTANCE      3   // calculated synchronization value represents distance in meter
#define EV_S_INDEX         4   // calculated synchronization value represents zero-based record index

// enumeration for member ev_range_type
#define EV_R_POINT         0   // event defines a point
#define EV_R_START         1   // event defines the beginning of a range
#define EV_R_END           2   // event defines the end of a range

// enumeration for member ev_cause
#define EV_C_OTHER        0    // OTHER cause of event is not known or does not fit into given categories.
#define EV_C_ERROR        1    // ERROR event was caused by some error.
#define EV_C_TOOL         2    // TOOL event was caused by tool-internal condition, e.g. trigger condition or re-configuration.
#define EV_C_SCRIPT       3    // SCRIPT event was caused by a scripting command.
#define EV_C_USER         4    // USER event was caused directly by user, e.g. user input or some other interaction with GUI.

// Bit definitions for member ev_flags
#define EV_F_POST         BIT0 // Post processing flag If set, the event has been generated during post processing of the file.

typedef struct
{
  // Block Header
  enum { RID=M4ID_EV };
  // enumeration of links
  enum
  {
    ev_ev_next,     // Link to next EVBLOCK (linked list) (can be NIL)
    ev_ev_parent,   // Referencing link to EVBLOCK with parent event (can be NIL).
    ev_ev_range,    // Referencing link to EVBLOCK with event that defines the beginning of a range (can be NIL, must be NIL if ev_range_type ≠ 2).
    ev_tx_name,     // Pointer to TXBLOCK with event name (can be NIL) Name must be according to naming rules stated in 4.4.2 Naming Rules. If available, the name of a named trigger condition should be used as event name. Other event types may have individual names or no names.
    ev_md_comment,  // Pointer to TX/MDBLOCK with event comment and additional information, e.g. trigger condition or formatted user comment text (can be NIL)
    LinkMax,        // # of known links
    ev_scope=5,       // List of links to channels and channel groups to which the event applies (referencing links to CNBLOCKs or CGBLOCKs). This defines the "scope" of the event.
    ev_at_reference // List of attachments for this event (references to ATBLOCKs in global linked list of ATBLOCKs). [ev_attachment_count]
  };
  // Data members
  M_UINT8 ev_type; // Event type (see EV_T_xxx)
  M_UINT8 ev_sync_type; // Sync type (see EV_S_xxx)
  M_UINT8 ev_range_type; // Range Type (see EV_R_xxx)
  M_UINT8 ev_cause;      // Cause of event (see EV_C_xxx)
  M_UINT8 ev_flags;      // flags (see EV_F_xxx)
  M_BYTE  ev_reserved[3]; // Reserved
  M_UINT32 ev_scope_count; // Length M of ev_scope list. Can be zero.
  M_UINT16 ev_attachment_count; // Length N of ev_at_reference list, i.e. number of attachments for this event. Can be zero.
  M_UINT16 ev_creator_index; // Creator index, i.e. zero-based index of FHBLOCK in global list of FHBLOCKs that specifies which application has created or changed this event (e.g. when generating event offline).
  M_INT64  ev_sync_base_value; // Base value for synchronization value.
  M_REAL   ev_sync_factor;  // Factor for event synchronization value.
} m4EVRecord;

//-------------------------------------------------------------------------------------------------------
// ##DG Data Group Block
//-------------------------------------------------------------------------------------------------------
// The DGBLOCK gathers information and links related to its data block. Thus the branch in the tree of MDF 
// blocks that is opened by the DGBLOCK contains all information necessary to understand and decode the 
// data block referenced by the DGBLOCK.
// The DGBLOCK can contain several channel groups. In this case the data group (and thus the MDF file) is "unsorted". 
// If there is only one channel group in the DGBLOCK, the data group is "sorted"
//(for details please refer to 4.21.3 Sorted and Unsorted Data).
//-------------------------------------------------------------------------------------------------------

typedef struct
{
  // Block Header
  enum { RID=M4ID_DG };
  // enumeration of links
  enum
  {
    dg_dg_next, // Pointer to next data group block (DGBLOCK) (can be NIL)
    dg_cg_first, // Pointer to first channel group block (CGBLOCK) (can be NIL)
    dg_data,     // Pointer to data block (DTBLOCK or DZBLOCK for this block type) or data list block (DLBLOCK of data blocks or its HLBLOCK)  (can be NIL)
    dg_md_comment, // Pointer to comment and additional information (TXBLOCK or MDBLOCK) (can be NIL)
    LinkMax      // # of known links
  };
  // Data Members
  M_UINT8 dg_rec_id_size; // Number of Bytes used for record IDs in the data block. (allowed: 0,1,2,4,8)
  M_BYTE  dg_reserved[7]; // Reserved
} m4DGRecord;


//-------------------------------------------------------------------------------------------------------
// ##CG Channel Group Block
//-------------------------------------------------------------------------------------------------------
// The CGBLOCK contains a collection of channels which are stored in one record, i.e. which have equal sampling.
// The only exception is a channel group for variable length signal data (VLSD), called "VLSD channel group".
// It has no channel collection and can only occur in an unsorted data group. It describes signal values of 
// variable length which are stored as variable length records in a normal DTBLOCK.
//-------------------------------------------------------------------------------------------------------

// Bit definitions for member ev_flags
#define CG_FL_VLSD         BIT0 //VLSD channel group flag. If set, this is a "variable length signal data" (VLSD) channel group. See explanation in 4.14.3 Variable Length Signal Data (VLSD) CGBLOCK.
#define CG_F_BUSEVENT      BIT1
#define CG_F_PLAINBUSEVENT BIT2

typedef struct
{
  // Block Header
  enum { RID=M4ID_CG };
  // enumeration of links
  enum
  {
     cg_cg_next, // Pointer to next channel group block (CGBLOCK) (can be NIL)
     cg_cn_first, // Pointer to first channel block (CNBLOCK) (can be NIL, must be NIL for VLSD CGBLOCK, i.e. if "VLSD channel group" flag (bit 0) is set)
     cg_tx_acq_name, // Pointer to acquisition name (TXBLOCK) (can be NIL, must be NIL for VLSD CGBLOCK)
     cg_si_acq_source, // Pointer to acquisition source (SIBLOCK) (can be NIL, must be NIL for VLSD CGBLOCK) See also rules for uniqueness explained in 4.4.3 Identification of Channels.
     cg_sr_first, // Pointer to first sample reduction block (SRBLOCK) (can be NIL, must be NIL for VLSD CGBLOCK)
     cg_md_comment, //Pointer to comment and additional information (TXBLOCK or MDBLOCK) (can be NIL, must be NIL for VLSD CGBLOCK)  
     LinkMax      // # of known links
  };
  // Data Members
  M_UINT64  cg_record_id; // Record ID, value must be less than maximum unsigned integer value allowed by dg_rec_id_size in parent DGBLOCK. Record ID must be unique within linked list of CGBLOCKs.
  M_UINT64  cg_cycle_count; // Number of cycles, i.e. number of samples for this channel group. This specifies the number of records of this type in the data block.
  M_UINT16  cg_flags; // Flags The value contains the following bit flags (see CG_F_xx):
	union  {
		M_CHAR    cg_path_separator;
		M_BYTE    cg_reserved[6]; // Reserved.
	} v41;
  M_UINT32  cg_data_bytes; // Normal CGBLOCK: Number of data Bytes (after record ID) used for signal values in record, i.e. size of plain data for each recorded sample of this channel group. VLSD CGBLOCK: Low part of a UINT64 value that specifies the total size in Bytes of all variable length signal values for the recorded samples of this channel group. See explanation for cg_inval_bytes.
  M_UINT32  cg_inval_bytes; // Normal CGBLOCK: Number of additional Bytes for record used for invalidation bits. Can be zero if no invalidation bits are used at all. Invalidation bits may only occur in the specified number of Bytes after the data Bytes, not within the data Bytes that contain the signal values. VLSD CGBLOCK: High part of UINT64 value that specifies the total size in Bytes of all variable length signal values for the recorded samples of this channel group, i.e. the total size in Bytes can be calculated by cg_data_bytes + (cg_inval_bytes << 32) Note: this value does not include the Bytes used to specify the length of each VLSD value!
} m4CGRecord;

//-------------------------------------------------------------------------------------------------------
// ##SI Source Information Block
//-------------------------------------------------------------------------------------------------------
// The SIBLOCK describes the source of an acquisition mode or of a signal. The source information is also 
// used to ensure a unique identification of a channel as explained in section 4.4.3 Identification of Channels.
//-------------------------------------------------------------------------------------------------------

// enumeration for member si_type
#define SI_T_OTHER   0  // OTHER source type does not fit into given categories or is unknown
#define SI_T_ECU     1  // ECU source is an ECU
#define SI_T_BUS     2  // BUS source is a bus (e.g. for bus monitoring)
#define SI_T_DEVICE  3  // I/O source is an I/O device (e.g. analog I/O)
#define SI_T_TOOL    4  // TOOL source is a software tool (e.g. for tool generated signals/events)
#define SI_T_USER    5  //  USER source is a user interaction/input (e.g. for user generated events)

// enumeration for member si_bus_type
#define SI_BUS_NONE  0  // NONE no bus 
#define SI_BUS_OTHER 1  // OTHER bus type does not fit into given categories or is unknown
#define SI_BUS_CAN   2  // CAN 
#define SI_BUS_LIN   3  // LIN 
#define SI_BUS_MOST  4  // MOST 
#define SI_BUS_FLEXR 5  // FLEXRAY 
#define SI_BUS_KLINE 6  // K_LINE 
#define SI_BUS_NET   7  // ETHERNET 
#define SI_BUS_USB   8  // USB
                        // Vender defined bus types can be added starting with value 128.


// Bit definitions for member si_flags
#define SI_FL_SIMULATED  BIT0  //simulated source Source is only a simulation (can be hardware or software simulated) Cannot be set for si_type = 4 (TOOL).

typedef struct
{
  // Block Header
  enum { RID=M4ID_SI };
  // enumeration of links
  enum
  {
    si_tx_name, // Pointer to TXBLOCK with name (identification) of source (must not be NIL). The source name must be according to naming rules stated in 4.4.2 Naming Rules.
    si_tx_path, // Pointer to TXBLOCK with (tool-specific) path of source (can be NIL). The path string must be according to naming rules stated in 4.4.2 Naming Rules.
                // Each tool may generate a different path string. The only purpose is to ensure uniqueness as explained in section 4.4.3 Identification of Channels. As a recommendation, the path should be a human readable string containing additional information about the source. However, the path string should not be used to store this information in order to retrieve it later by parsing the string. Instead, additional source information should be stored in generic or custom XML fields in the comment MDBLOCK si_md_comment.
    si_md_comment, // Pointer to source comment and additional information (TXBLOCK or MDBLOCK) (can be NIL)  
    LinkMax      // # of known links

  };
  // Data Members
  M_UINT8   si_type; // Source type additional classification of source (see SI_T_xxx)
  M_UINT8   si_bus_type; // Bus type additional classification of used bus (should be 0 for si_type ≥ 3) (see SI_BUS_xxx)
  M_UINT8   si_flags;    // Flags The value contains the following bit flags (see SI_F_xxx)):
  M_BYTE    si_reserved[5]; //reserved
} m4SIRecord;


//-------------------------------------------------------------------------------------------------------
// ##CN Channel Block
//-------------------------------------------------------------------------------------------------------
// The CNBLOCK describes a channel, i.e. it contains information about the recorded signal and how its 
// signal values are stored in the MDF file.
//-------------------------------------------------------------------------------------------------------

// enumeration for member cn_type
#define CN_T_FIXEDLEN      0  // fixed length data channel
#define CN_T_VLSD          1  // variable length data channel
#define CN_T_MASTER        2  // master channel for all signals of this group
#define CN_T_VMASTER       3  // virtual master channel
#define CN_T_SYNC          4  // synchronization channel
#define CN_T_MAXLENDATACN  5  // maximum length data channel (4.1)
#define CN_T_VIRTUALDATACN 6  // virtual data channel (4.1)

// enumeration for member cn_sync_type
#define CN_S_NONE      0  // None (to be used for normal data channels)
#define CN_S_TIME      1  // Time (physical values must be seconds) 
#define CN_S_ANGLE     2  // Angle (physical values must be radians)
#define CN_S_DISTANCE  3  // Distance (physical values must be meters)
#define CN_S_INDEX     4  // Index (physical values must be zero-based index values)

// enumeration for member cn_data_type
#define CN_D_UINT_LE   0  // Unsigned Integer LE Byte Order
#define CN_D_UINT_BE   1  // Unsigned Integer BE Byte Order
#define CN_D_SINT_LE   2  // Signed Integer LE Byte Order
#define CN_D_SINT_BE   3  // Snsigned Integer BE Byte Order
#define CN_D_FLOAT_LE  4  // Float (IEEE 754) LE Byte Order
#define CN_D_FLOAT_BE  5  // Float (IEEE 754) BE Byte Order
#define CN_D_STRING    6  // String (ISO-8859-1 Latin), NULL terminated)
#define CN_D_UTF8      7  // String (UTF8-encoded), NULL terminated)
#define CN_D_UTF16_LE  8  // String (UTF16-LE Byte order), NULL terminated)
#define CN_D_UTF16_BE  9  // String (UTF16-BE Byte order), NULL terminated)
#define CN_D_BYTES    10  // Byte array
#define CN_D_SAMPLE   11  // MIME sample
#define CN_D_STREAM   12  // MIME stream
#define CN_D_CODATE   13  // CANOpen Date
#define CN_D_COTIME   14  // CANOpen Time

// Bit definitions for member cn_flags

#define CN_F_ALLINVALID   BIT0  // All values invalid flag If set, all values of this channel are invalid. If in addition an invalidation bit is used (bit 1 set), then the value of the invalidation bit must be set (high) for every value of this channel. Must not be set for a master channel (channel types 2 and 3).
#define CN_F_INVALID      BIT1  // Invalidation bit valid flag If set, this channel uses an invalidation bit (position specified by cn_inval_bit_pos). Must not be set if cg_inval_bytes is zero. Must not be set for a master channel (channel types 2 and 3).
#define CN_F_PRECISION    BIT2  // Precision valid flag If set, the precision value for display of floating point values specified in cn_precision is valid and overrules a possibly specified precision value of the conversion rule (cc_precision).
#define CN_F_RANGE        BIT3  // Value range valid flag If set, both the minimum and the maximum raw value that occurred for this signal within the samples recorded in this file are known and stored in cn_val_range_min and cn_val_range_max. Otherwise the two fields are not valid. Note: the raw value range can only be expressed for numeric channel data types (cn_data_type ≤ 5). For all other data types, the flag must not be set.
#define CN_F_LIMIT        BIT4  // Limit range valid flag If set, the limits of the signal value are known and stored in cn_limit_min and cn_limit_max. Otherwise the two fields are not valid. (see [MCD-2 MC] keywords LowerLimit/UpperLimit for MEASUREMENT and CHARACTERISTIC)
#define CN_F_EXTLIMIT     BIT5  // Extended limit range valid flag If set, the extended limits of the signal value are known and stored in cn_limit_ext_min and cn_limit_ext_max. Otherwise the two fields are not valid. (see [MCD-2 MC] keyword EXTENDED_LIMITS) See also remarks for "limit range valid" flag (bit 4).
#define CN_F_DISCRETE     BIT6  // Discrete value flag If set, the signal values of this channel are discrete and must not be interpolated. (see [MCD-2 MC] keyword DISCRETE)
#define CN_F_CALIBRATION  BIT7  // Calibration flag If set, the signal values of this channel correspond to a calibration object, otherwise to a measurement object (see [MCD-2 MC] keywords MEASUREMENT and CHARACTERISTIC)
#define CN_F_CALCULATED   BIT8  // Calculated flag; If set, the values of this channel have been calculated from other channel inputs. (see [MCD-2 MC] keywords VIRTUAL and DEPENDENT_CHARACTERISTIC) In MDBLOCK for cn_md_comment the used input signals and the calculation formula can be documented, see Table 37.
#define CN_F_VIRTUAL      BIT9  // Virtual flag If set, this channel is virtual, i.e. it is simulated by the recording tool. (see [MCD-2 MC] keywords VIRTUAL and VIRTUAL_CHARACTERISTIC) Note: for a virtual measurement according to [MCD-2 MC] both the "Virtual" flag (bit 9) and the "Calculated" flag (bit 8) should be set.
#define CN_F_BUSEVENT     BIT10 // 4.1: Bus event flag. If set, this channel contains information about a bus event. 
#define CN_F_MONOTONOUS   BIT11 // 4.1: Monotonous flag. If set, this channel contains only strictly monotonous increasing/decreasing values. The flag is optional. 
#define CN_F_XAXIS        BIT12 // 4.1: Default X axis flag. If set, a channel to be preferably used as X axis is specified by cn_default_x. This is only a recommendation, a tool may choose to use a different X axis.

// Numerical equivalence for montony
#define CN_MON_NOTDEFINED       0
#define CN_MON_DECREASE         1
#define CN_MON_INCREASE         2
#define CN_MON_STRICT_DECREASE  3
#define CN_MON_STRICT_INCREASE  4
#define CN_MONOTONOUS           5
#define CN_STRICT_MON           6
#define CN_NOT_MON              7

typedef struct
{
  // Block Header
  enum { RID=M4ID_CN };
  // enumeration of links
  enum
  {
    cn_cn_next, // Pointer to next channel block (CNBLOCK) (can be NIL)
    cn_composition, // Composition of channels: Pointer to channel array block (CABLOCK) or channel block (CNBLOCK) (can be NIL). Details see 4.18 Composition of Channels
    cn_tx_name, // Pointer to TXBLOCK with name (identification) of channel. Name must be according to naming rules stated in 4.4.2 Naming Rules.
    cn_si_source, // Pointer to channel source (SIBLOCK) (can be NIL) Must be NIL for component channels (members of a structure or array elements) because they all must have the same source and thus simply use the SIBLOCK of their parent CNBLOCK (direct child of CGBLOCK).
    cn_cc_conversion, // Pointer to the conversion formula (CCBLOCK) (can be NIL, must be NIL for complex channel data types, i.e. for cn_data_type ≥ 10). If the pointer is NIL, this means that a 1:1 conversion is used (phys = int).  };
    cn_data, // Pointer to channel type specific signal data For variable length data channel (cn_type = 1): unique link to signal data block (SDBLOCK) or data list block (DLBLOCK) or, only for unsorted data groups, referencing link to a VLSD channel group block (CGBLOCK). Can only be NIL if SDBLOCK would be empty. For synchronization channel (cn_type = 4): referencing link to attachment block (ATBLOCK) in global linked list of ATBLOCKs starting at hd_at_first. Cannot be NIL.
    cn_md_unit, // Pointer to TXBLOCK/MDBLOCK with designation for physical unit of signal data (after conversion) or (only for channel data types "MIME sample" and "MIME stream") to MIME context-type text. (can be NIL). The unit can be used if no conversion rule is specified or to overwrite the unit specified for the conversion rule (e.g. if a conversion rule is shared between channels). If the link is NIL, then the unit from the conversion rule must be used. If the content is an empty string, no unit should be displayed. If an MDBLOCK is used, in addition the A-HDO unit definition can be stored, see Table 38. Note: for (virtual) master and synchronization channels the A-HDO definition should be omitted to avoid redundancy. Here the unit is already specified by cn_sync_type of the channel. In case of channel data types "MIME sample" and "MIME stream", the text of the unit must be the content-type text of a MIME type which specifies the content of the values of the channel (either fixed length in record or variable length in SDBLOCK). The MIME content-type string must be written in lowercase, and it must apply to the same rules as defined for at_tx_mimetype in 4.11 The Attachment Block ATBLOCK.
    cn_md_comment, // Pointer to TXBLOCK/MDBLOCK with comment and additional information about the channel, see Table 37. (can be NIL)
    LinkMax      // # of known links
  };
  // Data Members
  M_UINT8 cn_type; // Channel type (see CN_T_xxx)
  M_UINT8 cn_sync_type; // Sync type: (see CN_S_xxx)
  M_UINT8 cn_data_type; // Channel data type of raw signal value (see CN_DT_xxx)
  M_UINT8 cn_bit_offset; // Bit offset (0-7): first bit (=LSB) of signal value after Byte offset has been applied (see 4.21.4.2 Reading the Signal Value). If zero, the signal value is 1-Byte aligned. A value different to zero is only allowed for Integer data types (cn_data_type ≤ 3) and if the Integer signal value fits into 8 contiguous Bytes (cn_bit_count + cn_bit_offset ≤ 64). For all other cases, cn_bit_offset must be zero.
  M_UINT32 cn_byte_offset; // Offset to first Byte in the data record that contains bits of the signal value. The offset is applied to the plain record data, i.e. skipping the record ID.
  M_UINT32 cn_bit_count; // Number of bits for signal value in record
  M_UINT32 cn_flags;     // Flags (see CN_F_xxx)
  M_UINT32 cn_inval_bit_pos; // Position of invalidation bit.
  M_UINT8  cn_precision; // Precision for display of floating point values. 0xFF means unrestricted precision (infinite). Any other value specifies the number of decimal places to use for display of floating point values. Only valid if "precision valid" flag (bit 2) is set
  M_BYTE   cn_reserved[3]; // Reserved
  M_REAL cn_val_range_min; // Minimum signal value that occurred for this signal (raw value) Only valid if "value range valid" flag (bit 3) is set.
  M_REAL cn_val_range_max; // Maximum signal value that occurred for this signal (raw value) Only valid if "value range valid" flag (bit 3) is set.
  M_REAL cn_limit_min;    // Lower limit for this signal (physical value for numeric conversion rule, otherwise raw value) Only valid if "limit range valid" flag (bit 4) is set.
  M_REAL cn_limit_max;    // Upper limit for this signal (physical value for numeric conversion rule, otherwise raw value) Only valid if "limit range valid" flag (bit 4) is set.
  M_REAL cn_limit_ext_min; // Lower extended limit for this signal (physical value for numeric conversion rule, otherwise raw value) Only valid if "extended limit range valid" flag (bit 5) is set.
  M_REAL cn_limit_ext_max; // Upper extended limit for this signal (physical value for numeric conversion rule, otherwise raw value) Only valid if "extended limit range valid" flag (bit 5) is set.
} m4CNRecord;

//-------------------------------------------------------------------------------------------------------
// ##CC Channel Conversion Block
//-------------------------------------------------------------------------------------------------------
// The data records can be used to store raw values (often also denoted as implementation values or internal values).
// The CCBLOCK serves to specify a conversion formula to convert the raw values to physical values with a physical unit.
// The result of a conversion always is either a floating-point value (REAL) or a character string (UTF-8).
//-------------------------------------------------------------------------------------------------------

// enumeration for member cc_type
#define CC_T_IDENTITY      0  // 1:1 conversion (in this case, the CCBLOCK simply can be omitted)
#define CC_T_LINEAR        1  // linear conversion      
#define CC_T_RATIONAL      2  // rational conversion 
#define CC_T_FORMULA       3  // algebraic conversion (MCD-2 MC text formula) 
#define CC_T_N2N_INTERPOL  4  // value to value tabular look-up with interpolation 
#define CC_T_N2N           5  // value to value tabular look-up without interpolation 
#define CC_T_R2N           6  // value range to value tabular look-up 
#define CC_T_N2T           7  // value to text/scale conversion tabular look-up 
#define CC_T_R2T           8  // value range to text/scale conversion tabular look-up 
#define CC_T_T2N           9  // text to value tabular look-up 
#define CC_T_T2T          10  // text to text tabular look-up (translation)

// Bit definitions for member cc_flags
#define CC_F_PRECISION    BIT0 //Precision valid flag If set, the precision value for display of floating point values specified in cc_precision is valid
#define CC_F_RANGE        BIT1 // Physical value range valid flag If set, both the minimum and the maximum physical value that occurred after conversion for this signal within the samples recorded in this file are known and stored in cc_phy_range_min and cc_phy_range_max. Otherwise the two fields are not valid. Note: the physical value range can only be expressed for conversions which return a numeric value (REAL). For conversions returning a string value or for the inverse conversion rule the flag must not be set.
#define CC_F_STATUS       BIT2 // Status string flag
                          //This flag indicates for conversion types 7 and 8 (value/value range to text/scale conversion tabular look-up) that the normal table entries are status strings (only reference to TXBLOCK), and the actual conversion rule is given in CCBLOCK
                          // referenced by default value. This also implies special handling of limits, see [MCD-2 MC] keyword STATUS_STRING_REF. Can only be set for 7 ≤ ca_type ≤ 8.

typedef struct
{
  // Block Header
  enum { RID=M4ID_CC };
  // enumeration of links
  enum
  {
    cc_tx_name, // Link to TXBLOCK with name (identifier) of conversion (can be NIL). Name must be according to naming rules stated in 4.4.2 Naming Rules.
    cc_md_unit, // Link to TXBLOCK/MDBLOCK with physical unit of signal data (after conversion). (can be NIL) Unit only applies if no unit defined in CNBLOCK. Otherwise the unit of the channel overwrites the conversion unit.
                // An MDBLOCK can be used to additionally reference the A-HDO unit definition (see Table 55). Note: for channels with cn_sync_type > 0, the unit is already defined, thus a reference to an A-HDO definition should be omitted to avoid redundancy.
    cc_md_comment, // Link to TXBLOCK/MDBLOCK with comment of conversion and additional information, see Table 54. (can be NIL)
    cc_cc_inverse, // Link to CCBLOCK for inverse formula (can be NIL, must be NIL for CCBLOCK of the inverse formula (no cyclic reference allowed).
    LinkMax,      // # of known links
    cc_ref=LinkMax  // List of additional links to TXBLOCKs with strings or to CCBLOCKs with partial conversion rules. Length of list is given by cc_ref_count. The list can be empty. Details are explained in formula-specific block supplement.
  };
  // Data Members
  M_UINT8 cc_type; // Conversion type (formula identifier) (see CC_T_xxx)
  M_UINT8 cc_precision; // Precision for display of floating point values. 0xFF means unrestricted precision (infinite) Any other value specifies the number of decimal places to use for display of floating point values. Note: only valid if "precision valid" flag (bit 0) is set and if cn_precision of the parent CNBLOCK is invalid, otherwise cn_precision must be used.
  M_UINT16 cc_flags; // Flags  (see CC_F_xxx)
  M_UINT16 cc_ref_count; // Length M of cc_ref list with additional links. See formula-specific block supplement for meaning of the links.
  M_UINT16 cc_val_count; // Length N of cc_val list with additional parameters. See formula-specific block supplement for meaning of the parameters.
  M_REAL cc_phy_range_min; // Minimum physical signal value that occurred for this signal. Only valid if "physical value range valid" flag (bit 1) is set.
  M_REAL cc_phy_range_max; // Maximum physical signal value that occurred for this signal. Only valid if "physical value range valid" flag (bit 1) is set.
  // plus cc_val :  List of additional conversion parameters. Length of list is given by cc_val_count. The list can be empty. Details are explained in formula-specific block supplement.
} m4CCRecord;


//-------------------------------------------------------------------------------------------------------
// ##CA Channel Array Block
//-------------------------------------------------------------------------------------------------------
// The CABLOCK is used to model an array.
//-------------------------------------------------------------------------------------------------------

// enumeration for member ca_type
#define CA_T_ARRAY  0 // Array The array is a simple D-dimensional value array (value block) without axes and without input/output/comparison quantities. (see [MCD-2 MC] keywords VAL_BLK for CHARACTERISTIC or MATRIX_DIM/ARRAY_SIZE for MEASUREMENT)
#define CA_T_AXIS   1 // Axis The array is an axis (1-dimensional vector), possibly referenced by one or more arrays of type "look-up" (ca_type = 2). (see [MCD-2 MC] keywords AXIS_PTS/AXIS_DESCR) Note that an axis may have itself an axis (e.g. CURVE_AXIS) and an own input quantity.
#define CA_T_LOOKUP 2 // Look-up The array is a D-dimensional array with axes. It can have input/output/comparison quantities. (see [MCD-2 MC] keywords CURVE/MAP/CUBOID/CUBE_n)
#define CA_T_INTERVAL_AXIS 3 
#define CA_T_CLASSIFICATION_RESULT 4

// enumeration for member ca_storage
#define CA_S_CN_TEMPLATE 0 // Values of all elements of the array are stored in the same record (i.e. all elements are measured together). The parent CNBLOCK defines the first element in the record (k = 0). All other elements are defined by the same CNBLOCK except that the values for cn_byte_offset and cn_inval_bit_pos change for each component (see explanation below).
#define CA_S_CG_TEMPLATE 1 // Value for each element of the array is stored in a separate record (i.e. elements are stored independently of each other).
                           // All records are stored in the same data block referenced by the parent DGBLOCK. As a consequence the data group (and thus the MDF file) is unsorted. All elements have exactly the same record layout specified by the parent CGBLOCK. However, each element uses a different cycle count (given by ca_cycle_count[k]) and a different record ID which must be calculated by "auto-increment" of the record ID of the parent CGBLOCK: cg_record_id + k. Since ca_cycle_count[0] must be equal to cg_cycle_count of the parent CGBLOCK, the parent CNBLOCK of the CABLOCK automatically describes the first array element (k = 0). When sorting a data group, a CABLOCK with "CG template" storage will be converted to a CABLOCK with "DG template" storage.
#define CA_S_DG_TEMPLATE 2 //Similar to CG template, the value of each element of the array is stored in a separate record (i.e. elements are stored independently of each other). However, the records for each element are stored in separate data blocks referenced by the list of links in ca_data. Similar to "CG template" storage, all elements have exactly the same record layout (defined by the parent CGBLOCK) but a different cycle count (specified by ca_cycle_count[k], see below). Since ca_cycle_count[0] must be equal to cg_cycle_count of the parent CGBLOCK, and ca_data[0] must be equal to dg_data of the parent DGBLOCK, the parent CNBLOCK of the CABLOCK automatically describes the first array element (k = 0).

// Bit definitions for member ca_flags
#define CA_F_DYNAMIC      BIT0 // dynamic size flag If set, the number of scaling points for the array is not fixed but can vary over time. Must not be set for array type "array".
#define CA_F_INPUT        BIT1 // input quantity flag If set, a channel for the input quantity is specified for each dimension by ca_input_quantity. Must not be set for array type "array".
#define CA_F_OUTPUT       BIT2 // output quantity flag If set, a channel for the output quantity is specified by ca_output_quantity. Must not be set for array types "array" and "axis".
#define CA_F_COMPARISON   BIT3 // comparison quantity flag If set, a channel for the comparison quantity is specified by ca_comparison_quantity. Must not be set for array types "array" and "axis".
#define CA_F_AXIS         BIT4 // axis flag If set, a scaling axis is given for each dimension of the array, either as fixed or as dynamic axis, depending on "fixed axis" flag (bit 5). Must not be set for array type "array".
#define CA_F_FIXED        BIT5 // fixed axis flag If set, the scaling axis is fixed and the axis points are stored as raw values in ca_axis_value list. If not set, the scaling axis may vary over time and the axis points are stored as channel referenced in ca_axis for each dimension. Only relevant if "axis" flag (bit 4) is set.
#define CA_F_INVERSE      BIT6 // inverse layout flag If set, the record layout is "column oriented" instead of "row oriented". See [MCD-2 MC] keywords ROW_DIR and COLUMN_DIR. Only relevant for "CN template" storage type.
#define CA_F_LEFTOPEN     BIT7 // left-open interval flag. If set, the interval ranges for the class interval axes are left-open and right-closed, i.e. ]a, b] ={x | a < x ≤ b}.
                               // If not set, the interval ranges for the class interval axes are left-closed and right-openi.e.[a, b[={x | a ≤ x < b}. Note that opening or closing is irrelevant for
                               // infinite endpoints(there can only be -INF for the left border of the very first interval, or +INF for right border of the very last interval).
                               // Only relevant for array type "interval axes".

typedef struct
{
  // Block Header
  enum { RID=M4ID_CA };
  // enumeration of links
  enum
  {
    ca_composition, // [] Array of composed elements: Pointer to a CNBLOCK for array of structures, or to a CABLOCK for array of arrays (can be NIL). If a CABLOCK is referenced, it must use the "CN template" storage type (ca_storage = 0).
    LinkMax,      // # of known links
    ca_data=LinkMax,  // [Π N(d) or empty] Only present for storage type "DG template". List of links to data blocks (DTBLOCK/DLBLOCK) for each element in case of "DG template" storage (ca_storage = 2). A link in this list may only be NIL if the cycle count of the respective element is 0: ca_data[k] = NIL => ca_cycle_count[k] = 0 The links are stored line-oriented, i.e. element k uses ca_data[k] (see explanation below). The size of the list must be equal to Π N(d), i.e. to the product of the number of elements per dimension N(d) over all dimensions D. Note: link ca_data[0] must be equal to dg_data link of the parent DGBLOCK.
    ca_dynamic_size, // [Dx3 or empty] Only present if "dynamic size" flag (bit 0) is set. References to channels for size signal of each dimension (can be NIL). Each reference is a link triple with pointer to parent DGBLOCK, parent CGBLOCK and CNBLOCK for the channel (either all three links are assigned or NIL). Thus the links have the following order: DGBLOCK for size signal of dimension 1 CGBLOCK for size signal of dimension 1 CNBLOCK for size signal of dimension 1 … DGBLOCK for size signal of dimension D CGBLOCK for size signal of dimension D CNBLOCK for size signal of dimension D The size signal can be used to model arrays whose number of elements per dimension can vary over time. If a size signal is specified for a dimension, the number of elements for this dimension at some point in time is equal to the value of the size signal at this time (i.e. for time-synchronized signals, the size signal value with highest time stamp less or equal to current time stamp). If the size signal has no recorded signal value for this time (yet), assume 0 as size.
    ca_input_quantity,  // [Dx3 or empty] Only present if "input quantity" flag (bit 1) is set. Reference to channels for input quantity signal for each dimension (can be NIL). Each reference is a link triple with pointer to parent DGBLOCK, parent CGBLOCK and CNBLOCK for the channel (either all three links are assigned or NIL). Thus the links have the following order: DGBLOCK for input quantity of dimension 1 CGBLOCK for input quantity of dimension 1 CNBLOCK for input quantity of dimension 1 … DGBLOCK for input quantity of dimension D CGBLOCK for input quantity of dimension D CNBLOCK for input quantity of dimension D Since the input quantity signal and the array signal must be synchronized, their channel groups must contain at least one common master channel type.
    ca_output_quantity, // [3 or empty] Only present if "output quantity" flag (bit 2) is set. Reference to channel for output quantity (can be NIL). The reference is a link triple with pointer to parent DGBLOCK, parent CGBLOCK and CNBLOCK for the channel (either all three links are assigned or NIL). Since the output quantity signal and the array signal must be synchronized, their channel groups must contain at least one common master channel type. For array type "look-up", the output quantity is the result of the complete look-up (see [MCD-2 MC] keyword RIP_ADDR_W). The output quantity should have the same physical unit as the array elements of the array that references it.
    ca_comparison_quantity, // [3 or empty] Only present if "comparison quantity" flag (bit 3) is set. Reference to channel for comparison quantity (can be NIL). The reference is a link triple with pointer to parent DGBLOCK, parent CGBLOCK and CNBLOCK for the channel (either all three links are assigned or NIL). Since the comparison quantity signal and the array signal must be synchronized, their channel groups must contain at least one common master channel type. The comparison quantity should have the same physical unit as the array elements.
    ca_cc_axis_conversion,  // [D or empty] Only present if "axis" flag (bit 4) is set. Pointer to a conversion rule (CCBLOCK) for the scaling axis of each dimension. If a link NIL a 1:1 conversion must be used for this axis. If the "fixed axis" flag (Bit 5) is set, the conversion must be applied to the fixed axis values of the respective axis/dimension (ca_axis_value list stores the raw values as REAL). If the link to the CCBLOCK is NIL already the physical values are stored in the ca_axis_value list. If the "fixed axes" flag (Bit 5) is not set, the conversion must be applied to the raw values of the respective axis channel, i.e. it overrules the conversion specified for the axis channel, even if the ca_axis_conversion link is NIL! Note: ca_axis_conversion may reference the same CCBLOCK as referenced by the respective axis channel ("sharing" of CCBLOCK).
    ca_axis // [Dx3 or empty] Only present if "axis" flag (bit 4) is set and "fixed axes flag" (bit 5) is not set. References to channels for scaling axis of respective dimension (can be NIL). Each reference is a link triple with pointer to parent DGBLOCK, parent CGBLOCK and CNBLOCK for the channel (either all three links are assigned or NIL). Thus the links have the following order: DGBLOCK for axis of dimension 1 CGBLOCK for axis of dimension 1 CNBLOCK for axis of dimension 1 … DGBLOCK for axis of dimension D CGBLOCK for axis of dimension D CNBLOCK for axis of dimension D Each referenced channel must be an array of type "axis". The maximum number of elements of each axis (ca_dim_size[0] in axis) must be equal to the maximum number of elements of respective dimension d in "look-up" array (ca_dim_size[d-1]).
  };
  // Data Members
  M_UINT8 ca_type; // Array type (defines semantic of the array) see CA_T_xxx
  M_UINT8 ca_storage; // Storage type (defines how the element values are stored) see CA_S_xxx
  M_UINT16 ca_ndim; //Number of dimensions D > 0 For array type "axis", D must be 1.
  M_UINT32 ca_flags; // Flags The value contains the following bit flags (Bit 0 = LSB): see CA_F_xxx
  M_UINT32 ca_byte_offset_base; // Base factor for calculation of Byte offsets for "CN template" storage type. ca_byte_offset_base should be larger than or equal to the size of Bytes required to store a component channel value in the record (all must have the same size). If it is equal to this value, then the component values are stored next to each other without gaps. Exact formula for calculation of Byte offset for each component channel see below.
  M_UINT32 ca_inval_bit_pos_base; //Base factor for calculation of invalidation bit positions for CN template storage type.
  /*// plus mvar*/
} m4CARecord;

//-------------------------------------------------------------------------------------------------------
// ##DT Data Block
//-------------------------------------------------------------------------------------------------------
// The data section of the DTBLOCK contains a sequence of records. 
// It contains records of all channel groups assigned to its parent DGBLOCK.
//-------------------------------------------------------------------------------------------------------
// Note: like the MD or TX blocks it does not have any Links and no fixed data members; the content simply
// is a BYTE-Array that has to be interpreted according to the infotrmation in DG/CG/CN blocks
//-------------------------------------------------------------------------------------------------------
typedef struct
{
  // Block Header
  enum { RID=M4ID_DT };
  // enumeration of links
  enum
  {
    dt_dt_next, // Pointer to next data block (DTBLOCK) (can be NIL)
    LinkMax      // # of known links
  };
  // Data Members
} m4DTRecord;

//-------------------------------------------------------------------------------------------------------
// ##SR Sample Reduction Block
//-------------------------------------------------------------------------------------------------------
// The SRBLOCK serves to describe a sample reduction for a channel group, i.e. an alternative sampling of 
// the signals in the channel group with a usually lower number of sampling points. 
// There can be several sample reductions for the same channel group which are stored in a forward linked
// list of SRBLOCKs starting at the CGBLOCK.
//-------------------------------------------------------------------------------------------------------


// enumeration for member sr_sync_type
#define SR_S_TIME     1   // sr_interval contains time interval in seconds
#define SR_S_ANGLE    2   // sr_interval contains angle interval in radians 
#define SR_S_DISTANCE 3   // sr_interval contains distance interval in meter
#define SR_S_INDEX    4   // sr_interval contains index interval for record index

// Bit definitions for member sr_flags
#define SR_F_INVALIDATION  BIT0 // invalidation Bytes flag If set, the sample reduction record contains invalidation Bytes, i.e. after the three data Byte sections for mean, minimum and maximum values, there is one invalidation Byte section. If not set, the invalidation Bytes are omitted. Must only be set if cg_inval_bytes > 0. If invalidation Bytes are used, and if the invalidation bit of the respective channel is valid (i.e. "invalidation bit valid" flag (bit 1) is set in cn_flags), then a set invalidation bit in the invalidation Bytes of the sample reduction record means that within the sample interval all signal values of the respective channel have been invalid.

typedef struct
{
  // Block Header
  enum { RID=M4ID_SR };
  // enumeration of links
  enum
  {
    sr_sr_next, // Pointer to next sample reduction block (SRBLOCK) (can be NIL)
    sr_data,     // Pointer to reduction data block (RDBLOCK) or a data list block (DLBLOCK of RDBLOCKs) with sample reduction records.
    LinkMax      // # of known links
  };
  // Data Members
  M_UINT64 sr_cycle_count;  // Number of cycles, i.e. number of sample reduction records in the reduction data block.
  M_REAL   sr_interval; // Length of sample interval > 0 used to calculate the sample reduction records (see explanation below). Unit depends on sr_sync_type.
  M_UINT8  sr_sync_type; // Sync type (see SR_S_xxx)
  M_UINT8  sr_flags;     // Flags (see SR_F_xxx)
  M_BYTE   sr_reserved[6];  // Reserved
} m4SRRecord;

//-------------------------------------------------------------------------------------------------------
// ##RD Reduction Data Block
//-------------------------------------------------------------------------------------------------------
// The data section of the RDBLOCK contains a sequence of sample reduction records. 
// It can only contain sample reduction records of its parent SRBLOCK. 
//-------------------------------------------------------------------------------------------------------
// Note: like the DT-Block there are neither links nor fixed data members
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
// ##SD Signal Data Block
//-------------------------------------------------------------------------------------------------------
// The data section of the SDBLOCK contains a sequence of signal values of variable length.
//-------------------------------------------------------------------------------------------------------
// Note: like the DT-Block there are neither links nor fixed data members
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// ##DL Data List Block
//-------------------------------------------------------------------------------------------------------
// The DLBLOCK references a list of data blocks (DTBLOCK) or a list of signal data blocks (SDBLOCK) or 
// a list of reduction data blocks (RDBLOCK). This list of blocks is equivalent to using a single
// (signal/reduction) data block and can be used to avoid a huge data block by splitting it 
// into smaller parts.
//-------------------------------------------------------------------------------------------------------

// bit definitions for member dl_flags
#define DL_F_EQUAL  BIT0  // Equal length flag If set, each DLBLOCK in the linked list has the same number of referenced blocks (dl_count) and the data sections of the blocks referenced by dl_data have a common length given by dl_equal_length. The only exception is that for the last DLBLOCK in the list (dl_dl_next = NIL), its number of referenced blocks dl_count can be less than or equal to dl_count of the previous DLBLOCK, and the data section length of its last referenced block (dl_data[dl_count-1]) can be less than or equal to dl_equal_length. If not set, the number of referenced blocks dl_count may be different for each DLBLOCK in the linked list, and the data section lengths of the referenced blocks may be different and a table of offsets is given in dl_offset.

typedef struct
{
  // Block Header
  enum { RID=M4ID_DL };
  // enumeration of links
  enum
  {
    dl_dl_next, // Pointer to next data list block (DLBLOCK) (can be NIL).
    LinkMax,      // # of known links
    dl_data=LinkMax,    // [N] Pointers to the data blocks (DTBLOCK, SDBLOCK or RDBLOCK). None of the links in the list can be NIL. All links must uniformly reference either DTBLOCKs or SDBLOCKs or RDBLOCKs. It is not allowed to mix the block types. Also all DLBLOCKs in the linked list of DLBLOCKs must reference the same block type.
  };
  // Data Members
  M_UINT8   dl_flags; //Flags  see DL_F_xxx
                      //Note: The value of the "equal length" flag must be equal for all DLBLOCKs in the linked list.
  M_BYTE    dl_reserved[3]; // Reserved
  M_UINT32  dl_count;    // Number of referenced blocks N If the "equal length" flag (bit 0 in dl_flags) is set, then dl_count must be equal for each DLBLOCK in the linked list except for the last one. For the last DLBLOCK (i.e. dl_dl_next = NIL) in this case the value of dl_count can be less than or equal to dl_count of the previous DLBLOCK.
  //M_UINT64  dl_equal_length; // Only present if "equal length" flag (bit 0 in dl_flags) is set. Equal data section length. Every block in dl_data list has a data section with a length equal to dl_equal_length. This must be true for each DLBLOCK within the linked list, and has only one exception: the very last block (dl_data[dl_count-1] of last DLBLOCK in linked list) may have a data section with a different length which must be less than or equal to dl_equal_length.
  //M_UINT64  dl_offset;  // Only present if "equal length" flag (bit 0 in dl_flags) is not set. Start offset (in Bytes) for the data section of each referenced block. If the data sections of all blocks referenced by dl_data list (for all DLBLOCKs in the linked list) are concatenated, the start offset for a referenced block gives the position of the data section of this block within the concatenated section. The start offset dl_offset[i] thus is equal to the sum of the data section lengths of all referenced blocks in dl_data list for every previous DLBLOCK in the linked list, and of all referenced blocks in dl_data list up to (i-1) for the current DLBLOCK. As a consequence, the start offset dl_offset[0] for the very first DLBLOCK must always be zero.
} m4DLRecord;

//-------------------------------------------------------------------------------------------------------
// ##DZ Data Zipped Block
//-------------------------------------------------------------------------------------------------------
// The DZBLOCK  represents a zipped (compressed) replacement for a (signal/reduction) 
// data block (DTBLOCK, SDBLOCK or RDBLOCK). It can be used instead of one of these 
// block types: whenever there is a DZBLOCK, a reading application internally must replace 
// it with the original block type containing the unzipped (uncompressed) data.
//-------------------------------------------------------------------------------------------------------

typedef struct
{
  // Block Header
  enum { RID=M4ID_DZ };
  // enumeration of links
  enum
  {
    LinkMax      // # of known links
  };
  // Data Members
	M_CHAR    dz_org_block_type[2]; // Block type identifier of the original (replaced) data block without the "##" prefix, i.e. either "DT", "SD" or "RD"
	M_UINT8   dz_zip_type;       // Zip algorithm used to compress the data stored in dz_data 
	                             // 0 = Deflate: The Deflate zip algorithm as used in various zip implementations (see [DEF] and [ZLIB])
                               // 1 = Transposition + Deflate: Before compression, the data block is transposed as explained in 5.26.2 Transposition of Data.
                               //     Typically only used for sorted data groups and DT or RD block types.
  M_BYTE    dl_reserved;       // Reserved
	M_UINT32  dz_zip_parameter;  // Parameter for zip algorithm. Content and meaning depends on dz_zip_type: For dz_zip_type = 1, the value must be > 1 
                               // and specifies the number of Bytes used as columns, i.e. usually the length of the record for a sorted data group.
                               // Otherwise the value must be 0.
	M_UINT64  dz_org_data_length; // Length of uncompressed data in Bytes, i.e. length of data section for original data block. 
                               // For a sorted data group, this should not exceed 2^22 Byte (4 MByte).
	M_UINT64  dz_data_length;    // Length N of compressed data in Bytes, i.e. the number of Bytes stored in dz_data. 
                               // If dz_org_data_length < dz_data_length + 24 the overhead of the compression is higher 
                               // than the memory saved. In this case, we recommend writing the original data block instead.
	//M_BYTE    dz_data;         // Contains compressed binary data for data section of original data block. 
} m4DZRecord;

//-------------------------------------------------------------------------------------------------------
// ##HL Header List Block
//-------------------------------------------------------------------------------------------------------
// The HLBLOCK represents the "header" of a list of data blocks, i.e. the start of a linked list 
// of DLBLOCKs. It contains information about the  DLBLOCKs and the contained data blocks.
//-------------------------------------------------------------------------------------------------------

typedef struct
{
  // Block Header
  enum { RID=M4ID_HL };
  // enumeration of links
  enum
  {
		hl_dl_first,  // Pointer to the first data list block (DLBLOCK) 
    LinkMax       // # of known links
  };
  // Data Members
  M_UINT16  hl_flags;    // Flags  see HL_F_xxx
	M_UINT8   hl_zip_type; // Zip algorithm used by DZBLOCKs referenced in the list, i.e. in an DLBLOCK of the link list starting at hl_dl_first.
                         // Note: all DZBLOCKs in the list must use the same zip algorithm. For possible values, please refer to dz_zip_type member of DZBLOCK.
  M_BYTE  hl_reserved[5]; // Reserved
} m4HLRecord;

// Bit definitions for member hl_flags
#define HL_F_EQUALLENGTH  BIT0 // For the referenced DLBLOCK (and thus for each DLBLOCK in the linked list), the value of the "equal length" flag (bit 0 in dl_flags) must be equal to this flag.

class MDF4File;
class M4SRBlock;

//---pure virtual base class for all blocks ------------------
// overwrite
//   getSize() - return the size of the block (w/o hdr/link)
//   getData() - return @ of payload[i] - return NULL if done
//   hdrId()   - return ID of block (e.g. 'HD')
class m4Block
{
public:
  // ctor for output blocks needs the record Id
  m4Block(size_t initLinks=0) : m_Links(initLinks),m_File(NULL),m_At(0)
  {
    memset(&m_Hdr,0,sizeof(m_Hdr));
    m_Hdr.hdr_hdr=M4ID_ID;  // '##'
  }
  virtual ~m4Block()
  {
  }

  DECLARE_DUMP

  // Record Layout
  // common Header
  // + nLinks*sizeof(M_LINK)
  // + FixedSize
  // + VariablePart(s)
  virtual M_UINT32 getFixedSize() { return 0; }
  virtual void *   getFixedPart() { return NULL; }

  //-------------------------------------------------
  // Output blocks
  //-------------------------------------------------
  // getSize() & getData(): without common header
  virtual M_SIZE getSize()=0;
  virtual PVOID  getData(int Index,M_SIZE &szRemain)=0;
  // Write to the stored file
  M_LINK WriteHdr();
  BOOL Save(bool bVar=true,bool bFixed=true);
  // reserve space and optionally write to file
  M_LINK Create(mDirectFile *f,int bWrite=0);

#if !_MDF4_WONLY     // [B]---- READ Support
  //-------------------------------------------------
  // Input blocks
  //-------------------------------------------------
  virtual BOOL Read(mDirectFile *File,M_LINK At,const m4BlockHdr &h);
  virtual BOOL readData(M_LINK At,M_SIZE szRemain)=0;
#endif               // [E]---- READ Support 

  //-------------------------------------------------
  // LINK section
  //-------------------------------------------------
  bool hasLink(size_t linkNo)
  {
    return linkNo<m_Links.getSize() &&
           *m_Links.get(linkNo)!=0;
  }
  void setLink(size_t linkNo,M_LINK at)
  {
    *m_Links.get(linkNo)=at;
  }
  M_LINK getLink(size_t linkNo)
  {
    if (linkNo>=m_Links.getSize())
      return 0;
    else
      return *m_Links.get(linkNo);
  }
  BOOL setLinkBlk(m4Block &blk,size_t linkNo)
  {
    // cannot call twice
    if (!hasLink(linkNo))
    {
      assert(m_File);
      M_LINK lkAt=blk.Create(m_File,3);
      if (lkAt)
      {
        setLink(linkNo,lkAt);
        return TRUE;
      }
    }
    return FALSE;
  }

  // return file position
  M_LINK Link() const { return m_At; }
  // get the RID
  virtual M_UINT16 hdrID() const
  {
    return m_Hdr.hdr_id;
  }

public:
  mDirectFile *m_File; // associated file
  M_LINK        m_At;   // and position
  m4BlockHdr    m_Hdr;  // the common Header
  dynArray<M_LINK> m_Links; // dynamic Array with Links
};

//-----------------------------------------------------
// ##TX Text
//-----------------------------------------------------
class M4TXBlock : public m4Block
{
public:
  M4TXBlock()
  {
    m_Hdr.hdr_id=M4ID_TX;
  }
  M4TXBlock(M_UTF8 Text)
  {
    m_Hdr.hdr_id=M4ID_TX;
    Append(Text);
  }
#if _MDF4_UTF8      // [B]---- UTF8 Support
  M4TXBlock(const wchar_t *pszText)
  {
    m_Hdr.hdr_id=M4ID_TX;
    Append(pszText);
  }
#endif              // [E]---- UTF8 Support
  virtual M_SIZE getSize() { return m_utf8.length()+1; }
  virtual PVOID  getData(int Index,M_SIZE &szRemain)
  {
    if (Index==0)
    {
      szRemain=getSize();
      return (void *)m_utf8.data();
    }
    return NULL;
  }
  virtual BOOL readData(M_LINK At,M_SIZE szRemain)
  {
    M_UINT32 dwRemain=(M_UINT32)szRemain; // Only 2G supported
    void *Buf=(void *)m_utf8.init(dwRemain);
    return m_File->ReadAt(At,dwRemain,Buf);
  }
#if _MDF4_UTF8      // [B]---- UTF8 Support
  void Append(M_UNICODE pszText)
  {
    if (pszText) while (*pszText)
    {
      m_utf8 << *pszText++;
    }
  }
#endif             // [E]---- UTF8 Support 
  void Append(M_UTF8 pszText)
  {
    if (pszText) 
    {
      size_t len=strlen((const char *)pszText);
			m_utf8.push((const unsigned char *)pszText,len);
    }
  }
  DECLARE_DUMP
#if _MDF4_UTF8    // [B]---- UTF8 Support  
  utf8out m_utf8;
#else             // [e]---- UTF8 Support
  byte_buffer m_utf8;
#endif            // [B]---- UTF8 Support
};
//-----------------------------------------------------
// ##MD MetaData
//-----------------------------------------------------
class M4MDBlock : public M4TXBlock
{
public:
  M4MDBlock()
  {
  }
#if _MDF4_XML        // [B]---- MSXML Support
  MSXML2::IXMLDOMNodePtr common_element(M_UNICODE elemName,M_UNICODE elemValue);
  M4MDBlock(M_UNICODE rootName,M_UNICODE txText);
  MSXML2::IXMLDOMNodePtr common() { return element(L"common_properties"); }
  MSXML2::IXMLDOMNodePtr extension() { return element(L"extensions"); }
  MSXML2::IXMLDOMNodePtr root() { return m_Doc ? m_Doc->documentElement : NULL; }
  void md_element(M_UNICODE elemName,M_UNICODE elemValue,M_UNICODE elemParent=NULL);
  virtual void closeXML();
  MSXML2::IXMLDOMNodePtr createNode(M_UNICODE elemName);
  MSXML2::IXMLDOMNodePtr element(M_UNICODE elemName);
  BOOL CreateInstance();
  MSXML2::IXMLDOMDocument3Ptr m_Doc;
#else               // [e]---- MSXML Support
#  if _MDF4_UTF8    // [B]---- UTF8  Support
  M4MDBlock(M_UNICODE Text) : M4TXBlock(Text)
  {
    m_Hdr.hdr_id=M4ID_MD;
  }
#  endif           // [E]---- UTF8  Support  
  M4MDBlock(M_UTF8 Text) : M4TXBlock(Text)
  {
    m_Hdr.hdr_id=M4ID_MD;
  }
#endif            // [E]---- MSXML Support 
};

#if _MDF4_XML     // [B]---- MSXML  Support
//-------------------------------------------------------------------------
// M4HDComment - a MD Block for HD.hd_md_comment
//-------------------------------------------------------------------------
class M4HDComment : public M4MDBlock
{
public:
  M4HDComment(M_UNICODE txText);
  void setAuthor(M_UNICODE author) { common_element(L"author",author); }
  void setDepartment(M_UNICODE dep){ common_element(L"department",dep); }
  void setProject(M_UNICODE proj)  { common_element(L"project",proj); }
  void setSubject(M_UNICODE subj)  { common_element(L"subject",subj); }
  void setTimeSource(M_UNICODE ts);
};
//-------------------------------------------------------------------------
// M4FHComment - a MD Block for FH.fh_md_comment
//-------------------------------------------------------------------------
class M4FHComment : public M4MDBlock
{
public:
  M4FHComment(M_UNICODE txText);
  void setToolName(M_UNICODE name) { fh_element(L"tool_id",name); }
  void setToolVendor(M_UNICODE vendor) { fh_element(L"tool_vendor",vendor); }
  void setToolVersion(M_UNICODE ver){ fh_element(L"tool_version",ver); }
  void setUserName(M_UNICODE name)  { fh_element(L"user_name",name); }
  void fh_element(M_UNICODE elemName,M_UNICODE elemValue);
  virtual void closeXML();
};
//-------------------------------------------------------------------------
// M4ATComment - a MD Block for AT.at_md_comment
//-------------------------------------------------------------------------
class M4ATComment : public M4MDBlock
{
public:
  M4ATComment(M_UNICODE txText) : M4MDBlock(L"ATcomment",txText)
  {
  }
};
#endif         // [E]---- MSXML  Support

//-------------------------------------------------------------------------
// template to associate data structures with m4Blocks
// template Arguments
//   class R = RecordType
//   class T = variable part
//   ID=M4_ID
// Note: the object is derived from m4Block as well as from R
//-------------------------------------------------------------------------
template<class R,class T=M_BYTE,int ID=R::RID> class m4BlockImpl : public m4Block,public R
{
public:
  m4BlockImpl(size_t nVar=0) : m4Block(R::LinkMax),m_var(nVar)
  {
    m_Hdr.hdr_hdr=M4ID_ID;
    m_Hdr.hdr_id=ID;
    R *pThis=static_cast<R *>(this);
    memset(pThis,0,sizeof(R));
  }
  virtual ~m4BlockImpl()
  {
  }
  virtual M_UINT32 getFixedSize() { return sizeof(R); }
  virtual void *   getFixedPart() { return static_cast<R *>(this); }

  virtual BOOL readData(M_LINK At,M_SIZE szRemain)
  {
    // read the fixed Part
    R *pThis=static_cast<R *>(this);
    M_SIZE szThis=sizeof(R);
    if (szThis>szRemain) szThis=szRemain;
    if (!m_File->ReadAt(At,(M_UINT32)szThis,pThis))
        return FALSE;
    if ((szRemain-=szThis)>0)
    {
      // read the variable part
      PVOID pVar=m_var.resize((size_t)szRemain);
      At+=szThis;
      if (!m_File->ReadAt(At,(M_UINT32)szRemain,pVar))
        return FALSE;
    }
    return TRUE;
  }
  virtual M_SIZE getSize() { return m_var.getBytes(); }
  virtual PVOID  getData(int Index,M_SIZE &szRemain) 
  {
    if (Index==0)
    {
      assert(szRemain==m_var.getBytes());
      return szRemain ? m_var.get() : NULL;
    }
    else
    {
      assert(FALSE);
    }
    return NULL;
  }
  BOOL setCommentBlk(m4Block &TXorMD,int linkNo)
  {
    // cannot call twice
    if (!hasLink(linkNo))
    {
      assert(TXorMD.hdrID()==M4ID_TX || TXorMD.hdrID()==M4ID_MD);
      assert(m_File);
      M_LINK mdAt=TXorMD.Create(m_File,3);
      if (mdAt)
      {
        setLink(linkNo,mdAt);
        return TRUE;
      }
    }
    return FALSE;
  }
  BOOL setText(m4Block &TX,int linkNo)
  {
    // cannot call twice
    if (!hasLink(linkNo))
    {
      assert(TX.hdrID()==M4ID_TX);
      assert(m_File);
      M_LINK txAt=TX.Create(m_File,3);
      if (txAt)
      {
        setLink(linkNo,txAt);
        return TRUE;
      }
    }
    return FALSE;
  }
  T *get(size_t Index=0) { return m_var.get(Index); }
  dynArray<T> m_var;
};

// helper struct to count stored records for ChannelGroups (RecordId)
// also used to count messages per bus (Id,count)
struct idCount
{
  M_UINT64 id;  // (DGNum<<45)|recId
  M_LINK aov;
  M_SIZE cnt;
};
typedef std::map<M_UINT64,idCount> idCounts;

//-----------------------------------------------------
// ##FH FileHistory
//-----------------------------------------------------
class M4FHBlock : public m4BlockImpl<m4FHRecord>
{
public:
  M4FHBlock(MDF4File *File);     // ctor: create and insrt current time
  M4FHBlock();                   // used for reading
  
  BOOL setComment(m4Block &md);  // CANNOT be a TX Block!
  DECLARE_DUMP

};
//-----------------------------------------------------
// ##CH Channel Hierarchy
//-----------------------------------------------------
class M4CHBlock : public m4BlockImpl<m4CHRecord>
{
public:
  M4CHBlock() // used for reading
  {
  }
  DECLARE_DUMP
};
//-----------------------------------------------------
// ##CA Channel Array
//-----------------------------------------------------
class M4CABlock : public m4BlockImpl<m4CARecord, M_UINT64>
{
public:
  M4CABlock() // used for reading
  {
  }
  DECLARE_DUMP
};
//-----------------------------------------------------
// ##EV Events
//-----------------------------------------------------
class M4EVBlock : public m4BlockImpl<m4EVRecord>
{
public:
  M4EVBlock(M_UINT8 t=EV_T_RECORDING)
  {
    ev_type=t;
  }
  DECLARE_DUMP

  BOOL setScope(m4Block &cg)
  {
    // cannot call twice
    if (!hasLink(ev_scope))
    {
      assert(cg.hdrID()==M4ID_CG || cg.hdrID()==M4ID_CN);
      M_LINK evAt=cg.Create(m_File,3);
      if (evAt)
      {
        setLink(ev_scope,evAt);
				ev_scope_count++;
	      return TRUE;
      }
    }
    return FALSE;
  }

};
//-----------------------------------------------------
// ##AT Attachment
//-----------------------------------------------------
class M4ATBlock : public m4BlockImpl<m4ATRecord>
{
public:
  M4ATBlock(mDirectFile *f,M_UINT16 creatorIndex=0);
  M4ATBlock()
  {
  }
  BOOL LinkFile(M_FILENAME ExtPath,bool bPath=true,bool bMD5=false);
  BOOL EmbedFile(M_FILENAME ExtPath,bool bZip=false,bool bMD5=false);
  BOOL Open(bool bZip=false,bool bMD5=false);
  BOOL Write(void *Data,M_UINT32 Size);
  void Close();
  BOOL setComment(m4Block &TxOrMD) { return setCommentBlk(TxOrMD,at_md_comment); }
  BOOL setMimeType(m4Block &tx)    { return setText(tx,at_tx_mimetype); }
  BOOL setMimeType(M_UTF8 mime)   { M4TXBlock tx(mime); return setMimeType(tx); }
#if _MDF4_UTF8       // [B]---- MSXML  Support
  BOOL setMimeType(M_UNICODE mime)   { M4TXBlock tx(mime); return setMimeType(tx); }
#endif               // [E]---- MSXML  Support   

  virtual M_SIZE getSize() { return at_embedded_size; }
  virtual void *getData(int Index,M_SIZE &szRemain) { return NULL; }
  DECLARE_DUMP
protected:
  M_LINK m_embeddedPos;  // File Position 
#if _MDF4_MD5       // [B]---- MD5  Support
  MD5Context m_md5;      // running md5 checksum
#endif              // [B]---- MD5  Support

};
//-----------------------------------------------------
// ##SI SourceInformation
//-----------------------------------------------------
class M4SIBlock : public m4BlockImpl<m4SIRecord>
{
public:
  M4SIBlock(M_UINT8 siType,M_UINT8 siBusType)
  {
    si_type=siType;
    si_bus_type=siBusType;
  }
  M4SIBlock() 
  {
  }
  void setSimulated(bool bSim) { if (bSim) si_flags|=SI_FL_SIMULATED; else si_flags&=~SI_FL_SIMULATED; }
  bool getSimulated() const { return (si_flags&SI_FL_SIMULATED)!=0; }
  BOOL setComment(m4Block &TXorMD) { return setCommentBlk(TXorMD,si_md_comment); }
  BOOL setName(m4Block &tx) { return setText(tx,si_tx_name); }
  BOOL setName(M_UTF8 Name) { M4TXBlock tx(Name); return setName(tx); }
  BOOL setPath(m4Block &tx) { return setText(tx,si_tx_path); }
  BOOL setPath(M_UTF8 Path) { M4TXBlock tx(Path); return setPath(tx); }
#if _MDF4_UTF8   // [B]---- UTF8  Support
  BOOL setName(M_UNICODE Name) { M4TXBlock tx(Name); return setName(tx); }
  BOOL setPath(M_UNICODE Path) { M4TXBlock tx(Path); return setPath(tx); }
#endif          // [B]---- UTF8  Support 
  DECLARE_DUMP

protected:

};
//-----------------------------------------------------
// ##CC ChannelConversion
//-----------------------------------------------------
class M4CCBlock : public m4BlockImpl<m4CCRecord,M_REAL>
{
public:
  M4CCBlock(M_UINT8 ccType)
  {
    cc_type=ccType;
  }
  M4CCBlock()
  {
  }
  BOOL setComment(m4Block &TXorMD) { return setCommentBlk(TXorMD,cc_md_comment); }
  BOOL setName(m4Block &tx) { return setText(tx,cc_tx_name); }
  BOOL setUnit(m4Block &TXorMD) { return setCommentBlk(TXorMD,cc_md_unit); }
  BOOL setInverse(M4CCBlock &inv) { return setLinkBlk(inv,cc_cc_inverse); }

  void setFlags(M_UINT32 set,M_UINT32 clr=0)
  {
    cc_flags&=~clr;
    cc_flags|=set;
  }

  void setRange(M_REAL rMin,M_REAL rMax)
  {
    cc_phy_range_min=rMin;
    cc_phy_range_max=rMax;
    setFlags(CC_F_RANGE);
  }
  void setPrecision(M_UINT8 p)
  {
    cc_precision=p;
    setFlags(CC_F_PRECISION);
  }

  DECLARE_DUMP

protected:

};
//-----------------------------------------------------
// ##CC Identity (same as no CC block)
//-----------------------------------------------------
class M4CCIdentity : public M4CCBlock
{
public:
  M4CCIdentity() : M4CCBlock(CC_T_IDENTITY)
  {
  }
};
//-----------------------------------------------------
// ##CC Linear (P0,P1) R=I*P1+P0
//-----------------------------------------------------
class M4CCLinear : public M4CCBlock
{
public:
  M4CCLinear(M_REAL Factor=1.0,M_REAL Offset=0.0) : M4CCBlock(CC_T_LINEAR)
  {
    m_var.add(cc_val_count=2);
    *m_var.get(0)=Offset;
    *m_var.get(1)=Factor;
  }
  void setFactor(M_REAL Factor) { *m_var.get(1)=Factor; }
  void setOffset(M_REAL Offset) { *m_var.get(0)=Offset; }
  M_REAL getFactor() { return *m_var.get(1); }
  M_REAL getOffset() { return *m_var.get(0); }
};
//-----------------------------------------------------
// ##CC Rational (P0-P5) R=(P0*I*I + P1*I + P2)/
//                         (P3*I*I + P4*I + P5)
//-----------------------------------------------------
class M4CCRational : public M4CCBlock
{
public:
  M4CCRational() : M4CCBlock(CC_T_RATIONAL)
  {
    m_var.add(cc_val_count=6);
  }
  void setParam(int pNo,M_REAL pVal) 
  {
    assert(pNo<cc_val_count);
    *m_var.get(pNo)=pVal;
  }
  M_REAL getParam(int pNo) 
  {
    assert(pNo<cc_val_count);
    return *m_var.get(pNo);
  }
};
//-----------------------------------------------------
// ##CC Formula (ASAM MCD 3 MC)
//-----------------------------------------------------
class M4CCFormula : public M4CCBlock
{
public:
  M4CCFormula() : M4CCBlock(CC_T_FORMULA)
  {
    m_Links.add(cc_ref_count=1);
  }
  void setFormula(M4TXBlock &tx) 
  {
    setLinkBlk(tx,cc_ref);
  }
};
//-----------------------------------------------------
// ##CC Numerical Table (with/without interpolation)
// (key,value)-pairs
//-----------------------------------------------------
class M4CCNumTable : public M4CCBlock
{
public:
  M4CCNumTable(M_UINT8 ccType) : M4CCBlock(ccType)
  {
  }
  void addPair(M_REAL key,M_REAL val) 
  {
    int valNo=cc_val_count;
    cc_val_count+=2;
    *m_var.get(valNo++)=key;
    *m_var.get(valNo)=val;
  }
};
class M4CCN2NInterpol : public M4CCNumTable
{
public:
  M4CCN2NInterpol() : M4CCNumTable(CC_T_N2N_INTERPOL)
  {
  }
};
class M4CCN2N : public M4CCNumTable
{
public:
  M4CCN2N() : M4CCNumTable(CC_T_N2N)
  {
  }
};

//-----------------------------------------------------
// ##CC Numerical Table
// (key_min,key_max,value)-pairs
// + default value
//-----------------------------------------------------
class M4CCR2N : public M4CCBlock
{
public:
  M4CCR2N() : M4CCBlock(CC_T_R2N)
  {
  }
  void addPair(M_REAL key_min,M_REAL key_max,M_REAL val) 
  {
    int valNo=cc_val_count;
    cc_val_count+=3;
    *m_var.get(valNo++)=key_min;
    *m_var.get(valNo++)=key_max;
    *m_var.get(valNo)=val;
  }
  void setDefault(M_REAL defVal)
  {
    int valNo=cc_val_count++;
    *m_var.get(valNo)=defVal;
  }
};
//-----------------------------------------------------
// ##CC NumericalToText Table
// (key,textvalue)-pairs
// + default text
//NOTE: "textvalue" may also be another CC block!
//-----------------------------------------------------
class M4CCN2T : public M4CCBlock
{
public:
  M4CCN2T() : M4CCBlock(CC_T_N2T)
  {
  }
  void addEntry(M_REAL key,m4Block &TXorCC) 
  {
    assert(cc_val_count==cc_ref_count);
    int valNo=cc_val_count++;
    cc_ref_count++;
    *m_var.get(valNo)=key;
    setLinkBlk(TXorCC,cc_ref+valNo);
  }
  void setDefault(m4Block &TXorCC)
  {
    int valNo=cc_ref_count++;
    setLinkBlk(TXorCC,cc_ref+valNo);
  }
};
//-----------------------------------------------------
// ##CC NumericalRangeToText Table
// (key_min,key_max,textvalue)-pairs
// + default text
//NOTE: "textvalue" may also be another CC block!
//-----------------------------------------------------
class M4CCR2T : public M4CCBlock
{
public:
  M4CCR2T() : M4CCBlock(CC_T_R2T)
  {
  }
  void addEntry(M_REAL key_min,M_REAL key_max,m4Block &TXorCC) 
  {
    assert(cc_val_count==cc_ref_count*2);
    *m_var.get(cc_val_count++)=key_min;
    *m_var.get(cc_val_count++)=key_max;
    setLinkBlk(TXorCC,cc_ref+cc_ref_count++);
  }
  void setDefault(m4Block &TXorCC)
  {
    setLinkBlk(TXorCC,cc_ref+cc_ref_count++);
  }
};
//-----------------------------------------------------
// ##CC TextToNumerical Table
// (textIn,value)-pairs
// + default value
//-----------------------------------------------------
class M4CCT2N : public M4CCBlock
{
public:
  M4CCT2N() : M4CCBlock(CC_T_T2N)
  {
  }
  void addEntry(M4TXBlock &txKey,M_REAL val) 
  {
    assert(cc_val_count==cc_ref_count);
    *m_var.get(cc_val_count++)=val;
    setLinkBlk(txKey,cc_ref+cc_ref_count++);
  }
  void setDefault(M_REAL defVal)
  {
    *m_var.get(cc_val_count++)=defVal;
  }
};
//-----------------------------------------------------
// ##CC TextToText Table (Translation)
// (textIn,textOut)-pairs
// + default text
//-----------------------------------------------------
class M4CCT2T : public M4CCBlock
{
public:
  M4CCT2T() : M4CCBlock(CC_T_T2T)
  {
  }
  void addEntry(M4TXBlock &txKey,M4TXBlock &txVal) 
  {
    setLinkBlk(txKey,cc_ref+cc_ref_count++);
    setLinkBlk(txKey,cc_ref+cc_ref_count++);
  }
  void setDefault(M4TXBlock &defVal)
  {
    setLinkBlk(defVal,cc_ref+cc_ref_count++);
  }
};

//-----------------------------------------------------
// ##CN Channel
//-----------------------------------------------------
class M4CNBlock : public m4BlockImpl<m4CNRecord>
{
public:
  M4CNBlock(M_UINT8 Type,M_UINT8 SyncType=CN_S_NONE)
  {
    cn_type=Type; cn_sync_type=SyncType;
  }
  M4CNBlock()
  {
  }
  BOOL setComment(m4Block &TXorMD) { return setCommentBlk(TXorMD,cn_md_comment); }
  BOOL setName(m4Block &tx)        { return setText(tx,cn_tx_name); }
  BOOL setSource(M4SIBlock &si)    { return setLinkBlk(si,cn_si_source); }
  BOOL setUnit(m4Block &TXorMD)    { return setCommentBlk(TXorMD,cn_md_unit); }
  BOOL setData(m4Block &blk)       { return setLinkBlk(blk,cn_data); }
  BOOL setConversion(M4CCBlock &cc){ return setLinkBlk(cc,cn_cc_conversion); }
  void setFlags(M_UINT32 set,M_UINT32 clr=0)
  {
    cn_flags=(cn_flags&~clr)|set;
  }

  void setLocation(M_UINT8 DataType,M_UINT64 startBit,M_UINT32 nBits)
  {
    cn_data_type=DataType;
    cn_byte_offset=(M_UINT32)(startBit>>3);
    cn_bit_offset=(M_UINT8)(startBit&7);
    cn_bit_count=nBits;
  }
  void setLocationBytes(M_UINT8 DataType,M_UINT64 startByte,M_UINT32 nBytes)
  {
    setLocation(DataType,startByte*8,nBytes*8);
  }
  void setInvalidation(M_UINT32 iBit,bool bOn=true)
  {
    if (bOn)
    {
      cn_inval_bit_pos=iBit;
      setFlags(CN_F_INVALID,0);
    }
    else
    {
      setFlags(0,CN_F_INVALID);
    }
  }
  void setRange(M_REAL rMin,M_REAL rMax)
  {
    cn_val_range_min=rMin;
    cn_val_range_max=rMax;
    setFlags(CN_F_RANGE);
  }
  void setLimit(M_REAL rMin,M_REAL rMax)
  {
    cn_limit_min=rMin;
    cn_limit_max=rMax;
    setFlags(CN_F_LIMIT);
  }
  void setExtLimit(M_REAL rMin,M_REAL rMax)
  {
    cn_limit_ext_min=rMin;
    cn_limit_ext_max=rMax;
    setFlags(CN_F_EXTLIMIT);
  }
  void setPrecision(M_UINT8 p)
  {
    cn_precision=p;
    setFlags(CN_F_PRECISION);
  }
  DECLARE_DUMP

protected:

};

//-----------------------------------------------------
// ##CG ChannelGroup
//-----------------------------------------------------
class M4CGBlock : public m4BlockImpl<m4CGRecord>
{
public:
  M4CGBlock(M_UINT64 recId=0,bool bVLSD=false)
  {
    cg_record_id=recId;
    if (bVLSD) cg_flags=CG_FL_VLSD;
    m_cnNext=NULL;
  }
  virtual ~M4CGBlock()
  {
    delete m_cnNext;
  }
  BOOL setComment(m4Block &TXorMD) { return setCommentBlk(TXorMD,cg_md_comment); }
  BOOL setAcqName(m4Block &tx)     { return setText(tx,cg_tx_acq_name); }
  BOOL setSource(M4SIBlock &si)    { return setLinkBlk(si,cg_si_acq_source); }
  void setRecordCount(M_UINT64 cycles) { cg_cycle_count=cycles; }
  void adrRecordCount(idCount &c,M_UINT64 recId)
  {
    //assert(cg_cycle_count==0);
    c.id=recId;
    c.aov=m_At+sizeof(m4BlockHdr)+m_Links.getSize()*sizeof(M_LINK)+offsetof(m4CGRecord,cg_cycle_count);
    //c.cnt=0;
		c.cnt=cg_cycle_count;
  }
  void setRecordSize(M_UINT32 ds,M_UINT32 is=0)
  {
    assert((cg_flags&CG_FL_VLSD)==0);
    cg_data_bytes=ds;
    cg_inval_bytes=is;
  }
  void setRecordSize(M_UINT64 vlsdSize)
  {
    assert((cg_flags&CG_FL_VLSD)!=0);
    cg_data_bytes=(M_UINT32)vlsdSize;
    cg_inval_bytes=(M_UINT32)(vlsdSize>>32);
  }
  M_SIZE getRecordSize(M_UINT8 recIdSize=0) const
  {
    M_SIZE szRecord=cg_inval_bytes;
    if (cg_flags&CG_FL_VLSD)
      szRecord=(szRecord<<32)|cg_data_bytes;
    else
      szRecord+=cg_data_bytes;
    return szRecord+recIdSize;
  }
  void setCNnext(M4CNBlock *cnNext) { m_cnNext = cnNext; }
  M4CNBlock *addChannel(M4CNBlock *cn, BOOL bNoDelParent=FALSE);
  DECLARE_DUMP

protected:
  M4CNBlock *m_cnNext;

};

//-----------------------------------------------------
// ##SR SignalReduction
//-----------------------------------------------------
class M4SRBlock : public m4BlockImpl<m4SRRecord>
{
public:
  M4SRBlock()
  {
  }
};

//-----------------------------------------------------
// ##DT DataBlock
// ##SD Signal data Block
// ##RD Reduction data Block 
//-----------------------------------------------------
class M4DataBlock : public m4Block
{
public:
  M4DataBlock(M_UINT16 blkId,M_UINT32 uiBlockSize) : m_blkSize(uiBlockSize),m_wrtSize(0),m_Buffer(NULL)
  {
    m_Hdr.hdr_id=blkId;
		if (m_blkSize)
			m_Buffer = calloc((size_t)m_blkSize,1);
  }
  virtual ~M4DataBlock()
  {
		if (m_Buffer)
			free(m_Buffer);
  }
  virtual M_UINT32 getFixedSize() { return 0; }
  virtual M_SIZE getSize() { return m_wrtSize;}
  virtual void   setSize(M_SIZE wrtSize) { m_wrtSize = wrtSize;}
	virtual M_SIZE getBlkSize() { return m_blkSize;}
  virtual void   setBlkSize(M_SIZE blktSize) { m_blkSize = blktSize;}
	virtual bool   ReallocBuffer(M_UINT32 uiBlockSize)
	{
     void *p = realloc(m_Buffer, uiBlockSize);
    if(p)
      m_Buffer = p;
    else
    {
      free(m_Buffer);
      m_Buffer = NULL;
    }
    return m_Buffer != NULL;
	}
  virtual PVOID getData(int Index,M_SIZE &szRemain)
  {
    if (Index==0)
    {
      szRemain=getSize();
			return m_Buffer;
    }
    return NULL;
  }
  virtual BOOL readData(M_LINK At,M_SIZE szRemain)
  {
    M_UINT32 dwRemain=(M_UINT32)szRemain; // Only 2G supported
		if (m_Buffer == NULL || m_blkSize < szRemain)
		{
			if (m_Buffer)	free(m_Buffer);
			m_blkSize = m_wrtSize=szRemain;
			if ((m_Buffer = calloc((size_t)m_blkSize,1)) == NULL)
				return FALSE;
		}
		return m_File->ReadAt(At,dwRemain,m_Buffer);
  }
  M_UINT32 CanWrite() const
  {
    return (M_UINT32)(m_blkSize-m_wrtSize);
  }
  bool isEmpty() const
  {
    return m_wrtSize==0;
  }
  virtual M_LINK Reset(mDirectFile *f)
  {
    if (m_File) Save(true,true);
    m_wrtSize=0;
    return m_At=Reserve(m_File=f);
  }

  bool DoWrite(M_UINT32 nBytes,const void *Memory)
  {
    if (CanWrite()>=nBytes)
    {
      M_BYTE *b=(M_BYTE *)m_Buffer;
      memcpy(b+m_wrtSize,Memory,nBytes);
      m_wrtSize+=nBytes;
      return true;
    }
    return false;
  }

  M_LINK Reserve(mDirectFile *f)
  {
    M_SIZE szMax=sizeof(m4BlockHdr)+m_blkSize; // no fixed part, no links
    M_LINK at=f->Request(szMax,true);
		ATLTRACE("##DT (%8I64X) %I64d+%I64d=%I64d\n",at,at,szMax,at+szMax);
    return at;
  }

  DECLARE_DUMP
protected:
	M_SIZE     m_blkSize;
	M_SIZE     m_wrtSize;
	void      *m_Buffer;
};
//-----------------------------------------------------
// ##DT DataBlock
//-----------------------------------------------------
class M4DTBlock : public M4DataBlock
{
public:
  M4DTBlock(M_UINT32 blkSize=0) : M4DataBlock(M4ID_DT,blkSize)
  {
  }
};
//-----------------------------------------------------
// ##SD Signal data Block
//-----------------------------------------------------
class M4SDBlock : public M4DataBlock
{
public:
  M4SDBlock(M_UINT32 blkSize=0) : M4DataBlock(M4ID_SD,blkSize)
  {
  }
};
//-----------------------------------------------------
// ##RD Reduction data Block 
//-----------------------------------------------------
class M4RDBlock : public M4DataBlock
{
public:
  M4RDBlock(M_UINT32 blkSize=0) : M4DataBlock(M4ID_RD,blkSize)
  {
  }
};
//-----------------------------------------------------
// ##HL Header List Block 
//-----------------------------------------------------
class M4HLBlock : public m4BlockImpl<m4HLRecord,M_UINT64>
{
public:
  M4HLBlock()
  {
  }
};
//-----------------------------------------------------
// ##DZ Data Zipped Block for Reading
//-----------------------------------------------------
class M4DZBlock : public m4BlockImpl<m4DZRecord,M_UINT8>
{
public:
  M4DZBlock()
  {
  }
};
//-----------------------------------------------------
// ##DZ Data Zipped Block for Writing
//-----------------------------------------------------
class M4DZBlockWr : public m4BlockImpl<m4DZRecord,M_UINT64>
{
public:
  M4DZBlockWr(M_UINT16 blkId,M_UINT32 uiBlockSize) : m_blkSize(uiBlockSize),m_wrtSize(0),m_Buffer(NULL)
  {
    m_Hdr.hdr_id=blkId;
		if (m_blkSize)
			m_Buffer = calloc((size_t)m_blkSize,1);
  }
  virtual ~M4DZBlockWr()
  {
		if (m_Buffer)
			free(m_Buffer);
  }
  virtual M_SIZE getSize() { return m_wrtSize;}
	virtual void setSize(M_SIZE Size) {m_wrtSize = Size;}
	virtual M_SIZE getBlkSize() { return m_blkSize;}
  virtual PVOID getData(int Index,M_SIZE &szRemain)
  {
    if (Index==0)
    {
      szRemain=getSize();
			return m_Buffer;
    }
    return NULL;
  }
  M_UINT32 CanWrite() const
  {
    return (M_UINT32)(m_blkSize-m_wrtSize);
  }
  bool isEmpty() const
  {
    return m_wrtSize==0;
  }
  void Reset()
  {
    m_wrtSize=0;
  }

  bool DoWrite(M_UINT32 nBytes,const void *Memory)
  {
    if (CanWrite()>=nBytes)
    {
      M_BYTE *b=(M_BYTE *)m_Buffer;
      memcpy(b+m_wrtSize,Memory,nBytes);
      m_wrtSize+=nBytes;
      return true;
    }
    return false;
  }

  M_LINK Reserve(mDirectFile *f)
  {
    M_SIZE szMax=sizeof(m4BlockHdr)+m_blkSize; // no fixed part, no links
    M_LINK at=f->Request(szMax,true);
    ATLTRACE("##DT (%8I64X) %I64d+%I64d=%I64d\n",at,at,szMax,at+szMax);
    return at;
  }
  DECLARE_DUMP
protected:
	M_SIZE     m_blkSize;
	M_SIZE     m_wrtSize;
	void      *m_Buffer;
};
//-----------------------------------------------------
// ##DL DataList (of DT/SR/SD/RD blocks)
//-----------------------------------------------------
class M4DLBlock : public m4BlockImpl<m4DLRecord,M_UINT64>
{
public:
  M4DLBlock() : m_maxLinks(0)
  {
  }
  M4DLBlock(M_UINT32 dlEntries,M_UINT64 eqLen) : m_maxLinks(dlEntries)
  {
    dl_flags=DL_F_EQUAL;
    dl_count=0;
    *get(0)=eqLen;
    // m_Links.add(dlEntries);
  }
  void addLink(M_LINK lAt)
  {
    ++dl_count;
    setLink(dl_count,lAt);
  }
	void setMaxLinks(M_UINT32 nLinks)
	{
		m_maxLinks = nLinks;
	}
  bool isFull() const
  {
    return dl_count>=m_maxLinks;
  }
  M_LINK Reset(mDirectFile *f)
  {
    M_LINK lNo;
    if (f)
    {
      lNo=Reserve(m_File=f);
    }
    else
    {
      lNo=Reserve(m_File);
      setLink(dl_dl_next,lNo);
      Save();
    }
    m_Links.resize(sizeof(M_LINK));
    dl_count=0;
    return m_At=lNo;
  }

  virtual ~M4DLBlock()
  {
  }
  DECLARE_DUMP
protected:
  M_LINK Reserve(mDirectFile *f)
  {
    //calc maximum size of Block
    M_SIZE szMax=sizeof(m4BlockHdr)+(m_maxLinks+1)*sizeof(M_LINK)+getFixedSize()+getSize();
    M_LINK at=f->Request(szMax,true);
    ATLTRACE("##DL (%8I64X) %I64d+%I64d=%I64d\n",at,at,szMax,at+szMax);
    return at;
  }
protected:
  M_UINT32 m_maxLinks;
};


class M4DGBlock;
#if !_MDF4_WONLY     // [B] READ Support
#include <vector>
typedef std::vector<M_UINT8> byteBuffer;
#endif
//--------------------------------------------------------------------------------
// m4DataStream - a helper class to write to/read from a DT- or DL-Block
// also supports SR,SD,RD blocks
//--------------------------------------------------------------------------------
class m4DataStream
{
public:

  // for writing: fixed DT-Block or equal_size DL block(s)
  m4DataStream(M4DGBlock *dg,M_UINT32 BlkSize,M_UINT32 dlEntries=5,M_UINT16 dtID=M4ID_DT);
  m4DataStream(M4SRBlock *sr,M_UINT32 BlkSize,M_UINT32 dlEntries=5,M_UINT16 dtID=M4ID_RD);
  m4DataStream();

  virtual ~m4DataStream()
  {
    Close();
		if (m_pHL)
		{
			delete m_pHL;
			m_pHL = NULL;
		}
  }

#if !_MDF4_WONLY     // [B] READ Support
  // for reading: init with the dg_data link
  m4DataStream(MDF4File *File,M_LINK At);

  // for intermediate reading from an open stream (dup)
  m4DataStream(m4DataStream &org,M_LINK Position=0);

  // get the current size of the stream
  M_SIZE getSize() const { return m_currentSize; }

  // Seek to a position in the stream
  M_LINK Seek(M_LINK Position);
  // Read from the current position
  M_UINT32 Read(M_UINT32 nBytes,void *Into);
  bool     ReadAll(M_UINT32 nBytes,void *Into)
  {
    return nBytes==Read(nBytes,Into);
  }
	// Compression
	void* ReadDZBlock(M_LINK At, M_SIZE Size);
	int Inflate(M_BYTE* dst, M_UINT32 *pdwSize, M_BYTE *src, M_UINT32 dwCompressedLength);
	int Deflate(M_BYTE* dst, M_UINT32 *pdwCompressedLength, M_BYTE *src, M_UINT32 dwSize);
	BOOL Transpose(M_BYTE *dst, M_UINT32 dwSize, M_UINT32 dwRecordSize);
	// Error strings
  struct dlErrorEntry
  {
		M_UINT32 nErrorClass;
		M_UNICODE strError;
  };
  dynArray<dlErrorEntry> m_ErrorList;
	void AddError(M_UNICODE strError, M_UINT32 nClass)
	{
		//dlErrorEntry *e = m_ErrorList.add(1);
		//e->strError = _wcsdup(strError);
		//e->nErrorClass = nClass;
	}
	M_UINT32 GetErrors(void) { return m_ErrorList.getSize(); }
	M_UNICODE GetError(M_UINT32 nIndex)
	{
		if (nIndex < m_ErrorList.getSize())
			return m_ErrorList.get(nIndex)->strError;
		return NULL;
	}

protected:
  bool initSize(M_LINK At);
public:
	bool IsValid() { return m_bValid; }
#endif             // [E] READ Support

	int GetZipCompressionLevel() { return m_ZipCompressionLevel; }
	void SetZipCompressionLevel(int ZipCompressionLevel) { m_ZipCompressionLevel = ZipCompressionLevel; }

  // Write to the stream
  virtual bool Write(M_UINT32 nBytes, const void *Memory);
  // 
  void AddRecords(M_UINT32 nRecs=1,M_UINT64 RecId=0);
  virtual void Flush(BOOL bKeepSize = FALSE);
  virtual void Close();

protected:
  MDF4File *m_File;
	M4HLBlock *m_pHL; // Pointer to an HL-Block or NULL
  bool      m_dg;
	bool      m_bValid;
  M4DataBlock *m_db;
  M4DLBlock   *m_dl;
  M_UINT16     m_blkType;     
	int					m_ZipCompressionLevel;

#if !_MDF4_WONLY     // [B] READ Support
  M_SIZE    m_currentSize;
  M_LINK     m_currentPos;
  M_SIZE     m_currentOff;
	byteBuffer m_DZReadBuffer;
	byteBuffer m_TransposedBuffer;

  struct dlEntry
  {
    M_LINK At;
    M_SIZE Size;
		M_BYTE *pBuffer;
		M_INT32 BlkID;
  };
  dynArray<dlEntry> m_List;
  dlEntry *m_currentEntry;
  dlEntry *m_firstEntry;
  M_UINT32 m_nEntries;
#endif             // [E] READ Support
};

//--------------------------------------------------------------------------------
// m4DZStream - a helper class to write to a DZ-Block
//--------------------------------------------------------------------------------
class m4DZStream : public m4DataStream
{
public:

  // for writing: fixed DT-Block or equal_size DL block(s)
	m4DZStream() { m_dz = 0; }
  m4DZStream(M4DGBlock *dg,M_UINT32 BlkSize,M_UINT32 RecordSize,M_UINT32 dlEntries=5);
  m4DZStream(M4SRBlock *sr,M_UINT32 BlkSize,M_UINT32 RecordSize,M_UINT32 dlEntries=5);

  virtual ~m4DZStream()
  {
		if (m_dz)
			delete m_dz;
		m_dz = 0;
  }

  // get the current size of the stream
  M_SIZE getSize() const { return m_currentSize; }

public:
	bool IsValid() { return m_bValid; }

  // Write to the stream
  virtual bool Write(M_UINT32 nBytes, const void *Memory);
  // 
  //void AddRecords(M_UINT32 nRecs=1,M_UINT64 RecId=0);
  virtual void Flush(BOOL bKeepSize = FALSE);
  virtual void Close();

protected:
  M4DZBlockWr *m_dz;

};


////////////////////////////////////////////////////////////////////////////
// class m4DataStreamEx
// Calls to add data to an existing group. (by Yvan Pouget, d2t)
////////////////////////////////////////////////////////////////////////////

class m4DataStreamEx : public m4DataStream
{
public:
	m4DataStreamEx(M4DGBlock *dg, M_UINT32 BlkSize, M_UINT32 dlEntries, M_UINT16 dtID = M4ID_DT);
	virtual ~m4DataStreamEx();
};

class m4DZStreamEx : public m4DZStream
{
public:
	m4DZStreamEx(M4DGBlock *dg, M_UINT32 BlkSize, M_UINT32 recordSize, M_UINT32 dlEntries);
	virtual ~m4DZStreamEx();
};

//-----------------------------------------------------
// ##DG DataGroup
//-----------------------------------------------------
class M4DGBlock : public m4BlockImpl<m4DGRecord>
{
public:
  M4DGBlock(M_UINT8 recIdSize=0)
  {
    assert(recIdSize==0 || recIdSize==1 || recIdSize==2 || recIdSize==4 || recIdSize==8);
    m_cgNext=NULL;
    dg_rec_id_size=recIdSize;
  }
  virtual ~M4DGBlock()
  {
    cg_delete();
  }
  BOOL setComment(m4Block &TXorMD) { return setCommentBlk(TXorMD,dg_md_comment); }
  M4CGBlock *addChannelGroup(M4CGBlock *cg);

  m4DataStream *WriteStream(M_UINT32 blkSize,M_UINT32 dlSize=5);
  m4DZStream *DZWriteStream(M_UINT32 blkSize,M_UINT32 recordSize,M_UINT32 dlSize=5);
	m4DataStreamEx *WriteStreamEx(M_UINT32 blkSize, M_UINT32 dlSize=5);
	m4DZStreamEx *DZWriteStreamEx(M_UINT32 blkSize, M_UINT32 recordSize, M_UINT32 dlSize=5);
#if !_MDF4_WONLY     // [B] READ Support
  m4DataStream *ReadStream();
#endif             // [E] READ Support


  DECLARE_DUMP
protected:
  void cg_delete();

protected:
  M4CGBlock *m_cgNext;

};

//-----------------------------------------------------
// ##HD Header
//-----------------------------------------------------
class M4HDBlock : public  m4BlockImpl<m4HDRecord>
{
public:
  M4HDBlock()
  {
    m_fhNext=NULL;
    m_atNext=NULL;
    m_dgNext=NULL;
	  m_evNext=NULL;
    m_bPrepared=false;
  }
  virtual ~M4HDBlock()
  {
    delete m_fhNext;
    delete m_atNext;
    delete m_dgNext;
    delete m_evNext;
  }
  BOOL setComment(m4Block &TXorMD) { return setCommentBlk(TXorMD,hd_md_comment); }
  M4FHBlock *addHistory(M4FHBlock *fh);
  M4EVBlock *addEvent(M4EVBlock *ev);
  M4ATBlock *addAttachment(M4ATBlock *fh);
  M4DGBlock *addDataGroup(M4DGBlock *dg);
  void Prepare();
  void Close()
  {
    if (m_bPrepared)
    {
      Save();
      m_bPrepared=false;
    }
  }

#if !_MDF4_WONLY      // [B]---- READ  Support
  BOOL Load(mDirectFile *f);
#endif                // [E]---- READ  Support 

  DECLARE_DUMP

  M4FHBlock *m_fhNext;
  M4ATBlock *m_atNext;
  M4DGBlock *m_dgNext;
  M4EVBlock *m_evNext;

protected:
  bool       m_bPrepared;
};




class MDF4File : public mDirectFile
{
public:
  MDF4File();
  virtual ~MDF4File();
  virtual void Close();

  BOOL Create(M_FILENAME strPathName, const char *strProducer = NULL, int iVersion = 400);
#if !_MDF4_WONLY     // [B]---- READ  Support
  BOOL Open(M_FILENAME strPathName, BOOL bUpdate = FALSE);
  m4Block *LoadBlock(M_LINK At);
  //m4Block *LoadLink(m4Block &parent,int linkNo);
  m4Block *LoadLink(m4Block &parent,int linkNo,M_UINT16 id=0);
  bool     LoadBlkHdr(M_LINK At,m4BlockHdr &h);
#endif              // [E]---- READ  Support


#if _MDF4_WIN       // [B]---- WIN  Support
  void setFileTime(const FILETIME &t);
  void setFileTime();
#endif              // [E]---- READ  Support
  void setFileTime(const M_DATE &t)
  {
    m_Hdr.hd_start_time=t;
  }

  BOOL setComment(m4Block &TXorMD) { return m_Hdr.setComment(TXorMD); }
  void addHistory(M4FHBlock *fh) { m_Hdr.addHistory(fh); }
  void addAttachment(M4ATBlock *at) { m_Hdr.addAttachment(at); }
  M4DGBlock *addDataGroup(M4DGBlock *dg) { return m_Hdr.addDataGroup(dg); }
  void *addEvent(M4EVBlock *ev) { return m_Hdr.addEvent(ev); }
  BOOL Save() { return m_Hdr.Save(); }

#if _MDF4_DUMP     // [B]---- DUMP  Support
  void dump(m4TreeItem *Tree);
#endif             // [E]---- DUMP  Support

  void Prepare()
  {
    m_Hdr.Prepare();
  }
  void addRecordCount(M4CGBlock *cg,M_UINT64 recId=0);
  void AddRecords(M_UINT32 nRecs=1,M_UINT64 recId=0);
  void FlushRecordCount();
	M4HDBlock *GetHdr() { return &m_Hdr; }
	mdfFileId *GetIdBlk() { return &m_Id; }

	M4SRBlock *CreateSRBlock(M4DGBlock *dg, /* the data group */
		          M4CGBlock *cg,              /* the channel group */
							double dt,                  /* the new delta t */
							double tRange,              /* the time-range of the group in X units (probably "s") */
							double OldXFactor,          /* the x increment of the time signal */
							double OldXOffset,          /* the offset of the time signal */
							M_UINT32 uiMaxBlockSize = 0x40000, /* the maximal uncompressed block size */
							BOOL bCalcStats=FALSE);      /* calc Min/Max of signals and monotony of time signal */
	
	void CalculateMinMax(M4DGBlock *dg, M4CGBlock *cg, 
							M_REAL OldXFactor, M_REAL OldXOffset);
	
	// Helpers for SR blocks
protected:
	void CreateSRBlockFromSR(M4SRBlock *sr, M4DGBlock *dg, M4CGBlock *cg, double dt, double tRange,
		                      M_UINT32 nChan, M_REAL OldXFactor, M_REAL OldXOffset, M_REAL FactorXSig, M_REAL OffsetXSig, M_UINT32 uiMaxBlockSize);
	void CreateSRBlockFromDG(M4SRBlock *sr, M4DGBlock *dg, M4CGBlock *cg, double dt, double tRange,
		                      M_UINT32 nChan, M_REAL OldXFactor, M_REAL OldXOffset, M_REAL FactorXSig, M_REAL OffsetXSig, 
													M_UINT32 uiMaxBlockSize, BOOL bCalcStats);
	M4CNBlock *FindMasterChannel(M4CGBlock *cg, M_UINT32 *pnChan);
	M_UINT32 CountChannels(M4CGBlock *cg);

	// Helper for SR blocks:
	typedef struct stats {
		double Min, Max;
		double mindt, maxdt;
		M_UINT32 uiInitialized;
	} Stats;

	typedef struct three {
		double Mean, Min, Max;
		M_UINT64 n, nMiss;
		struct {
			unsigned sbit:16;   /*   Start Bit         */
			unsigned nbit:7;    /*   Number of Bits    */
			unsigned bMaster:2;
			unsigned nbytes:4;  
			unsigned bInteger:1;
			unsigned bSigned:1;
			unsigned bMotorola:1;
		} flags;
		M_UINT32 iInvalBitPos;
	} Three;
	void FillThree(M4DGBlock *dg, M4CGBlock *cg, Three *pBuffer);
	void ResetThree(Three *pBuffer, M_UINT32 nChan);
	double Interpret(M_UINT8 *pRecord, Three *pBuffer, M_UINT32 RecLen, bool bValueOnly=false);
	double InterpretSR(M_UINT8 *pRecord, Three *pBuffer, M_UINT32 RecLen, bool bValueOnly=false);
	void Encode(M_UINT8 *pRecord, Three *pBuffer, M_UINT32 RecLen);
	void MovAndSwap(M_UINT8 *pVal,M_UINT8 *pDst,M_UINT32 nBytes)
	{
	  pDst+=nBytes;
	  while (nBytes--) *--pDst=*pVal++;
	}
	void OrAndSwap(M_UINT8 *pVal,M_UINT8 *pDst,M_UINT32 nBytes)
	{
	  pDst+=nBytes;
	  while (nBytes--) *--pDst |= *pVal++;
	}
	M_UINT32 get_dword_mask(M_INT32 idx);
	M_UINT32 get_signed_max(M_INT32 idx);
	M_UINT32 get_signed_sub(M_INT32 idx);
	M_INT64  get_i64_mask(M_INT32 idx);
	// End: Helper for SR blocks:

protected:
  mdfFileId m_Id;
  M4HDBlock m_Hdr;
  idCounts  m_recCnt;
};



