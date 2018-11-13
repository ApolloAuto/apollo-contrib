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
#define	RX_THREAD_WAIT_MS	10	/* in ms */

#define	TEST_STD_FRAME_ID	0x72
#define	TEST_EXT_FRAME_ID	0x18ff84a9
#define	TEST_DATA_LEN		8

int gnum_msg = 9;
int is_loopback = 0;
uint64_t g_txto = 10; /* tx timeout in ms */
uint64_t g_rxto = 100; /* rx timeout in ms */
uint64_t g_val = 0xAB;
int g_err = 0;
uint32_t g_tx_id = 0;
uint32_t g_rx_id = 1;
int g_debug = 0;
int g_ext_frame = 0;

void *tx_func(void *arg);
void *rx_func(void *arg);
void print_usage(char *name);
void print_status(bcan_hdl_t hdl, int id, const char *prefix);
int configure(bcan_hdl_t *hdl, int channel_id);

pthread_mutex_t plock;
static int g_num_sent = 0;
static int g_num_recv = 0;
static int g_tx_running = 0;
static int g_rx_running = 0;
static int tx_err = 0;
static int rx_err = 0;

#define	xprintf(...)	\
	pthread_mutex_lock(&plock);	\
	printf(__VA_ARGS__);		\
	pthread_mutex_unlock(&plock);

static void int_handler()
{
	g_tx_running = 0;
	printf("Program interrupted.\n");
}

