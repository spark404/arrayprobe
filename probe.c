/*
 * probe.c
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <malloc.h>
#include <getopt.h>
#include <errno.h>

#include <linux/compiler.h>
#include <linux/cciss_ioctl.h>

#include <ida_ioctl.h>

#include "cciss_events.h"

#include "config.h"

typedef struct _logdrv_state {
  int state;
  char *message;
  int severity;
} logdrv_state;

typedef struct _logdrv {
  char *devicestr;  /* filesystem device node eg. /dev/cciss/c0d0 */
  int type; /* type of controller eg. CCISS or IDA */
  int drvnum; /* number of this logical drive */
  logdrv_state state; /* current state of this logical drive */
} logdrv;

/* globals */
int verbose = 0;

/* macros */
#define log(format,...) \
    if (verbose) { printf (format, __VA_ARGS__); }

/* defines */
#define CTTYPE_CCISS 1
#define CTTYPE_IDA 2

int
cciss_get_event (int device_fd, int reset_pointer, cciss_event_type * event)
{
  int result;
	IOCTL_Command_struct iocommand;
	unsigned char *buffer;

	iocommand.LUN_info.LunAddrBytes[0] = 0;
	iocommand.LUN_info.LunAddrBytes[1] = 0;
	iocommand.LUN_info.LunAddrBytes[2] = 0;
	iocommand.LUN_info.LunAddrBytes[3] = 0;
	iocommand.LUN_info.LunAddrBytes[4] = 0;
	iocommand.LUN_info.LunAddrBytes[5] = 0;
	iocommand.LUN_info.LunAddrBytes[6] = 0;
	iocommand.LUN_info.LunAddrBytes[7] = 0;

	iocommand.Request.Type.Type = TYPE_CMD;
	iocommand.Request.Type.Attribute = ATTR_SIMPLE;
	iocommand.Request.Type.Direction = XFER_READ;

	iocommand.Request.Timeout = 0;	/* don't time out */

	iocommand.Request.CDBLen = 13;
	iocommand.Request.CDB[0] = 0xC0;	/* CISS Read */
	iocommand.Request.CDB[1] = 0xD0;	/* Notify on Event */
	iocommand.Request.CDB[2] = 0x0;	/* reserved, leave 0 */
	iocommand.Request.CDB[3] = 0x0;	/* reserved, leave 0 */
	iocommand.Request.CDB[4] = 0x0;
	iocommand.Request.CDB[5] = 0x0;
	iocommand.Request.CDB[6] = 0x0;
	iocommand.Request.CDB[7] = (reset_pointer) ? 0x7 : 0x3;	/* bit 2 set = reset pointer, bit 0 set = synchronous mode */
	iocommand.Request.CDB[8] = 0x0;
	iocommand.Request.CDB[9] = 0x0;
	iocommand.Request.CDB[10] = 0x2;
	iocommand.Request.CDB[11] = 0x0;
	iocommand.Request.CDB[12] = 0x0;

	buffer = (unsigned char *) malloc (512);
	memset (buffer, 0x0, 512);
	iocommand.buf_size = 512;
	iocommand.buf = buffer;

	result = ioctl (device_fd, CCISS_PASSTHRU, &iocommand);
	if (result < 0)
	{
		perror (" * ioctl failed");
		return -1;
	}

	/* Ignore these errors
	 * overrun (1) should not happen anyway and
	 * underrun (2) is not a problem
	 */
	if ((iocommand.error_info.CommandStatus != 1) && (iocommand.error_info.CommandStatus != 2) && (iocommand.error_info.CommandStatus != 0)) {
	  printf (" * Command failed with Comnmand Status %d\n", iocommand.error_info.CommandStatus);
	  return -1;
	}

	memcpy (event, buffer, 512);
	return 0;
}

