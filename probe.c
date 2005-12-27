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

#include <linux/compiler.h>
#include <linux/cciss_ioctl.h>

#include "cciss_events.h"

#include "config.h"

typedef struct _logdrv_state {
  int state;
  char *message;
  int severity;
} logdrv_state;

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
	iocommand.Request.CDB[7] = (reset_pointer) ? 0x3 : 0x7;	/* bit 2 set = reset pointer, bit 0 set = synchronous mode */
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

int
main (int argc, char *argv[])
{
	cciss_event_type event;
	cciss_report_logicallun_struct logluns;
	logdrv_state *states;
	int fd, option;
	int simulate = 0;
	char *filename = NULL;
	int report_mode = 0;
	int num_logical_drives = 0;
	int only_new_events = 0;

	while ((option = getopt (argc, argv, "f:srho")) != EOF)
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
			break;
		case 'o':
		  only_new_events = 1;
		  break;
		case 'h':
		default:
			printf ("Usage: ccissprobe [-f filename] [-s]\n");
			printf (" -f <filename>  : device to open\n");
			printf (" -s             : simultion mode (use with -f)\n");
			printf (" -r             : report mode\n");
			printf (" -o             : only read new events (since last run)\n");
			exit (1);
		}
	}

	if (filename == NULL)
	{
		fd = open ("/dev/cciss/c0d0", O_RDWR);
	}
	else
	{
		fd = open (filename, O_RDWR);
	}
	if (fd < 0)
	{
		perror (" * controller open failed");
		exit (2);
	}
	if (report_mode) {
	  printf ("CCISS probe\n");
	  printf (" * opening device.\n");
	  printf (" * device c0d0 opened.\n");
	}

	if (simulate != 1) {
	  cciss_get_logical_luns(fd, &logluns);
	  if (report_mode) {
	    cciss_print_logicalluns(logluns);
	  }
	  num_logical_drives = cciss_get_num_logicalluns(logluns);
	}
	else {
	  /* set to 8 logical drives in simulation mode */
	  num_logical_drives = 8;
	}
	states = (logdrv_state *)malloc(num_logical_drives * sizeof(logdrv_state));
	bzero(states, num_logical_drives * sizeof(logdrv_state));

	int first_time = 1;
	do 
	  {
	    if (simulate)
	      {
		cciss_simulate_get_event (fd, only_new_events && first_time, &event);
	      }
	    else
	      {
		cciss_get_event (fd, only_new_events && first_time, &event);
	      }
	    if (CompareEvent(event, 5,0,0)) {
	      /* i'm only interested in logical drive state for now */
	      int drivenum = event.detail.logstatchange.logicaldrivenumber;
	      states[drivenum].state = event.detail.logstatchange.newlogicaldrivestate;
	      states[drivenum].severity = logicaldrivestatusseverity[event.detail.logstatchange.newlogicaldrivestate];
	      states[drivenum].message = (char *)malloc(strlen(logicaldrivestatusstr[event.detail.logstatchange.newlogicaldrivestate] + 1));
	      strcpy (states[drivenum].message, logicaldrivestatusstr[event.detail.logstatchange.newlogicaldrivestate]);
	    }
	    if (report_mode) {
	      cciss_print_event (event);
	      printf ("\n");
	    }
	    first_time = 0;
	  }
	while (event.class.class != 0);


	/* Nagios part
	 * nagios wants only one line with a status, so we print the worst situation we can find
	 * and exit with a corresponding return code
         */
	int worst_lun;
        int worst_sev = SEV_NORMAL;
	int cntr;
	for (cntr = 0; cntr<num_logical_drives; cntr++) {
	  if (states[cntr].state != 0) {
	    if (states[cntr].severity > worst_sev) {
	      worst_sev = states[cntr].severity;
	      worst_lun = cntr;
	    }
	  }
	}

	close (fd);

	if (worst_sev == SEV_CRITICAL) {
	  printf ("CRITICAL Arrayprobe Logical drive %d: %s\n", worst_lun, states[worst_lun].message);
	  return 2;
	}
	else if (worst_sev == SEV_WARNING) {
	  printf ("WARNING Arrayprobe Logical drive %d: %s\n", worst_lun, states[worst_lun].message);
	  return 1;
	}

	printf ("OK Arrayprobe All drives ok\n");
	return 0;
}
