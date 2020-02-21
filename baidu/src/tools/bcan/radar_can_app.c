/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "bcan.h"

#define	VERSION			"3.0.0.1"
#define	SHOW_STATS_SEC		300 /* 300s == 5 minutes */

uint64_t gnum_msg = 12;
uint64_t gto = 100; /* rx timeout in ms */
int g_err = 0;
int baudrate = 1;

uint32_t g_rx_id = 1;
int print_stats = 0;

const char *g_baudrate_desc[BCAN_BAUDRATE_NUM] = {
	"1Mbps",
	"500kbps",
	"250kbps",
	"150kbps",
};

void *rx_func(void *arg);
void print_usage(char *name);
void print_status(bcan_hdl_t hdl, int id);
int configure(bcan_hdl_t *hdl, int channel_id);
void do_sigalrm(int sig);
struct timespec stats_time_start = {0, 0};
struct timespec stats_time_last = {0, 0};
uint64_t num_msg_last = 0;

static int g_keep_running = 1;

static void int_handler()
{
	g_keep_running = 0;
	printf("Program interrupted.\n");
}

int main(int argc, char **argv)
{
	bcan_hdl_t hdl;
	pthread_t  rx_thr;
	struct sigaction sa;
	extern char *optarg;
	int c;

	printf("software version: %s, libbcan version: %s\n",
	    VERSION, bcan_get_libversion());

	memset(&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = int_handler;
	sigaction(SIGINT, &sa, NULL);

	while ((c= getopt(argc, argv, "hb:n:t:d:")) != EOF) {
		switch (c) {
		case '?':
		case 'h':
			print_usage(argv[0]);
			return 0;
		case 'n':
			gnum_msg = (uint64_t)strtol(optarg, NULL, 10);
			break;
		case 'b':
			baudrate = optarg[0] - '0';
			break;
		case 't':
			gto = strtol(optarg, NULL, 10);
			break;
		case 'd':
			g_rx_id = optarg[0] - '0';
			break;
		default:
			printf("wrong arguments\n");
			print_usage(argv[0]);
			return -1;
		}
	}

	if (baudrate < 0 || baudrate >= BCAN_BAUDRATE_NUM) {
		printf("buadrate setting is wrong, set default baudrate to 500kbps.");
		baudrate = 1;
	}


	printf("baudrate = %s, rx_to = %" PRIu64 ", gnum_msg = %ld, "
	    "rx_channel_id = %" PRIu32 "\n",
	    g_baudrate_desc[baudrate], gto, (int64_t)gnum_msg, g_rx_id);

	if (configure(&hdl, g_rx_id)) {
		return -1;
	}

	if (pthread_create(&rx_thr, NULL, rx_func, (void *)hdl) != 0) {
		printf("rx_thr create failed\n");
	        goto err;
	}

	/* enable alarm signal */
	if (signal(SIGALRM, do_sigalrm) == SIG_ERR) {
		printf("register sigalrm failed\n");
		goto err;
	}
	clock_gettime(CLOCK_MONOTONIC, &stats_time_start);
	stats_time_last = stats_time_start;
	alarm(SHOW_STATS_SEC); /* every 5 minutes */

	pthread_join(rx_thr, NULL);

	if (bcan_stop(hdl) != 0) {
		printf("bcan_stop failed\n");
	}

err:
	if (bcan_close(hdl) != 0) {
		printf("bcan_close failed\n");
	}

	return g_err;
}

void do_sigalrm(int UNUSED(sig))
{
	print_stats = 1;
	alarm(SHOW_STATS_SEC);
}

/* return time diffrence in second */
static long timespec_diff(struct timespec *start, struct timespec *end)
{
	return (1000000000L * (end->tv_sec - start->tv_sec) +
		(end->tv_nsec - end->tv_nsec)) / 1000000000L;
}

void print_statistics(uint64_t num_msg)
{
	struct timespec stats_time_now = {0, 0};
	long timediff_sec_last;
	long timediff_sec;

	clock_gettime(CLOCK_MONOTONIC, &stats_time_now);
	timediff_sec = timespec_diff(&stats_time_start, &stats_time_now);
	timediff_sec_last = timespec_diff(&stats_time_last, &stats_time_now);

	if (timediff_sec != 0 && timediff_sec_last != 0) {
		printf("\n******statistics: total_num_msg=%lu, "
		    "avg# msgs/sec=%0.3f, last msgs/sec=%lu******\n\n",
		    num_msg, ((double)num_msg)/timediff_sec,
		    (num_msg - num_msg_last)/timediff_sec_last);

		stats_time_last = stats_time_now;
		num_msg_last = num_msg;
	}
}

int configure(bcan_hdl_t *hdl, int channel_id)
{
	int rtn;

	rtn = bcan_open(channel_id, 0, gto, gto, hdl);
	if (rtn != 0) {
		printf("bcan_open failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
		goto err1;
	}
	printf("bcan_open success for channel %d\n", channel_id);

	rtn = bcan_set_baudrate(*hdl, baudrate);
	if (rtn != 0) {
		printf("bcan_set_baudrate failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
		goto err;
	}
	printf("bcan_set_baudrate %s success\n", g_baudrate_desc[baudrate]);

	rtn = bcan_start(*hdl);
	if (rtn != 0) {
		printf("bcan_start failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
		goto err;
	}
	printf("bcan_start success\n");

	return 0;

err:
	rtn = bcan_close(*hdl);
	if (rtn != 0) {
		printf("bcan_close failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
	}
err1:
	return -1;
}

#define	STATS_PRINT_NUM		1000
void *rx_func(void *arg)
{
	bcan_hdl_t hdl = (bcan_hdl_t)arg;
	int j, tmp_num;
	uint64_t num_msg = 0;
	uint8_t tx_radiate = 0;
	bcan_msg_t *buf;
	bcan_msg_t tmp_buf;

	buf = malloc(sizeof(bcan_msg_t) * 1024 * 1024);
	if (!buf) {
		perror("malloc");
		goto err;
	}

	while (g_keep_running) {
		if (!tx_radiate && (num_msg == 10)) {
			tmp_buf.bcan_msg_datalen = 8;
			tmp_buf.bcan_msg_id = 0x4f1;
			tmp_buf.bcan_msg_data[0] = 0;
			tmp_buf.bcan_msg_data[1] = 0;
			tmp_buf.bcan_msg_data[2] = 0;
			tmp_buf.bcan_msg_data[3] = 0;
			tmp_buf.bcan_msg_data[4] = 0;
			tmp_buf.bcan_msg_data[5] = 0;
			tmp_buf.bcan_msg_data[6] = 0xBF;
			tmp_buf.bcan_msg_data[7] = 0;

			tx_radiate = 1;
			if ((tmp_num = bcan_send_hi_pri(hdl, &tmp_buf)) < 0) {
				printf("TX RADIATE send failed %d\n", tmp_num);
				g_err = 1;
				goto err;
			}
			printf("TX RADIATE send success %d\n", tmp_num);
			continue;
		}

		if ((tmp_num = bcan_recv(hdl, buf, 1)) < 0) {
			printf("bcan_recv failed: %s (%d). total msg recvd %d\n",
			    bcan_get_err_msg(tmp_num), tmp_num, (int)num_msg);
			print_status(hdl, g_rx_id);
			g_err = 1;
			goto err;
		}

		printf("\tbcan_recv %lu: msg_id 0x%x, datalen %d "
		    "bytes, TS: %ld.%.6ld, data:\n", num_msg,
		    buf->bcan_msg_id, buf->bcan_msg_datalen,
		    buf->bcan_msg_timestamp.tv_sec,
		    buf->bcan_msg_timestamp.tv_usec);
		printf("\t\t");
		for (j = 0; j < buf->bcan_msg_datalen; j++) {
			printf("0x%x ", buf->bcan_msg_data[j]);
		}
		printf("\n");
		num_msg++;

		if (print_stats != 0) {
			print_statistics(num_msg);
			print_stats = 0;
		}

		if (num_msg >= gnum_msg) {
			break;
		}
	}

err:
	print_statistics(num_msg);
	print_status(hdl, g_rx_id);
	if (g_err) {
		printf("TEST FAILED\n");
	} else {
		printf("TEST PASSED\n");
	}
	if (buf) {
		free(buf);
	}
	pthread_exit(NULL);
}

void print_status(bcan_hdl_t hdl, int id)
{
	int status;
	uint8_t rx_err, tx_err;

	if ((status = bcan_get_status(hdl)) < 0) {
		printf("%s: channel %d, bcan_get_status() error: %s (%d)\n",
		    __func__, id, bcan_get_err_msg(status), status);
	} else {
		printf("Channel %d: status reg 0x%x\n", id, status);
	}

	if ((status = bcan_get_err_counter(hdl, &rx_err, &tx_err)) < 0) {
		printf("%s: channel %d, bcan_get_err_counter() error: %s (%d)\n",
		    __func__, id, bcan_get_err_msg(status), status);
	} else {
		printf("Channel %d: rx_err_counter %d, tx_err_counter %d\n",
		    id, rx_err, tx_err);
	}
}

void print_usage(char *name)
{
	printf("Usage: %s -d < > -n < > -t < > -b < > -v\n"
	    "\t-d -> specify Rx channel id. e.g. -d 1 (Rx on 1)\n"
	    "\t-n -> specify number of messages to be received. (-1 for ever)\n"
	    "\t-t -> specify Rx timeout value in ms (decimal)\n"
	    "\t-b -> specify baudrate "
	    "(0 -> 1Mbps, 1 -> 500Kbps, 2 -> 250Kbps,  3 -> 150Kbps)\n",
	    name);
}
