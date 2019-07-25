/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
 * @file   sensor_sync.cpp
 * @author krishnaelango@ ericyao@ feiaiguo@
 *         youxiangtao@ (May 2018)
 * @brief  Application to synchronize Cameras and Lidar
 */

#include <memory>
#include <string>
#include <thread>
#include <iostream>
#include <atomic>

#include <fcntl.h>
#include <memory.h>
#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "sensor_sync.h"

#define SENSOR_SYNC_APP_VERSION "1.1.2.1"

using namespace velodyne_driver;

#define TS_PRINT_FMT "%d-%02d-%02d %02d:%02d:%02d"
#define TS_PRINT_DATA(wt) wt.tm_year + 1900, wt.tm_mon + 1, wt.tm_mday, \
        wt.tm_hour, wt.tm_min, wt.tm_sec
#define DEFAULT_READ_TIMEOUT_MS 100
#define DEFAULT_STATS_PRINT_INTV_SEC 5
#define QUIT_READ_INTV_US 200000

enum {
    LOOP_RUN = 0,
    LOOP_STOP = 1,
    LOOP_DONE = 2
};

static std::atomic<int> g_stop_flag(LOOP_RUN);

// Socket read timeout in milli-seconds.
static int g_timeout_ms = DEFAULT_READ_TIMEOUT_MS;
// Stop after this many # of timeouts.
static int g_timeout_total = 0;
// Interval to print status & stats, in seconds.
static uint64_t g_print_intv = DEFAULT_STATS_PRINT_INTV_SEC;
// True to read pcap file as fast as possible.
static int g_read_fast = 0;

static void print_status(Input *input, const VeloProc *velo,
        const char *print_prefix)
{
    std::string gps_st;
    VeloStatus velo_st;

    velo->get_gps_status(gps_st);
    velo->get_status(velo_st);

    struct tm wt;
    time_t tx = (time_t)(velo_st.base_time_sec +
        velo_st.last_gps_timestamp / SEC_TO_USECS);
    wt = *(localtime(&tx));

    unsigned int ts_us = velo_st.last_gps_timestamp % SEC_TO_USECS;

    VELO_LOG_INFO("%s: %s, last packet @" TS_PRINT_FMT
        ".%06u (%lu secs, %010u usecs), invalid_date=%d;\n",
        print_prefix, gps_st.c_str(), TS_PRINT_DATA(wt), ts_us,
        velo_st.base_time_sec, velo_st.last_gps_timestamp,
        velo_st.invalid_date_val);

    input->log_stats();

    if (velo_st.base_time_sec == 0 &&
        ((velo_st.gps_status == VeloProc::GPS_OK) ||
        (velo_st.gps_status == VeloProc::GPS_NMEA_ONLY))) {
        VELO_LOG_DEBUG("    !! no proper GPS time while GPS status "
            "indicates Lidar getting time.\n");
    }
}

static void velo_loop(Input *input, VeloProc *velo, const char *print_prefix)
{
    uint64_t print_intv_us = g_print_intv * MICRO_SECS_IN_SEC;
    uint64_t ts_us = get_time_us();
    uint64_t nxt_print_tm = ts_us + print_intv_us;

    int to_cnt = 0;
    while (g_stop_flag.load() == LOOP_RUN) {
        switch (input->get_firing_data_packet(0)) {
        case Input::FATAL_ERR:
            VELO_LOG_ERR("%s: fata error getting data, can't continue\n",
                print_prefix);
            goto done;
        case Input::END:
            VELO_LOG_ERR("%s: end-of-file, done\n", print_prefix);
            goto done;
        case Input::OK:
            if (velo->proc_firing_data(input->data_buf(), input->pos_buf())) {
                VELO_LOG_ERR("%s: camera triggers not enabled, do nothing\n",
                    print_prefix);
                goto done;
            }
            break;
        case Input::POLL_TIMEOUT:
            VELO_LOG_INFO("%s: poll timeout, try again\n", print_prefix);
            ++to_cnt;
            if (g_timeout_total && to_cnt > g_timeout_total) {
                VELO_LOG_INFO("%s: too many poll timeout, I quit.\n",
                    print_prefix);
                goto done;
            }
            break;
        case Input::ERROR:
            VELO_LOG_ERR("%s: error, try again\n", print_prefix);
            break;
        }

        ts_us = get_time_us();
        if (g_print_intv && ts_us >= nxt_print_tm) {
            print_status(input, velo, print_prefix);
            nxt_print_tm = ts_us + print_intv_us;
        }
    }
    VELO_LOG_INFO("%s: OK, stop as requested.\n", print_prefix);

done:
    ts_us = get_time_us();
    print_status(input, velo, print_prefix);
    g_stop_flag.store(LOOP_DONE);
}