int main(int argc, char **argv)
{
	bcan_hdl_t hdl, hdl2;
	pthread_t tx_thr1, rx_thr1;
	struct sigaction sa;
	extern char *optarg;
	int num;
	int c;

	printf("software version: %s, libbcan version: %s\n",
	    VERSION, bcan_get_libversion());

	memset(&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = int_handler;
	sigaction(SIGINT, &sa, NULL);

	while ((c= getopt(argc, argv, "hln:t:d:eD")) != EOF) {
		switch (c) {
		case '?':
		case 'h':
			print_usage(argv[0]);
			return 0;
		case 'l':
			is_loopback = 1;
			break;
		case 'n':
			gnum_msg = strtol(optarg, NULL, 10);
			if (gnum_msg > BCAN_MAX_TX_MSG) {
				printf("wrong value specified in -n option: "
				    "exceed BCAN_MAX_TX_MSG %d\n",
				    BCAN_MAX_TX_MSG);
				return -1;
			}
			break;
		case 't':
			g_txto = strtol(optarg, NULL, 10);
			break;
		case 'd':
			g_tx_id = optarg[0] - '0';
			g_rx_id = optarg[1] - '0';
			break;
		case 'D':
			g_debug = 1;
			break;
		case 'e':
			g_ext_frame = 1;
			break;
		default:
			printf("unsupported argument\n");
			print_usage(argv[0]);
			return -1;
		}
	}

	if (is_loopback) {
		g_rx_id = g_tx_id;
	}

	num = (gnum_msg <= 0) ? 1 : gnum_msg;
	g_rxto = g_txto * num + RX_THREAD_WAIT_MS;
	printf("is_loopback = %d, baudrate = 500kbps, tx_to = %" PRIu64 "ms, "
	    "gnum_msg = %" PRId32 ", tx_channel_id = %" PRIu32
	    ", rx_channel_id = %" PRIu32 "\n",
	    is_loopback, g_txto, gnum_msg, g_tx_id, g_rx_id);

	if (configure(&hdl, g_tx_id)) {
		printf("configure Tx channel %" PRIu32 " failed.\n", g_tx_id);
		return -1;
	}

	if (!is_loopback && configure(&hdl2, g_rx_id)) {
		bcan_close(hdl);
		printf("configure Rx channel %" PRIu32 " failed.\n", g_rx_id);
		return -1;
	}

	if (pthread_create(&rx_thr1, NULL, rx_func,
	    ((is_loopback) ? (void *)hdl : (void *)hdl2)) != 0) {
		printf("rx_thr create failed\n");
		goto err;
	}

	if (pthread_create(&tx_thr1, NULL, tx_func, (void *)hdl) != 0) {
		printf("tx_thr create failed\n");
		goto err;
	}

	pthread_join(tx_thr1, NULL);
	pthread_join(rx_thr1, NULL);

	if (bcan_stop(hdl) != 0) {
		printf("bcan_stop hdl failed\n");
	}
	if (!is_loopback && (bcan_stop(hdl2) != 0)) {
		printf("bcan_stop hdl2 failed\n");
	}

	if (g_err) {
		printf("TEST FAILED\n");
	} else {
		printf("TEST PASSED\n");
	}
err:
	if (bcan_close(hdl) != 0) {
		printf("bcan_close hdl failed\n");
	}
	if (!is_loopback && (bcan_close(hdl2) != 0)) {
		printf("bcan_close hdl2 failed\n");
	}

	return tx_err + rx_err;
}

int configure(bcan_hdl_t *hdl, int channel_id)
{
	int rtn;

	rtn = bcan_open(channel_id, 0, g_txto, g_rxto, hdl);
	if (rtn != 0) {
		printf("bcan_open failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
		goto err1;
	}
	printf("bcan_open success for channel %d\n", channel_id);

	rtn = bcan_set_baudrate(*hdl, BCAN_BAUDRATE_500K);
	if (rtn  != 0) {
		printf("bcan_set_baudrate failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
		goto err;
	}
	printf("bcan_set_baudrate 500kbps success\n");

	rtn = bcan_start(*hdl);
	if (rtn != 0) {
		printf("bcan_start failed for channel %d: %s (%d)\n",
		    channel_id, bcan_get_err_msg(rtn), rtn);
		goto err;
	}
	printf("bcan_start success\n");

	if (is_loopback) {
		rtn = bcan_set_loopback(*hdl);
		if (rtn != 0) {
			printf("bcan_set_loopback failed for channel %d: %s (%d)\n",
			    channel_id, bcan_get_err_msg(rtn), rtn);
			goto err;
		}
		printf("bcan_set_loopback success\n");
	}

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

static void check_messages(bcan_msg_t *buf, int num)
{
	bcan_msg_t *msg;
	uint32_t msgid;
	uint8_t tmp_val;
	int i, j;

	if (g_ext_frame) {
		msgid = TEST_EXT_FRAME_ID | BCAN_EXTENDED_FRAME;
	} else {
		msgid = TEST_STD_FRAME_ID;
	}
	tmp_val = g_val + ((g_num_recv - num) * TEST_DATA_LEN);
	pthread_mutex_lock(&plock);
	for (i = 0; i < num; i++) {
		msg = buf + i;
		printf("\tbcan_recv: start_of_msg %d\n", i + 1);
		printf("\tbcan_recv: msg_id 0x%x", msg->bcan_msg_id);
		if (msg->bcan_msg_id != msgid) {
			printf("[ERR]");
			g_err = 1;
		}
		printf(", datalen %d bytes, TS: %ld.%06ld, data:\n",
		    msg->bcan_msg_datalen,
		    msg->bcan_msg_timestamp.tv_sec,
		    msg->bcan_msg_timestamp.tv_usec);
		printf("\t\t");
		for (j = 0; j < msg->bcan_msg_datalen; j++) {
			printf("0x%02x ", msg->bcan_msg_data[j]);
			if (msg->bcan_msg_data[j] != tmp_val++) {
				printf("[ERR] ");
				g_err = 1;
			}
			if (msg->bcan_msg_data[j] == 0xFF) {
				tmp_val = 0;
			}
		}
		printf("\n\tbcan_recv: end_of_msg %d\n", i + 1);
	}
	pthread_mutex_unlock(&plock);
}

void *tx_func(void *arg)
{
	bcan_hdl_t hdl = (bcan_hdl_t)arg;
	int i, j, num_msg, c;
	uint8_t tmp_val;
	uint32_t msgid;
	bcan_msg_t *buf, *msg;
	struct timespec ts;

	buf = malloc(sizeof(bcan_msg_t) * 1024 * 1024);
	if (!buf) {
		perror("malloc");
		goto err;
	}

	tmp_val = g_val;
	if (g_ext_frame) {
		msgid = TEST_EXT_FRAME_ID | BCAN_EXTENDED_FRAME;
	} else {
		msgid = TEST_STD_FRAME_ID;
	}

	while (!g_rx_running) {
		usleep(1000);
	}

	// Wait a little bit to make sure RX runs before TX does. If TX runs
	// first, the message transmitted may be received by driver before
	// RX actually runs and then RX won't receive anything and times-out.
	// Sleep of 10ms is not bullet-proof (in making sure RX runs first)
	// in theory, but it will be extremely rare for TX to race ahead of RX
	// after the sleep.
	usleep(RX_THREAD_WAIT_MS * 1000);

	xprintf("tx thread started, transmitting %" PRId32
	    " messages, data starts with 0x%x\n", gnum_msg, tmp_val);

	num_msg = (gnum_msg <= 0) ? 1: gnum_msg;
	g_tx_running = 1;

	while (g_tx_running) {
		msg = buf;
		for (i = 0; i < num_msg; i++, msg++) {
			msg->bcan_msg_id = msgid;
			msg->bcan_msg_datalen = TEST_DATA_LEN;
			for (j = 0 ; j < msg->bcan_msg_datalen; j++) {
				msg->bcan_msg_data[j] = tmp_val++;
			}
			if (tmp_val == 0xFF) {
				tmp_val = 0;
			}
		}

		if ((c = bcan_send(hdl, buf, num_msg)) < 0) {
			xprintf("bcan_send failed: %s (%d), total msg sent %d\n",
			    bcan_get_err_msg(c), c, g_num_sent);
			++tx_err;
			goto err;
		}

		clock_gettime(CLOCK_REALTIME, &ts);
		g_num_sent += c;
		xprintf("bcan_send: sent %d msg, total msg sent %d, TS: %ld.%06ld\n",
		    c, g_num_sent, ts.tv_sec, ts.tv_nsec / 1000);
		if (c < num_msg) {
			xprintf("bcan_send: not enough msg sent, expect %d\n",
			    num_msg);
			g_tx_running = 0;
		}
		if ((gnum_msg > 0) && (g_num_sent >= gnum_msg)) {
			g_tx_running = 0;
		}
	}

err:
	print_status(hdl, g_tx_id, "bcan_send");
	if (buf) {
		free(buf);
	}
	pthread_exit(NULL);
}

void *rx_func(void *arg)
{
	bcan_hdl_t hdl = (bcan_hdl_t)arg;
	int c, num_msg;
	bcan_msg_t *buf;

	buf = malloc(sizeof(bcan_msg_t) * 1024 * 1024);
	if (!buf) {
		perror("malloc");
		goto err;
	}

	xprintf("rx thread started, to receive %" PRId32 " msgs\n", gnum_msg);

	num_msg = (gnum_msg <= 0) ? 1 : gnum_msg;
	g_rx_running = 1;

	while (1) {
		if ((gnum_msg > 0) && (g_num_recv == gnum_msg)) {
			break;
		}
		if ((c = bcan_recv(hdl, buf, num_msg)) < 0) {
			if (gnum_msg <= 0) {
				if (!g_tx_running && !g_debug) {
					goto err;
				}
			} else {
				xprintf("bcan_recv failed: %s (%d). total msg recvd %d\n",
				    bcan_get_err_msg(c), c, g_num_recv);
				++rx_err;
				g_err = 1;
				if (!g_debug) {
					goto err;
				}
			}
			continue;
		}

		g_num_recv += c;
		xprintf("bcan_recv: recvd %d msg, total msg recvd %d\n",
		    c, g_num_recv);

		check_messages(buf, c);

		if (c < num_msg) {
			xprintf("bcan_recv: not enough msg recvd, expect %d\n",
			    num_msg);
			num_msg -= c;
		} else {
			num_msg = (gnum_msg <= 0) ? 1 : gnum_msg;
		}
	}

err:
	g_rx_running = 0;
	print_status(hdl, g_rx_id, "bcan_recv");
	if (buf) {
		free(buf);
	}
	pthread_exit(NULL);
}

void print_status(bcan_hdl_t hdl, int id, const char *prefix)
{
	int status;
	uint8_t rx_err, tx_err;

	pthread_mutex_lock(&plock);
	if ((status = bcan_get_status(hdl)) < 0) {
		printf("%s: channel %d, bcan_get_status() error: %s (%d)\n",
		    prefix, id, bcan_get_err_msg(status), status);
	} else {
		printf("%s: channel %d, status reg 0x%x\n",
		    prefix, id, status);
	}

	if ((status = bcan_get_err_counter(hdl, &rx_err, &tx_err)) < 0) {
		printf("%s: channel %d, bcan_get_err_counter() error: %s (%d)\n",
		    prefix, id, bcan_get_err_msg(status), status);
	} else {
		printf("%s: channel %d, rx_err_counter %d, tx_err_counter %d\n",
		    prefix, id, rx_err, tx_err);
	}
	pthread_mutex_unlock(&plock);
}

void print_usage(char *name)
{
	printf("Usage: %s -l -d < > -n < > -t < > -e -v\n"
	    "\t-l -> Loopback mode\n"
	    "\t-d -> Tx and Rx channel id. e.g. -d 10 (Tx on 1 and Rx on 0)\n"
	    "\t-n -> Number of messages to be transmitted (Maximum %d)\n"
	    "\t-t -> Tx timeout value in ms\n"
	    "\t-e -> Use extended frames\n",
	name, BCAN_MAX_TX_MSG);
}
