/************************************************************************
*
* Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
*
************************************************************************/

/*
 * @file   sensor_sync.h
 * @author youxiangtao@ (May 2018)
 * @brief  header file for sensor_sync support
 */

#ifndef SENSOR_SYNC_H
#define SENSOR_SYNC_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {

#include "adv_plat_common.h"
extern AdvPlatLogFn adv_plat_log_fn;

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

#ifdef ROS
#define VELO_LOG_INFO ROS_INFO
#define VELO_LOG_DEBUG ROS_DEBUG
#define VELO_LOG_WARN ROS_WARN
#define VELO_LOG_ERR  ROS_ERROR
#else
#define VELO_LOG_DEBUG adv_plat_log_fn
#define VELO_LOG_INFO printf
#define VELO_LOG_WARN printf
#define VELO_LOG_ERR  printf
#endif

} // end of extern "C" 
#endif // endof __cplusplus

#include "version.h"
#include "sensor_config.h"
#include "data_type.h"
#include "pcap_input.h"
#include "socket_input.h"
#include "sensor_proc.h"

#endif /* SENSOR_SYNC_H */