int
cciss_get_logical_luns (int device_fd, cciss_report_logicallun_struct * logluns)
{
  int result;
	IOCTL_Command_struct iocommand;
	unsigned char *buffer;

	iocommand.LUN_info.LunAddrBytes[0] = 0;
	iocommand.LUN_info.LunAddrBytes[1] = 0;
	iocommand.LUN_info.LunAddrBytes[2] = 0;
	iocommand.LUN_info.LunAddrBytes[3] = 0;
	iocommand.LUN_info.LunAddrBytes[4] = 0;
	iocommand.LUN_info.LunAddrBytes[5] = 0;
	iocommand.LUN_info.LunAddrBytes[6] = 0;
	iocommand.LUN_info.LunAddrBytes[7] = 0;

	iocommand.Request.Type.Type = TYPE_CMD;
	iocommand.Request.Type.Attribute = ATTR_SIMPLE;
	iocommand.Request.Type.Direction = XFER_READ;

	iocommand.Request.Timeout = 0;	/* don't time out */

	iocommand.Request.CDBLen = 12;
	iocommand.Request.CDB[0] = 0xC2; /* Report logical LUNs */
	iocommand.Request.CDB[1] = 0x0;	 /* reserved, leave 0 */
	iocommand.Request.CDB[2] = 0x0;	 /* reserved, leave 0 */
	iocommand.Request.CDB[3] = 0x0;	 /* reserved, leave 0 */
	iocommand.Request.CDB[4] = 0x0;  /* reserved, leave 0 */
	iocommand.Request.CDB[5] = 0x0;  /* reserved, leave 0 */
	iocommand.Request.CDB[6] = 0x0;  /* byte 6-9 alloc length = 128 (0x80)*/
	iocommand.Request.CDB[7] = 0x0;
	iocommand.Request.CDB[8] = 0x0;
	iocommand.Request.CDB[9] = 0x80;
	iocommand.Request.CDB[10] = 0x0; /* reserved, leave 0 */
	iocommand.Request.CDB[11] = 0x0; /* control ? */

	buffer = (unsigned char *) malloc (128);
	memset (buffer, 0x0, 128);
	iocommand.buf_size = 128;
	iocommand.buf = buffer;

	result = ioctl (device_fd, CCISS_PASSTHRU, &iocommand);
	if (result < 0)
	{
		perror (" * ioctl failed");
		return -1;
	}

	/* Ignore these errors
	 * overrun (1) should not happen anyway and
	 * underrun (2) is not a problem
	 */
	if ((iocommand.error_info.CommandStatus != 1) && (iocommand.error_info.CommandStatus != 2) && (iocommand.error_info.CommandStatus != 0)) {
	  printf (" * Command failed with Comnmand Status %d\n", iocommand.error_info.CommandStatus);
	  return -1;
	}
	
 	memcpy (logluns, buffer, 128);
	return 0;
}

int
cciss_simulate_get_event (int device_fd, int reset_pointer,
			  cciss_event_type * event)
{
	unsigned char *buffer;

	buffer = (unsigned char *) malloc (512);
	if ((read (device_fd, buffer, 512)) < 0)
	{
		perror ("reading from file");
		return -1;
	}
	memcpy (event, buffer, 512);
	return 0;
}

