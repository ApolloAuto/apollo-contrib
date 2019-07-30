/*
 * Get time from GPS and set system time in usec level accuracy.
 * Kun Huang @ Baidu, copyright reserved 2016
 * Modified by youxiangtao @date 2018/01/30
 * Usage: you need to be root to set time.
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include "linux/zynq_api.h"

#define VERSION "1.2.1.2"

static char g_dev_path[64] = "/dev/zynq_gps0";
static int g_verbose = 0;

static int init_fpga()
{
	int fd;
	int rt;

	fd = open(g_dev_path, O_RDWR);
	if (fd < 0) {
		perror("error open gps device");
		return -1;
	}

	rt = ioctl(fd, ZYNQ_IOC_GPS_FPGA_INIT);

	close(fd);

	if (rt == 0) {
		printf("Successfully initialized FPGA time!\n");
		return 0;
	}

	if (errno == EAGAIN) {
		printf("FPGA time already initialized!\n");
		return 0;
	}

	perror("error ioctl ZYNQ_IOC_GPS_FPGA_INIT");

	return rt;
}

static int get_gprmc(char *gprmc)
{
	int fd = 0;
	int rt = 0;
	char gprmc_val[ZYNQ_GPS_GPRMC_VAL_SZ + 1] = { 0 };
	char fields[14][40] = {
	    "Message ID", "UTC Time (hhmmss.sss)", "Status (A = data valid)",
	    "Latitude (ddmm.mmmm)", "N/S indicator", "Longitude (dddmm.mmmm)",
	    "E/W indicator", "Speed over ground (knots)",
	    "Course over ground (degrees)", "Date (ddmmyy)",
	    "Magnetic Variation (degrees)", "E/W indicator",
	    "Mode (see GlobalTop manual)", "Checksum"
	};
	char values[14][40];
	char *pch = NULL;
	char str[80] = "";
	// "MC,174830.000,A,3724.5615,N,12201.4532,W,0.03,247.94,310516,,,D*7A";
	int i = 0;

	fd = open(g_dev_path, O_RDWR);
	if (fd < 0) {
		perror("error open gps device");
		rt = -1;
		return rt;
	}

	rt = ioctl(fd, ZYNQ_IOC_GPS_GPRMC_GET, (void *)gprmc_val);
	if (rt != 0) {
		close(fd);
		perror("error ioctl ZYNQ_IOC_GPS_GPRMC_GET");
		return rt;
	}
	close(fd);

	gprmc_val[ZYNQ_GPS_GPRMC_VAL_SZ] = '\0';
	printf("GPRMC msg: %s\n", (char *)gprmc_val);

	memcpy((void *)gprmc, (const void *)gprmc_val, ZYNQ_GPS_GPRMC_VAL_SZ);
	memcpy((void *)str, (const void *)gprmc_val, ZYNQ_GPS_GPRMC_VAL_SZ);

	pch = strtok(str, ",");
	while(++i && i<11) {
		if (pch == NULL) {
			printf("%-30s: %-20s\n", fields[i-1], "N/A");
			continue;
		}
		strncpy(values[i-1], pch, 40);
		printf("%-30s: %-20s\n", fields[i-1], pch);
		pch = strtok(NULL, ",");
	}

	if (values[2][0] != 'A') {
		printf("GPRMC not valid\n");
		return 1;
	}

	return rt;
}

static int get_gps_time(struct timeval *now)
{
	// Register GPS_TIME_LO: ms[12],us[12],ns[7],valid[1]
	// Register GPS_TIME_HI: reserved[8],Hr[8],Mins[8],secs[8]
	// Register GPS_TIME_DATE: 00ddmmyy
	unsigned int reg_lo;
	unsigned int reg_hi;
	unsigned int reg_date;
	unsigned int hh = 0, mm = 0, ss = 0, ms = 0, us = 0;
	unsigned int year, mon, day;
	struct tm tm;
	time_t time;
	int fd = 0;
	int rt = 0;
	int locked = 0;
	unsigned char gps_val[ZYNQ_GPS_VAL_SZ] = { 0 };
	char date[16] = { 0 };

	memset(&tm, 0, sizeof(struct tm));

	fd = open(g_dev_path, O_RDWR);
	if (fd < 0) {
		perror("error open gps device");
		rt = -1;
		return rt;
	}

	rt = ioctl(fd, ZYNQ_IOC_GPS_GET, (void *)gps_val);
	if (rt != 0) {
		close(fd);
		perror("error ioctl ZYNQ_IOC_GPS_GET");
		return rt;
	}
	close(fd);

	reg_lo = *((int *)&gps_val[0]);
	reg_hi = *((int *)&gps_val[4]);
	reg_date = *((int *)&gps_val[8]);
	if (g_verbose) {
		printf("reg value: DATE 0x%x, HI 0x%x, LO 0x%x\n",
		    reg_date, reg_hi, reg_lo);
	}

	locked = ((reg_lo & 0x01));

	hh = (reg_hi >> 16) & 0xff;
	mm = (reg_hi >> 8) & 0xff;
	ss = reg_hi & 0xff;
	ms = reg_lo >> 20;
	us = (reg_lo >> 8) & 0xfff;
	year = reg_date & 0xff;
	mon = (reg_date >> 8) & 0xff;
	day = (reg_date >> 16) & 0xff;

	if (g_verbose) {
		printf("year %02u, month %02u, day %02u, "
		    "hh %02u, mm %02u, ss %02u, ms %03u, us %03u\n",
		    year, mon, day, hh, mm, ss, ms, us);
	}

	now->tv_sec = (hh * 3600) + (mm * 60) + ss;
	now->tv_usec = (ms * 1000) + us;

	sprintf(date, "%02u%02u%02u", day, mon, year);
	strptime(date, "%d%m%y", &tm);
	time = mktime(&tm);

	now->tv_sec += time;

	if (locked) {
		printf("GPS Time:  %ld.%06ld\n",
		    now->tv_sec, now->tv_usec);
	} else {
		printf("FPGA Time: %ld.%06ld (GPS not locked)\n",
		    now->tv_sec, now->tv_usec);
	}

	return rt;
}

static int get_sys_time(struct timeval *now)
{
	int rt = 0;

	rt = gettimeofday(now, NULL);
	if (rt != 0) {
		perror("error getting system time");
	}

	printf("SYS Time:  ");
	printf("%ld.%06ld\n", now->tv_sec, now->tv_usec);

	return rt;
}

static int gps_sys_sync()
{
	int fd = 0;
	int rt = 0;

	fd = open(g_dev_path, O_RDWR);
	if (fd < 0) {
		perror("error open gps device");
		rt = -1;
		return rt;
	}

	rt = ioctl(fd, ZYNQ_IOC_GPS_SYNC);
	if (rt) {
		perror("error ioctl ZYNQ_IOC_GPS_SYNC");
	} else {
		printf("Successfully sync system time with GPS time!\n");
	}

	close(fd);

	return rt;
}

int main (int argc, char **argv)
{
	struct timeval tv;
	char msg[128] = { 0 };
	int flag_p = 0, flag_m = 0, flag_s = 0, flag_i = 0;
	int c = 0, rt = 0;
	int i = 0;

	printf("Software version: %s\n", VERSION);
	while ((c = getopt(argc, argv, "ipmsvhd:")) != -1) {
		switch (c) {
		case 'i':
			flag_i = 1;
			break;
		case 'p':	// get gps time
			flag_p = 1;
			break;
		case 'm':	// get gprmc msg
			flag_m = 1;
			break;
		case 's':	// sync system time
			flag_s = 1;
			break;
		case 'v':	// verbose
			g_verbose = 1;
			break;
		case 'd':
			if (optarg == NULL) {
				printf("Please specify a GPS device path\n");
				return -1;
			}
			strncpy(g_dev_path, optarg, sizeof(g_dev_path));
			g_dev_path[sizeof(g_dev_path) - 1] = '\0';
			break;
		case 'h':
		default:
			printf("Usage: %s [-gpmvh] [-d /dev/zynq_gps#]\n"
			    "\t[-d /dev/zynq_gps#]: GPS device path\n"
			    "\t[-s]: Sync system time with GPS time\n"
			    "\t[-p]: Print GPS time\n"
			    "\t[-m]: GPRMC message data\n"
			    "\t[-i]: Initialize FPGA time with system time\n"
			    "\t[-v]: Verbose\n"
			    "\t[-h]: Help\n",
			    argv[0]);
			return -1;
		}
	}

	rt = setenv("TZ", "UTC", 1);
	if (rt < 0) {
		perror("error setenv");
		return rt;
	}

	if (flag_i) {
		return init_fpga();
	}

	if (flag_p) {
		(void) get_sys_time(&tv);
		return get_gps_time(&tv);
	}

	if (flag_m) {
		for (i = 0; i < 10; i++) {
			rt = get_gprmc(msg);
			if (rt == 0) {
				break;
			}
			sleep(1);
		}
		return rt;
	}

	if (flag_s) {
		return gps_sys_sync();
	}

	return 0;
}
