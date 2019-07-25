/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
*@file   version.h
*@author feiaiguo@
*@brief  Velo driver version & build info.
*
*/

//#include "adv_plat_bld_info.h"

#define SENSOR_SYNC_LIB_VERSION "1.1.2.1"

namespace velodyne_driver {

const char *get_sensor_sync_version(void)
{
    return SENSOR_SYNC_LIB_VERSION;
}

}