void
cciss_print_event (cciss_event_type event)
{
	int prevstate, newstate;
	
	printf ("Event code %d/%d/%d",
		event.class.class, event.class.subclass, event.class.detail);
	if (event.tag == 0)
	{
		printf ("\n");
	}
	else
	{
		printf (" with tag %d\n", event.tag);
	}

	if (event.time.month != 0)
	{
		printf ("at %d-%d-%d %02d:%02d:%02d\n",
			event.time.day, 
			event.time.month, 
			event.time.year,
			event.time.seconds / 3600,
			event.time.seconds % 3600 / 60,
			event.time.seconds % 60);
	}
	else if (event.timestamp != 0)
	{
		printf ("at %d seconds since last power cycle or controler reset\n", event.timestamp);
	}

	printf ("with message: %s\n", event.mesgstring);
	
/* 	printf ("on device %02X %02X %02X %02X %02X %02X %02X %02X\n",
 * 			event.deviceaddr.LunAddrBytes[0],
 * 			event.deviceaddr.LunAddrBytes[1],
 * 			event.deviceaddr.LunAddrBytes[2],
 * 			event.deviceaddr.LunAddrBytes[3],
 * 			event.deviceaddr.LunAddrBytes[4],
 * 			event.deviceaddr.LunAddrBytes[5],
 * 			event.deviceaddr.LunAddrBytes[6],
 * 			event.deviceaddr.LunAddrBytes[7]);
 */
	

	if (CompareEvent(event, 5,0,0)) {	
		prevstate = event.detail.logstatchange.previouslogicaldrivestate;
		newstate = event.detail.logstatchange.newlogicaldrivestate;
		printf ("logical drive %d, changed from state %d to %d\n",
		        event.detail.logstatchange.logicaldrivenumber,
				prevstate,
				newstate);
		printf ("state %d: %s\n", 
				prevstate,
				logicaldrivestatusstr[prevstate]);
		printf ("state %d: %s\n", 
				newstate,
				logicaldrivestatusstr[newstate]);
	}
	else if (CompareEvent(event, 4,0,0)) {
		printf ("physical drive %d has failed with failurecode %d.\n",
				event.detail.phystatchange.physicaldrivenumber,
				event.detail.phystatchange.failurereason);
		if (event.detail.phystatchange.configureddriveflag != 0) {
			printf("this drive is part of a logical drive\n");
		}
		if (event.detail.phystatchange.sparedriveflag != 0) {
			printf ("this drive is a hot-spare for a logical drive\n");
		}
		
	}
}

int
cciss_get_num_logicalluns(cciss_report_logicallun_struct logluns) {
  int listlength = 0;

  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[0])) << 24;
  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[1])) << 16;
  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[2])) << 8;
  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[3]));
  return listlength / 8;
}

void
cciss_print_logicalluns(cciss_report_logicallun_struct logluns) {
  printf ("Number of logical volumes (%02X %02X %02X %02X) : %d\n", 
	  logluns.LUNlist_len[0],
	  logluns.LUNlist_len[1],
	  logluns.LUNlist_len[2],
	  logluns.LUNlist_len[3],
	  cciss_get_num_logicalluns(logluns));
}


/* get the drivestates for logical drives attached to this controller
 * result:
 * >0 number of logical drives found
 * 0 no logical drives detected
 * <0 error while trying to get states, see log message
 */
int
cciss_get_drivestates (char *device, logdrv *logdrvs, int maxlogdrvs)
{
  int fd;
  cciss_report_logicallun_struct logluns;
  int num_logical_drives;
  int counter;
  int result;
  cciss_event_type event;

  fd = open(device, O_RDWR);
  if (fd < 0) {
    log ("failed to open device %s: %s\n", device, strerror(errno));
    return -1;
  }

  log ("Retrieving logical drive information from controller %s\n", device);
  if (cciss_get_logical_luns(fd, &logluns) < 0) {
    log ("Retrieval of cciss logical lun data failed (%d)\n", result);
    return -1;
  }

  if (verbose) {
    cciss_print_logicalluns(logluns);
  }
  num_logical_drives = cciss_get_num_logicalluns(logluns);

  log ("Controller %s reports %d logical drives\n", device, num_logical_drives);

  for (counter = 0; counter < num_logical_drives; counter++) {
    logdrvs[counter].devicestr = (char *)malloc(strlen(device)+1);
    strcpy(logdrvs[counter].devicestr, device);
    logdrvs[counter].type = CTTYPE_CCISS;
    logdrvs[counter].drvnum = counter;
    logdrvs[counter].state.state = 0;
    log ("Logical drive %d found on controller %s\n", counter, device);
  }

  int first_time = 1;
  do 
    {
      result = cciss_get_event (fd, first_time, &event);
      if (CompareEvent(event, 5,0,0)) {
	/* i'm only interested in logical drive state for now */
	int drivenum = event.detail.logstatchange.logicaldrivenumber;
	logdrvs[drivenum].state.state = event.detail.logstatchange.newlogicaldrivestate;
	logdrvs[drivenum].state.severity = logicaldrivestatusseverity[event.detail.logstatchange.newlogicaldrivestate];
	logdrvs[drivenum].state.message = (char *)malloc(strlen(logicaldrivestatusstr[event.detail.logstatchange.newlogicaldrivestate] + 1));
	strcpy (logdrvs[drivenum].state.message, logicaldrivestatusstr[event.detail.logstatchange.newlogicaldrivestate]);
      }
      if (verbose) {
	cciss_print_event (event);
	printf ("\n");
      }
      first_time = 0;
    }
  while (event.class.class != 0);

  return num_logical_drives;
}

