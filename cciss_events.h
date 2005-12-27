/*
 * cciss_events.h
 * Copyright (C) 2003  Hugo Trippaers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
/*
 * $Header$
 */

#define BYTE __u8
#define WORD __u16
#define DWORD __u32
#define QWORD __u64

/* from cciss_cmd.h */
#define CISS_MAX_LUN    16
#define CISS_MAX_PHYS_LUN       1024

#define CompareEvent(event,par_class,par_subclass,par_detail) \
                     ((event.class.class==par_class) && \
					 (event.class.subclass == par_subclass) && \
					 (event.class.detail == par_detail))

#pragma pack(1) /* these structures must be byte aligned */
typedef struct _cciss_event_physicaldrivechange {
	WORD physicaldrivenumber;
	BYTE configureddriveflag;
	BYTE sparedriveflag;
	BYTE bigphysicaldrivenumber;
	BYTE enclosurebaynumber;
} cciss_event_physicaldrivechange;

typedef struct _cciss_event_powersupplydetail {
	WORD port;
	WORD psubid;
	WORD box;
} cciss_event_powersupplydetail;

typedef struct _cciss_event_fandatadetail {
	WORD port;
	WORD fanid;
	WORD box;
} cciss_event_fandatadetail;

typedef struct _cciss_event_upsdetail {
	WORD port;
	WORD psupid;
} cciss_event_upsdetail;

typedef struct _cciss_event_redctrlrdetail {
	WORD slot;
} cciss_event_redctrldetail;

typedef struct _cciss_event_temperaturedatadetail {
	WORD port;
	WORD sensorid;
	WORD box;
} cciss_event_temperaturedetail;

typedef struct _cciss_event_chassisdetail {
	WORD port;
	WORD reserved;
	WORD box;
} cciss_event_chassisdetail;

typedef struct _cciss_event_phystatchange {
	WORD physicaldrivenumber;
	BYTE failurereason;
	BYTE configureddriveflag;
	BYTE sparedriveflag;
	BYTE bigphysicaldrivenumber;
	BYTE enclosurebaynumber;
} cciss_event_phystatchange;

typedef struct _cciss_event_logstatchange {
	WORD logicaldrivenumber;
	BYTE previouslogicaldrivestate;
	BYTE newlogicaldrivestate;
	BYTE currentsparestatus;
} cciss_event_logstatchange;

typedef struct _cciss_event_rebuildaborted {
	WORD logicaldrivenumber;
	BYTE replacementdrive;
	BYTE errordrive;
	BYTE bigreplacementdrive;
	BYTE bigerrordrive;
} cciss_event_rebuildaborted;

typedef struct _cciss_event_logfatalerrorio {
	WORD logicaldrivenumber;
	DWORD logicalblockaddress;
	WORD logicalblockcount;
	BYTE logicalcommand;
	BYTE fataldrivebus;
	BYTE fataldriveid;
	QWORD biglogicalblockaddress;
} cciss_event_logfatalerrorio;

typedef struct _cciss_event_surfacedata {
	WORD logicaldrivenumber;
} cciss_event_surfacedata;

typedef struct _cciss_event_redstatchange {
	BYTE preferredslotcontrol;
	BYTE currentredundancymode;
	BYTE redundantcontrollerstatus;
	BYTE redundantfailurereason;
	BYTE previousslotcontrol;
	BYTE previousredundancymode;
	BYTE previousredundantctrlrstatus;
	BYTE previousredundantfailurereason;
} cciss_event_redstatchange;

typedef struct _cciss_event_loophwinfo {
	WORD bus;
} cciss_event_loophwinfo;

typedef union _cciss_event_detail_type {
	cciss_event_physicaldrivechange physicaldrivechange;
	cciss_event_powersupplydetail powersupplydetail;
	cciss_event_fandatadetail fandatadetail;
	cciss_event_upsdetail upsdetail;
	cciss_event_redctrldetail redctrldetail;
	cciss_event_temperaturedetail temperaturedetail;
	cciss_event_chassisdetail chassisdetail;
	cciss_event_phystatchange phystatchange;
	cciss_event_logstatchange logstatchange;
	cciss_event_rebuildaborted rebuildaborted;
	cciss_event_logfatalerrorio logfatalerrorio;
	cciss_event_surfacedata surfacedata;
	cciss_event_redstatchange redstatchange;
	cciss_event_loophwinfo loophwinfo;
	BYTE data[64];
} cciss_event_detail_type;

typedef struct _cciss_event_time_type {
  BYTE month;
  BYTE day;
  WORD year;
  DWORD seconds;
} cciss_event_time_type;

typedef struct _cciss_event_class_type {
  WORD class;
  WORD subclass;
  WORD detail;
} cciss_event_class_type;

typedef struct _cciss_event_type {
  DWORD timestamp;                /* Relative Controller Time   byte 0-3 */
  cciss_event_class_type class;   /* Event Class                byte 4-9 */
  cciss_event_detail_type detail; /* Event Specific Data Fields byte 10-73 */
  unsigned char mesgstring[80];   /* Null Terminated ASCII Mesg byte 74-153 */
  DWORD tag;                      /* Event Tag                  byte 154-157 */
  cciss_event_time_type time;     /* Event Time                 byte 158-165 */
  WORD prepowertime;              /* Pre power up Time (sec)    byte 166-167 */
  LUNAddr_struct deviceaddr;      /* Device address             byte 168-175 */
  unsigned char padding[335];     /* padding                    byte 176-511 */
} cciss_event_type;


typedef struct _cciss_report_logicallun_struct
{
  BYTE LUNlist_len[4];
  DWORD reserved;
  LUNAddr_struct luns[CISS_MAX_LUN];
} cciss_report_logicallun_struct;

#pragma pack() /* normal alignment */

const char *logicaldrivestatusstr[] = {
        "Logical drive is ok",
        "Logical drive is failing",
        "Logical drive is not configured",
        "Logical drive is using interim recovery mode",
        "Logical drive is ready for recovery operation",
        "Logical drive is is currently recovering",
        "Wrong physical drive was replaced",
        "A physical drive is not properly connected",
        "Hardware is overheating",
        "Hardware has overheated",
        "Logical drive is currently expanding",
        "Logical drive is not yet available",
        "Logical drive is queued for expansion",
};

#define SEV_CRITICAL 2
#define SEV_WARNING 1
#define SEV_NORMAL 0

const int logicaldrivestatusseverity[] = {
        SEV_NORMAL,
        SEV_CRITICAL,
        SEV_WARNING,
        SEV_CRITICAL,
        SEV_WARNING,
        SEV_WARNING,
        SEV_CRITICAL,
        SEV_CRITICAL,
        SEV_CRITICAL,
        SEV_CRITICAL,
        SEV_WARNING,
        SEV_WARNING,
        SEV_WARNING
};

const char *sparestatusstr[] = {
	"online",
};
