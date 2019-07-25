/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
*@file   data_type.h
*@author fengkaiwen01@ (original), feiaiguo@ (re-design/implementation)
*@brief
*/

#ifndef VELODYNE_DRIVER_DATA_TYPE_H
#define VELODYNE_DRIVER_DATA_TYPE_H

#include <memory>
#include <stdint.h>

namespace velodyne_driver {

/**
* Raw Velodyne packet constants and structures.
*/
static const uint16_t RAW_SCAN_SIZE = 3;
static const uint16_t SCANS_PER_BLOCK = 32;
static const uint16_t BLOCK_DATA_SIZE = (SCANS_PER_BLOCK * RAW_SCAN_SIZE);

static const size_t FIRING_DATA_PACKET_SIZE = 1206;
static const size_t POSITIONING_DATA_PACKET_SIZE = 512;
static const uint16_t BLOCKS_PER_PACKET = 12;
static const uint16_t PACKET_STATUS_SIZE = 4;
static const uint16_t SCANS_PER_PACKET = (SCANS_PER_BLOCK * BLOCKS_PER_PACKET);

static const int MICRO_SECS_IN_SEC = 1000000;

/** \brief Raw Velodyne data block.
*
*  Each block contains data from either the upper or lower laser
*  bank.  The device returns three times as many upper bank blocks.
*
*  use stdint.h types, so things work with both 64 and 32-bit machines
*/
struct RawBlock {
    uint16_t laser_block_id;        ///< UPPER_BANK or LOWER_BANK
    uint16_t rotation;      ///< 0-35999, divide by 100 to get degrees
    uint8_t  data[BLOCK_DATA_SIZE];
};

/** \brief Raw Velodyne packet data.
*
*  revolution is described in the device manual as incrementing
*    (mod 65536) for each physical turn of the device.  Our device
*    seems to alternate between two different values every third
*    packet.  One value increases, the other decreases.
*
*  status has either a temperature encoding or the microcode level
*/
union VeloPktData {
    struct {
        RawBlock blocks[BLOCKS_PER_PACKET];
        uint32_t gps_timestamp;
        unsigned char status_type;
        unsigned char status_value;
    } s;
    uint8_t data[1206];
};
typedef std::unique_ptr<VeloPktData> VeloPktDataPtr;

/**
*\brief Raw Velodyne packet.
*/
enum StatusType {
    HOURS = 72,
    MINUTES = 77,
    SECONDS = 83,
    DATE = 68,
    MONTH = 78,
    YEAR = 89,
    GPS_STATUS = 71
};

struct NMEATime {
    int16_t year;
    int16_t mon;
    int16_t day;
    int16_t hour;
    int16_t min;
    int16_t sec;

    /** @brief Initializes time to some invalid value so we can test it. */
    NMEATime() : year(-1), mon(-1), day(-1), hour(-1), min(-1), sec(-1) {}
};
typedef std::unique_ptr<NMEATime> NMEATimePtr;

#define NMEATIME_PRINT_DATA(et) (et)->year, (et)->mon, (et)->day, (et)->hour, (et)->min, (et)->sec

static const uint16_t PANDAR_RAW_SCAN_SIZE = 3;
static const uint16_t PANDAR_SCANS_PER_BLOCK = 40;
static const uint16_t PANDAR_BLOCK_DATA_SIZE = (PANDAR_SCANS_PER_BLOCK * PANDAR_RAW_SCAN_SIZE);
static const uint16_t PANDAR_BLOCKS_PER_PACKET = 10;
static const size_t PANDAR_FIRING_DATA_PACKET_SIZE = 1262;
static const size_t PANDAR_POS_DATA_PACKET_SIZE = 512;
/** \brief Raw Pandar data block.
*
*  use stdint.h types, so things work with both 64 and 32-bit machines
*/
struct PandarRawBlock {
    uint16_t laser_block_id;  /// 0xFFEE
    uint16_t rotation;        ///< 0-35999, divide by 100 to get degrees
    uint8_t  data[PANDAR_BLOCK_DATA_SIZE];
};

/** \brief Raw Pandar packet data.
*
*/
union PandarPktData {
    struct {
        PandarRawBlock blocks[PANDAR_BLOCKS_PER_PACKET];
        uint8_t        reserve0[5];
        uint8_t        hight_temp_flag;
        uint8_t        reserve1[2];
        uint16_t       rotate_speed;
        uint32_t       gps_timestamp;
        uint8_t        return_strengh;
        uint8_t        vendor; ///0x42 or 0x43
        uint8_t        utc[6]; ///Year, month, day, hour, minute, second
    } __attribute__((packed)) s;
    uint8_t data[PANDAR_FIRING_DATA_PACKET_SIZE];
};
typedef std::unique_ptr<PandarPktData> PandarPktDataPtr;

/** \brief Raw Pandar GPS packet data.
*
*/
union PandarGpsPktData {
    struct {
        uint16_t       flag;     ///0xFFEE
        uint8_t        year[2];  ///ASCII
        uint8_t        month[2]; ///ASCII
        uint8_t        day[2];   ///ASCII
        uint8_t        sec[2];  ///ASCII
        uint8_t        min[2];   ///ASCII
        uint8_t        hour[2];   ///ASCII
        uint32_t       usec;
        uint8_t        gprmc[77];
        uint8_t        reserve0[411];
        uint8_t        gps_status;
        uint8_t        pps_status;
        uint8_t        reserve1[4];
    } __attribute__((packed)) s;
    uint8_t data[PANDAR_POS_DATA_PACKET_SIZE];
};
typedef std::unique_ptr<PandarGpsPktData> PandarGpsPktDataPtr;



#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                                         \
        TypeName(const TypeName&);                                                                 \
        TypeName& operator=(const TypeName&)
#endif

}

#endif /* CAR_ROS_DRIVERS_VELODYNE_VELODYNE_DRIVER_INCLUDE_VELODYNE_DRIVER_DATA_TYPE_H */