int
ida_get_num_logicalluns (int devicefd)
{
  ida_ioctl_t io;
  char buffer[30];
  int cntr;


  /* clear io */
  memset (&io, 0, sizeof (io));

  io.cmd = ID_CTLR;

  if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
    {
      log ("Error in ida ioctl: %s\n", strerror(errno)); 
      return -1;
    }

  //boardid2str (io.c.id_ctlr.board_id, buffer);

  return io.c.id_ctlr.nr_drvs;
}

int
ida_get_drivestate(int devicefd, int logicaldrv)
{
  ida_ioctl_t io;
  ida_ioctl_t io2;
  int nr_blks, blks_tr;

  memset (&io, 0, sizeof (io));

  io.cmd = ID_LOG_DRV;
  io.unit = logicaldrv | UNITVALID;

  if (ioctl (devicefd, IDAPASSTHRU, &io) < 0)
    {
      log ("FATAL: ID_LOG_DRV ioctl failed: %s", strerror(errno));
      return -1;
    }

  memset (&io2, 0, sizeof (io2));

  io2.cmd = SENSE_LOG_DRV_STAT;
  io2.unit = logicaldrv | UNITVALID;

  if (ioctl (devicefd, IDAPASSTHRU, &io2) < 0)
    {
      log ("FATAL: SENSE_LOG_DRV_STAT ioctl failed: %s", strerror(errno));
      return -1;
    }

  return io2.c.sense_log_drv_stat.status;
}

int
ida_get_drivestates (char *device, logdrv *logdrvs, int maxlogdrvs)
{
  int fd;
  int num_logical_drives;
  int counter;
  int result;

  fd = open(device, O_RDWR);
  if (fd < 0) {
    log ("failed to open device %s: %s\n", device, strerror(errno));
    return -1;
  }

  num_logical_drives = ida_get_num_logicalluns(fd);
  if (num_logical_drives < 0) {
    log ("ioctl failed to retrieve number of logical drives\n", NULL);
    return -1;
  }

  for (counter = 0; counter < num_logical_drives; counter++) {
    result = ida_get_drivestate (fd, counter);
    if (result < 0) {
      log ("Error while retrieving state information (%d)\n", result);
      result = 0; /* we should set this to a critical state */
    }
    logdrvs[counter].devicestr = (char *)malloc(strlen(device)+1);
    strcpy(logdrvs[counter].devicestr, device);
    logdrvs[counter].type = CTTYPE_IDA;
    logdrvs[counter].drvnum = counter;
    logdrvs[counter].state.state = result;
    logdrvs[counter].state.severity = logicaldrivestatusseverity[result];
    logdrvs[counter].state.message = (char *)malloc(strlen(logicaldrivestatusstr[result] + 1));
    strcpy(logdrvs[counter].state.message, logicaldrivestatusstr[result]);
    log ("Logical drive %d found on controller %s\n", counter, device);
  }

  return num_logical_drives;
} 