static void velo_proc(struct Lidar &lidar)
{
    std::unique_ptr<VeloProc> velo;
    char print_pfx[256];

    memset(print_pfx, 0, sizeof(print_pfx));
    snprintf(print_pfx, sizeof(print_pfx) - 1, "%s", lidar.model.c_str());

	printf("%s@%d: device: %s, port: %d, %d, type: %d\n", __func__, __LINE__,
			lidar.model.c_str(), lidar.port, lidar.pos_port, lidar.type);

    switch (lidar.type) {
    case VELO_64E:
        printf("read from live device: %s, port: %d\n",
            lidar.model.c_str(), lidar.port);
        velo.reset(new VeloProc64e(lidar));
        break;
    case VELO_VLP16:  // fall-through intended
    case VELO_32E:
    case VELO_VLS128:
        printf("read from live device: %s, port: %d, %d\n",
            lidar.model.c_str(), lidar.port, lidar.pos_port);
        velo.reset(new VeloProc32eVlp16(lidar));
        break;

    case PANDORA_40LINE:
        printf("read from live device: %s, port: %d, %d\n",
            lidar.model.c_str(), lidar.port, lidar.pos_port);
		velo.reset(new Pandar40Line(lidar));
		break;
    default:
        printf("lidar model (%s) not supported, I give-up.\n",
            lidar.model.c_str());
        return;
    }

    std::unique_ptr<Input> input;

    if (!lidar.socket) {
		VELO_LOG_DEBUG("%s@%d pcap reset\n",__func__, __LINE__);
        switch (lidar.type) {
        case VELO_64E:
            input.reset(new PcapInput64e(lidar.rate,
                lidar.input, lidar.port, 
				FIRING_DATA_PACKET_SIZE, POSITIONING_DATA_PACKET_SIZE, lidar.live));
            break;
        case VELO_VLP16:  // fall-through intended
        case VELO_32E:
        case VELO_VLS128:
            input.reset(new PcapInput32eVlp16(lidar.rate,
                lidar.input, lidar.port, lidar.pos_port, 
				FIRING_DATA_PACKET_SIZE, POSITIONING_DATA_PACKET_SIZE, lidar.live));
			break;
		case PANDORA_40LINE:
            input.reset(new PcapInput32eVlp16(lidar.rate,
                lidar.input, lidar.port, lidar.pos_port, 
				PANDAR_FIRING_DATA_PACKET_SIZE, PANDAR_POS_DATA_PACKET_SIZE, lidar.live));
            break;
        }
    } else {
        switch (lidar.type) {
        case VELO_64E:
            input.reset(new SocketInput64e(lidar.port, g_timeout_ms));
            break;
        case VELO_VLP16:  // fall-through intended
        case VELO_32E:
        case VELO_VLS128:
			VELO_LOG_DEBUG("%s@%d velody reset\n",__func__, __LINE__);
            input.reset(new SocketInput32eVlp16(lidar.port, lidar.pos_port,
                g_timeout_ms, FIRING_DATA_PACKET_SIZE, POSITIONING_DATA_PACKET_SIZE));
			break;
		case PANDORA_40LINE:
			VELO_LOG_DEBUG("%s@%d, pandora 40line inialize\n",__func__, __LINE__);
            input.reset(new SocketInput32eVlp16(lidar.port, lidar.pos_port,
                g_timeout_ms, PANDAR_FIRING_DATA_PACKET_SIZE, PANDAR_POS_DATA_PACKET_SIZE));
            break;
        }
    }

    if (!input->init()) {
        printf("failed to init input\n");
        return;
    }

    velo_loop(input.get(), velo.get(), print_pfx);
}

