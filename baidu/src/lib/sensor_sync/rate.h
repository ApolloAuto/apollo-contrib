/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
*
************************************************************************/

/*
*@file   rate.h
*@author feiaiguo@
*@brief A shorter simplified version of ros::Rate, definitions.
*/

#ifndef VELODYNE_DRIVER_RATE_H
#define VELODYNE_DRIVER_RATE_H

#include <stdint.h>

namespace velodyne_driver {

static const int SEC_TO_USECS = 1000000;

/// Returns current time in micro-seconds.
uint64_t get_time_us();

class Rate {
public:

    /**
     * @brief Constructor, creates a Rate
     * @param frequency The desired rate to run at in Hz
     * @param min_sleep_us don't sleep-wait if the leftover time to the next firing is less than
     *      that
     */
    Rate(double frequency, int min_sleep_us);

    /**
     * @brief Sleeps for any leftover time in a cycle.
     *      Calculated from the last time sleep, reset, or the constructor was called.
     * @return True if the desired rate was met for the cycle, false otherwise.
    */
    bool sleep();

    /**
     * @brief Sets the start time for the rate to now
     */
    void reset();

private:
    // Skip sleep if should-sleep interval is less than that, in micro-seconds.
    int _min_sleep_us;
    // Interval between runs in micro-seconds.
    int _run_intv_us;

    // Timestamp of the last firing, in micro-seconds.
    uint64_t _last_run_us;
    // Scheduled times of the next firing, in micro-seconds.
    uint64_t _next_run_us;
};

}  // namespace velodyne_driver

#endif  // VELODYNE_DRIVER_RATE_H
