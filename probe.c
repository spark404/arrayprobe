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

#include <linux/cciss_ioctl.h>

#include "cciss_events.h"


int
cciss_get_event (int device_fd, int reset_pointer, cciss_event_type * event)
{
	int result, outfile;
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
	iocommand.Request.CDB[7] = (reset_pointer) ? 0x7 : 0x3;	/* 7 = start at oldest, 3 is get current */
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

	if (iocommand.error_info.CommandStatus == 1) {
		printf (" * Command succeeded with dataoverrun (code %d)\n", iocommand.error_info.CommandStatus);
	}
	else if (iocommand.error_info.CommandStatus == 2) {
		printf (" * Command succeeded with dataunderrun (code %d)\n", iocommand.error_info.CommandStatus);
	}
	else if (iocommand.error_info.CommandStatus != 0)
	{
		printf (" * Command failed with Comnmand Status %d\n", iocommand.error_info.CommandStatus);
		return -1;
	}

	if (reset_pointer)
	{
		outfile =
			open ("notify.bin", O_WRONLY | O_CREAT | O_TRUNC,
			      S_IREAD | S_IWRITE);
	}
	else
	{
		outfile =
			open ("notify.bin", O_WRONLY | O_CREAT | O_APPEND,
			      S_IREAD | S_IWRITE);
	}
	write (outfile, buffer, 512);
	close (outfile);

	memcpy (event, buffer, 512);
	return 0;
}

int
cciss_get_logical_luns (int device_fd, cciss_report_logicallun_struct * logluns)
{
	int result, outfile;
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

	if (iocommand.error_info.CommandStatus == 1) {
		printf (" * Command succeeded with dataoverrun (code %d)\n", iocommand.error_info.CommandStatus);
	}
	else if (iocommand.error_info.CommandStatus == 2) {
		printf (" * Command succeeded with dataunderrun (code %d)\n", iocommand.error_info.CommandStatus);
	}
	else if (iocommand.error_info.CommandStatus != 0)
	{
		printf (" * Command failed with Comnmand Status %d\n", iocommand.error_info.CommandStatus);
		return -1;
	}
	
	outfile = open ("reportLUN.bin", O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	write (outfile, buffer, 128);
	close (outfile);

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

void
cciss_print_logicalluns(cciss_report_logicallun_struct logluns) {
  int listlength = 0;

  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[0])) << 24;
  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[1])) << 16;
  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[2])) << 8;
  listlength |= (0xff & (unsigned int)(logluns.LUNlist_len[3]));
  printf ("Number of logical volumes (%02X %02X %02X %02X) : %d\n", 
	  logluns.LUNlist_len[0],
	  logluns.LUNlist_len[1],
	  logluns.LUNlist_len[2],
	  logluns.LUNlist_len[3],
	  listlength / 8);
}

int
main (int argc, char *argv[])
{
	cciss_event_type event;
	cciss_report_logicallun_struct logluns;
	int fd, option;
	int simulate = 0;
	char *filename = NULL;

	printf ("CCISS probe\n");

	while ((option = getopt (argc, argv, "f:s")) != EOF)
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
		case 'n':
			nagios_mode = 1;
			break;
		case 'h':
		default:
			printf ("Usage: ccissprobe [-f filename] [-s]\n");
			printf (" -f <filename>  : device to open\n");
			printf (" -s             : simultion mode (use with -f)\n");
			printf (" -n             : nagois check mode\n");
			exit (1);
		}
	}

	printf (" * opening device.\n");
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
	printf (" * device c0d0 opened.\n");

	if (simulate != 1) {
	  cciss_get_logical_luns(fd, &logluns);
	  cciss_print_logicalluns(logluns);
	}

	if (simulate)
	{
		cciss_simulate_get_event (fd, 1, &event);
	}
	else
	{
		cciss_get_event (fd, 1, &event);
	}
	cciss_print_event (event);
	printf ("\n");

	while (event.class.class != 0)
	{
		if (simulate)
		{
			cciss_simulate_get_event (fd, 0, &event);
		}
		else
		{
			cciss_get_event (fd, 0, &event);
		}
		cciss_print_event (event);
		printf ("\n");
	}

	close (fd);
	return 0;
}
