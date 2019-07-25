/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
 * @file   sensor_proc.h
 * @author krishnaelango@ ericyao@ feiaiguo@
 *         youxiangtao@ (May 2018)
 * @brief  Process Velodyne data packets.
 *
 * Modified from original ROS open source code and Baidu code by @fengkaiwen01.
 */

#ifndef VELODYNE_DRIVER_VELO_PROC_H
#define VELODYNE_DRIVER_VELO_PROC_H

namespace velodyne_driver {

struct VeloStatus {
    int gps_status;
    uint64_t base_time_sec;
    unsigned int last_gps_timestamp;
    int invalid_date_val;
};

/** @brief Abstract base class for Velodyne packet process */
class VeloProc {

public:
    /// GPS not connected
    static const int GPS_NOT_CONNECTED = 0;
    /// GPS connected and working (receiving both NMEA time record & PPS)
    static const int GPS_OK = 'A';
    /// Only receiving GPS NMEA time record
    static const int GPS_NMEA_ONLY = 'V';
    /// Only receiving GPS PPS
    static const int GPS_PPS_ONLY = 'P';

    static const int LIDAR_ROT_INTERVAL = 100000; /* 10 Hz */

    VeloProc(struct Lidar &lidar);

    virtual ~VeloProc() {};

    virtual int proc_firing_data(const uint8_t *data, const uint8_t *pos);

    /** @brief Processes one Velodyne positioning data packet,
     *
     * @param bytes: byte array of positioning data.
     *  The size and content depend on lidar type.
     */
    virtual void proc_positioning_data(const uint8_t *bytes) = 0;

    virtual std::string &get_status_dbg(std::string &info) const
    {
        info = "not implemented";
        return info;
    }

    bool gps_ok() const { return _gps_status == GPS_OK; }

    uint64_t get_base_time() { return _base_time_sec; }

    bool got_gpstime() { return _gps_status == GPS_OK && _base_time_sec > 0; }

    std::string &get_gps_status(std::string &status) const;

    /** @brief Gets current GPS & timestamp status.
     * Note: base_time_sec can be off by 1 hour if a firing packet
     * is being processed that advances the hours happen during
     * the invocation of this function.
     */
    inline void get_status(VeloStatus &status) const
    {
        status.gps_status = _gps_status;
        status.base_time_sec = _base_time_sec;
        status.last_gps_timestamp = _last_gps_timestamp;
        status.invalid_date_val = _invalid_date_val;
    }

protected:
    struct Lidar _lidar;
    // Accumulated count of invalid data value received.
    int _invalid_date_val;
    int _gps_status;
    NMEATime _time_current;
    NMEATime _time_previous;
    uint64_t _base_time_sec;
    uint64_t _base_time_new;
    unsigned int _last_gps_timestamp;
    bool _timestamp_rollback;

    void update_base_time();
    virtual void process_timestamp(uint32_t gps_timestamp);
	virtual int get_lidar_offset(const uint8_t *data, struct Camera *cam);
    void sync_trigger(const uint8_t *data, struct Camera *cam);
    int sync_all(const uint8_t *data);
};

class VeloProc64e : public VeloProc {

public:
    VeloProc64e(struct Lidar &lidar);

    virtual ~VeloProc64e() {};

    void proc_positioning_data(const uint8_t *bytes);

    virtual std::string &get_status_dbg(std::string &info) const;

private:

    // Flag to indicate that we should update time when year is received.
    bool _update_on_year;

    // Update time & status when hours is received
    void on_hours_received(void);

    // Update time & status when year is received
    void on_year_received(void);

};

class VeloProc32eVlp16 : public VeloProc {

public:
    VeloProc32eVlp16(struct Lidar &lidar);

    virtual ~VeloProc32eVlp16() {};

    void proc_positioning_data(const uint8_t *bytes);

};


class Pandar40Line : public VeloProc {
public:
	Pandar40Line(struct Lidar &lidar);
	virtual ~Pandar40Line() {};
    int proc_firing_data(const uint8_t *data, const uint8_t *pos);
    void proc_positioning_data(const uint8_t *bytes);
protected:
    void process_timestamp(uint32_t gps_timestamp);
	int get_lidar_offset(const uint8_t *data, struct Camera *cam);
};

} // velodyne_driver namespace

#endif // VELODYNE_DRIVER_VELO_PROC_H
