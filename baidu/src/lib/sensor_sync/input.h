/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
*@file   input.h
*@author fengkaiwen01@ (original), feiaiguo@ (re-design/implementation)
*        youxiangtao@ (May 2018)
*@brief  Interface (abstract) class of input handling.
*/

#ifndef VELODYNE_DRIVER_INPUT_H
#define VELODYNE_DRIVER_INPUT_H

#include <functional>

#include "data_type.h"
#include "rate.h"

namespace velodyne_driver {

static const size_t ETHERNET_HEADER_SIZE = 42;

class VeloProc;

/** @brief functor that processes positioning data packet */
typedef std::function<void (const u_char*)> PositioningDataProcFn;
/** @brief functor that processes firing data packet */
typedef std::function<void (const VeloPktData *)> FiringDataProcFn;

/** @brief Abstract Velodyne input interface class */
class Input {

public:

    struct Stats {
        int rcv_cnt;
        int sz_mismatch_cnt;
        int timeout_cnt;
        int err_cnt;
        int start_cnt;
        int last_cnt;
        uint64_t start_ts;
        uint64_t last_ts;

        Stats() : rcv_cnt(0), sz_mismatch_cnt(0), timeout_cnt(0), err_cnt(0),
            start_cnt(0), last_cnt(0), start_ts(0), last_ts(0) {}

        void log()
        {
            uint64_t ts = get_time_us();
            uint64_t lapse_us;
            uint64_t total_us;

            if (last_ts == 0) {
                lapse_us = 0;
            } else {
                lapse_us = ts - last_ts;
            }
            if (start_ts == 0) {
                start_cnt = rcv_cnt;
                start_ts = ts;
                total_us = 0;
            } else {
                total_us = ts - start_ts;
            }
            double pkt_rate = lapse_us > 0 ?
                (double)(rcv_cnt - last_cnt) * SEC_TO_USECS / lapse_us : 0;
            double all_rate = total_us > 0 ?
                (double)(rcv_cnt - start_cnt) * SEC_TO_USECS / total_us : 0;

            last_cnt = rcv_cnt;
            last_ts = ts;

            VELO_LOG_INFO("rcv#=%d, sz_mismatch=%d, timeout=%d, err=%d, "
                "pkt_rate=%0.3f/%0.3f (pps)\n",
                rcv_cnt, sz_mismatch_cnt, timeout_cnt, err_cnt,
                pkt_rate, all_rate);
        }
    };

    static const int OK = 0;
    static const int FATAL_ERR = -1;
    static const int NO_INPUT = -2;
    static const int POLL_TIMEOUT = -3;
    static const int ERROR = -4;
    static const int END = -5;

public:

    Input() {}
    virtual ~Input() {};

    /** @brief Reads next Velodyne packet.
     *
     * @param block: whether to wait for receiving data
     *
     * @returns OK if successful;
     *  POLL_TIMEOUT failed to receive pkt within the timeout period, try again;
     *  ERROR other recoverable failure, try again;
     *  FATAL_ERR un-recoverable error (i.e., will fail again if try again).
     *
     */
    virtual int get_firing_data_packet(bool block) = 0;

    /** @brief Input initialization.
     * @returns true if successful, false otherwise.
     *
     * get_firing_data_packet() must NOT be called on an Input object if init()
     * failed.
     */
    virtual bool init() = 0;

    /** @brief Loops through packets until we get GPS time.
     * @param loops # of loops to try (each loop may wait until a poll time-out).
     * @returns OK if we get valid GPS timestamp,
     *  ERROR if loop limit reached or end of input reached,
     *  FATAL_ERR if fatal error occured before we get valid GPS timestamp.
     */
    virtual int loop_until_gpstime(VeloProc *velo_proc, int loops) = 0;

    virtual uint8_t *data_buf() = 0;
    virtual uint8_t *pos_buf() = 0;
    virtual void log_stats() = 0;
};

} // velodyne_driver namespace

#endif // __VELODYNE__inputH