static void print_usage(const char *name)
{
    printf("Usage: %s [-h -? -v -D -c"
        " -r -t timeout_ms -n total_timeout -s print_intv_secs]\n", name);
    printf("    -h | -?: print help info\n"
        "    -v: print version & build info\n"
        "    -D: print debug information\n"
        "    -c: path of the config file\n"
        "    -r: read from pcap file as fast as possible\n"
        "    -t timeout_ms: timeout in milli-seconds when reading from live device; "
        "default to %d\n"
        "    -n total_timeout: quits if this many of timeout have happened;\n"
        "        default: 0 -- continues to run despite timeout\n"
        "    -s print_intv_secs: prints out status & stats at this interval, in seconds; "
        "default to %d\n"
        "This program runs until a uncoverable fatal error (or too many timeouts), "
        "or end of pcap file; \n    type q (and enter) to quit any time.\n\n",
        DEFAULT_READ_TIMEOUT_MS,
        DEFAULT_STATS_PRINT_INTV_SEC);
}

int main(int argc, char * const *argv)
{
    extern char *optarg;
    unsigned int i;
    int c;
    std::string cfg_file;

	setvbuf(stdout,NULL,_IOLBF,0);

    printf("app version: %s, lib version: %s\n", SENSOR_SYNC_APP_VERSION,
        get_sensor_sync_version());

    if (geteuid()) {
        printf("Please run this tool with root privileges. Exiting now.\n");
        return -1;
    }


    while ((c= getopt(argc, argv, "?hvt:rn:s:d:F:c:D")) != EOF) {
        switch (c) {
        case '?':
        case 'h':
            print_usage(argv[0]);
            return 0;
        case 'v':
            printf("build details: %s\n", SENSOR_SYNC_APP_VERSION);
            return 0;
        case 't':
            g_timeout_ms = atoi(optarg);
            break;
        case 'r':
            g_read_fast = atoi(optarg);
            break;
        case 'n':
            g_timeout_total = atoi(optarg);
            if (g_timeout_total > 0) {
                printf("Will quit after %d timeouts\n", g_timeout_total);
            } else {
                g_timeout_total = 0;
                printf("Will continue running even when there are timeouts\n");
            }
            break;
        case 's':
            g_print_intv = atoi(optarg);
            if (g_print_intv == 0) {
                printf("Invalid stats-print-interval: %lu; set to use default of %d\n",
                    g_print_intv, DEFAULT_STATS_PRINT_INTV_SEC);
                g_print_intv = DEFAULT_STATS_PRINT_INTV_SEC;
            }
            printf("Will print stats every %lu seconds\n", g_print_intv);
            break;
        case 'c':
            cfg_file = optarg;
            std::cout << "Use config file: " << cfg_file << "\n";
            break;
		case 'D':
			adv_plat_set_log(printf);
			break;
        default:
            printf("unsupported argument: %c, I quit\n", c);
            print_usage(argv[0]);
            return -1;
        }
    }

    if (setenv("TZ", "UTC", 1) < 0) {
        printf("Failed to setenv TZ\n");
        return -1;
    }

    SensorConfig cfg(cfg_file);
    if (cfg.lidars.size() == 0) {
        printf("No lidar configurations, please check the conf files\n");
        return 0;
    }
    cfg.dump();

    printf("running, type 'q' (without quote) to quit\n");
    std::vector<std::thread> velo_ths;
    for (i = 0; i < cfg.lidars.size(); i++) {
        std::thread velo_th(velo_proc, std::ref(cfg.lidars[i]));
        velo_ths.push_back(std::move(velo_th));
    }

    for (i = 0; i < velo_ths.size(); i++) {
        velo_ths[i].join();
    }

    return 0;
}