int
main (int argc, char *argv[])
{
	cciss_event_type event;
	logdrv_state *states;
	int fd, option;
	int simulate = 0;
	char *filename = NULL;
	int report_mode = 0;
	int num_logical_drives = 0;
	int reset_event_pointer = 1;
	int result;
	int ida_device = 0; /* only for use with -f , used to determine protocol to use */

	while ((option = getopt (argc, argv, "f:srhoi")) != EOF)
	{
		switch (option)
		{
		case 'f':
			filename = (char *) malloc (strlen (optarg) + 1);
			strncpy (filename, optarg, strlen (optarg) + 1);
			break;
		case 's':
			simulate = 1;
			break;
		case 'r':
			report_mode = 1;
			verbose = 1;
			break;
		case 'o':
		  reset_event_pointer = 0;
		  break;
		case 'i':
		  ida_device = 1;
		  break;
		case 'h':
		default:
			printf ("Usage: ccissprobe [-f filename] [-s]\n");
			printf (" -f <device>  : device to open\n");
			/* printf (" -s             : simultion mode (use with -f)\n"); */
			printf (" -r             : report (verbose) mode\n");
			printf (" -o             : only read new events (since last run, CCISS devices only)\n");
			printf (" -i             : force ida ioctls. (use with -f if the device is supported by the ida driver)");
			exit (1);
		}
	}


	/* prepare structures */
	int max_logical = 64; /* hardcoded */
	int cur_logical = 0; /* number of drives detected */
	logdrv *logdrvs = (logdrv *)malloc(sizeof(logdrv)*max_logical);

	/* If a device is supplied on the commandline use that device,
	 * otherwise scan for devices in all known places
	 */
	if (filename != NULL)
	{
	  if (ida_device) {
	    result = ida_get_drivestates(filename, logdrvs, max_logical);
	    if (result > 0) {
	      cur_logical += result;
	    }
	  }
	  else {
	    result = cciss_get_drivestates(filename, logdrvs, max_logical);
	    if (result > 0) {
	      cur_logical += result;
	    }
	  }
	}
	else {
	  /* if nothing is supplied on the commandline check the first cciss controller and the first ida controller */
	  result = cciss_get_drivestates("/dev/cciss/c0d0", logdrvs, max_logical);
	  if (result > 0) {
	    cur_logical += result;
	  }
	  result = ida_get_drivestates("/dev/ida/c0d0", logdrvs, max_logical);
	  if (result > 0) {
	    cur_logical += result;
	  }
	}

	if (cur_logical == 0) {
	  /* no logical drives found, this is bad */
	  printf ("CRITICAL no logical drives detected\n");
	  return (2);
	}

	/* Nagios part
	 * nagios wants only one line with a status, so we print the worst situation we can find
	 * and exit with a corresponding return code
         */
	num_logical_drives = 0;
	int worst_disk;
        int worst_sev = SEV_NORMAL;
	int cntr;
	for (cntr = 0; cntr<cur_logical; cntr++) {
	  if (logdrvs[cntr].state.state != 0) {
	    if (logdrvs[cntr].state.severity > worst_sev) {
	      worst_sev = logdrvs[cntr].state.severity;
	      worst_disk = cntr;
	    }
	  }
	  if (verbose) {
	    printf ("Logical drive %d on controller %s has state %d\n", 
		    logdrvs[cntr].drvnum, 
		    logdrvs[cntr].devicestr, 
		    logdrvs[cntr].state.state);
	  }
	}

	if (worst_sev == SEV_CRITICAL) {
	  printf ("CRITICAL Arrayprobe Logical drive %d on %s: %s\n", 
		  logdrvs[worst_disk].drvnum, 
		  logdrvs[worst_disk].devicestr, 
		  logdrvs[worst_disk].state.message);
	  return 2;
	}
	else if (worst_sev == SEV_WARNING) {
	  printf ("WARNING Arrayprobe Logical drive %d on %s: %s\n", 
		  logdrvs[worst_disk].drvnum,
		  logdrvs[worst_disk].devicestr,
		  logdrvs[worst_disk].state.message);
	  return 1;
	}

	printf ("OK Arrayprobe All controllers ok\n");
	return 0;
}
