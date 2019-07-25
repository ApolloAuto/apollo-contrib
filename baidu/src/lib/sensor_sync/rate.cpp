/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
*@file   rate.cpp
*@author feiaiguo@
*@brief  A shorter simplified version of ros::Rate, definitions.
*/

#include "rate.h"

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

namespace velodyne_driver {

uint64_t get_time_us()
{
    struct timeval t_now;
    // TODO check for error return from gettimeofday()
    gettimeofday(&t_now, NULL);
    return t_now.tv_sec * SEC_TO_USECS + t_now.tv_usec;
}

Rate::Rate(double frequency, int min_sleep_us) : _min_sleep_us(min_sleep_us)
{
    double intv = 1.0 / frequency;
    _run_intv_us = ((int)intv * SEC_TO_USECS) + (intv - (int)intv) * SEC_TO_USECS;

    reset();
}

bool Rate::sleep()
{
    uint64_t t_now = get_time_us();
    if (t_now > _next_run_us) {
        _next_run_us += _run_intv_us;
        while (_next_run_us <= t_now) {
            _next_run_us += _run_intv_us;
        }

        _last_run_us = t_now;
        return false;
    }
    if (t_now + _min_sleep_us >= _next_run_us) {
        // Too close to target time point, skip sleep.
        _next_run_us += _run_intv_us;
        _last_run_us = t_now;
        return true;
    }

    // TODO: handle error return of usleep()
    usleep(_next_run_us - t_now);
    _next_run_us += _run_intv_us;
    _last_run_us = get_time_us();
    return true;
}

void Rate::reset()
{
    _last_run_us = get_time_us();
    _next_run_us = _last_run_us + _run_intv_us;
}

}  // namespace velodyne_driver
