/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
* *
************************************************************************/

/*
*@file   sensor_proc.cpp
*@author krishnaelango@ ericyao@ feiaiguo@
*        youxiangtao@ (May 2018)
*@brief  Process Velodyne data packets and synchronize with cameras
*
* Modified from original ROS open source code and Baidu code by @fengkaiwen01.
*/

#include <ctime>
#include <iostream>
#include <sstream>

#include <memory.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "sensor_sync.h"

using namespace std;

namespace velodyne_driver {

VeloProc::VeloProc(struct Lidar &lidar) : _lidar(lidar)
{
    memset(&_time_current, 0, sizeof(_time_current));
    memset(&_time_previous, 0, sizeof(_time_previous));
    _invalid_date_val = 0;
    _gps_status = GPS_NOT_CONNECTED;
    _base_time_sec = 0;
    _base_time_new = 0;
    _last_gps_timestamp = 0;
    _timestamp_rollback = false;
}

std::string &VeloProc::get_gps_status(std::string &status) const
{
    switch(_gps_status) {
    case GPS_NOT_CONNECTED:
        status = "GPS not connected";
        break;
    case GPS_OK:
        status = "GPS OK";
        break;
    case GPS_NMEA_ONLY:
        status = "Receiving GPS NMEA only";
        break;
    case GPS_PPS_ONLY:
        status = "Receiving GPS PPS only";
        break;
    }
    return status;
}

void VeloProc::update_base_time()
{
    struct tm time;

    memset(&time, 0, sizeof(time));
    time.tm_year = _time_current.year - 1900;
    time.tm_mon = _time_current.mon - 1;
    time.tm_mday = _time_current.day;
    time.tm_hour = _time_current.hour;
    time.tm_min = 0;
    time.tm_sec = 0;

    uint64_t new_time = static_cast<uint64_t>(mktime(&time));

    if ((_time_current.hour != _time_previous.hour) ||
        (_time_current.year != _time_previous.year) ||
        (_time_current.mon != _time_previous.mon) ||
        (_time_current.day != _time_previous.day)) {
        // _base_time_sec should have been advanced via GPS timestamp wrap-around detection.
        // If for any reason it was done wrong, we will correct it here (for Velodyne 64E).
        // Or for 32E/VLP16, if positioning data packet arrives ahead of a firing packet right
        // after an hour wrap-around, this will happen (normal but not often).
        if (_base_time_sec == 0) {
            // Program just started, initialize the base time
            _base_time_sec = new_time;
        } else if (_timestamp_rollback) {
            // Timestamp wrap-around happened.
            // The base time has already been adjusted.
            if (new_time != _base_time_sec) {
                // This is a GPRMC time jump
                VELO_LOG_INFO("time jumped: %lu -> %lu: %d-%02d-%02d %02d:00:00\n",
                    _base_time_sec, new_time, _time_current.year, _time_current.mon,
                    _time_current.day, _time_current.hour);
                _base_time_sec = new_time;
            }
            _timestamp_rollback = false;
        } else {
            // No timestamp wrap-around happened
            if (new_time != (_base_time_sec + 3600)) {
                // This is a GPRMC time jump
                VELO_LOG_INFO("time jumped: %lu -> %lu: %d-%02d-%02d %02d:00:00\n",
                    _base_time_sec, new_time, _time_current.year, _time_current.mon,
                    _time_current.day, _time_current.hour);
                _base_time_sec = new_time;
            } else {
                // Delay the base time update to next process_timestamp
                _base_time_new = new_time;
            }
        }
        _time_previous = _time_current;
    }
}

void VeloProc::process_timestamp(uint32_t gps_timestamp)
{
    if ((gps_timestamp < _last_gps_timestamp) && (_base_time_sec > 0)) {
        // Timestamp wrap-around
        if (_base_time_new > 0) {
            // GPRMC new base time was received
            _base_time_sec = _base_time_new;
            _base_time_new = 0;
        } else {
            // Advance 1 hour, it is for velodyne
            _base_time_sec += 3600;
            _timestamp_rollback = true;
        }
        VELO_LOG_DEBUG("%s(): base_time=%lu; %u -> %u\n", __func__, _base_time_sec,
            _last_gps_timestamp, gps_timestamp);
    }

    _last_gps_timestamp = gps_timestamp;
}


int VeloProc::get_lidar_offset(const uint8_t *data, struct Camera *cam) 
{
    unsigned int lidar_ts;
    int angle_offset;
    unsigned int i;
    const VeloPktData *raw = (const VeloPktData *)data;
    const struct RawBlock *rb;
    // Find the first LiDAR data block matching camera's mounting angle
    for (i = 0; i < BLOCKS_PER_PACKET; i++) {
        rb = &raw->s.blocks[i];

        /*
         * Note that (some blocks of) 2 or even 3 successive packets might have
         * the same rotational angle. Ideally we should catch this and not go
         * through trigger adjustment, but since a packet shows up about every
         * 350us it's not possible for all of them to pass the drift tolerance
         * check below.
         */
        angle_offset = rb->rotation - (cam->angle * 100);
        if (abs(angle_offset) < MAX_ANGLE_OFFSET) {
            break;
        } else if ((36000 - abs(angle_offset)) < MAX_ANGLE_OFFSET) {
            if (angle_offset < 0) {
                angle_offset = angle_offset + 36000;
	    } else {
                angle_offset = angle_offset - 36000;
	    }
            break;
        }
    }

    // This block did not match mounting angle
    if (i == BLOCKS_PER_PACKET)  {
        return -1;
    }

    /*
     * Calculate the lidar's timestamp right at camera's angle
     */
    lidar_ts = _last_gps_timestamp -
        (LIDAR_ROT_INTERVAL * angle_offset / 36000);
    VELO_LOG_DEBUG("%s@%d: @%04u.%06u, "
        "rotation=%d, cam_angle=%d, angle_offset=%d, lidar_ts=%u\n", __func__,__LINE__,
        _last_gps_timestamp / SEC_TO_USECS, _last_gps_timestamp % SEC_TO_USECS,
        rb->rotation, cam->angle, angle_offset,
        lidar_ts);

    // Get LiDAR time offset within LiDAR's rotation cycle
    return lidar_ts % LIDAR_ROT_INTERVAL;
}

// Use rotation and GPS information information in Velodyne packets to
// set Hercules FPGA trigger delay register and UVC video timestamp delay
void VeloProc::sync_trigger(const uint8_t *data, struct Camera *cam)
{
    unsigned int exposure_time = 0;
    unsigned int trigger_delay;
    int cam_offset;

    struct timeval tv;
    (void) gettimeofday(&tv, NULL);


    int lidar_offset = get_lidar_offset(data, cam);
	if (lidar_offset < 0) return;

    // Get camera time offset within LiDAR's roation cycle.
    // For camera in shutter sync slave mode, exposure time affects
    // image formation time negatively, thus the current exposure has -sign
    //
    // Get current trigger_delay and exposure time
    adv_trigger_delay_ctl(cam->vdev.c_str(), 0, &trigger_delay, &exposure_time);

    if (exposure_time == 0) {
        return;
    }

    cam_offset = (cam->last_offset + ((cam->last_exposure/2 - exposure_time/2)) +
        LIDAR_ROT_INTERVAL) % LIDAR_ROT_INTERVAL;

    VELO_LOG_DEBUG("%s@%d: host_time %ld.%06lds:"
        "trigger_fix_bias = %d, drift_thr = %d, cam_last_offset = %d, cam_last_exposure = %d, readout_delay = %d, "
        "last_trigger_delay = %d, exposure_time = %d, cam_offset = %d, lidar_offset = %d\n",
        __func__, __LINE__, tv.tv_sec, tv.tv_usec,
        _lidar.offset, _lidar.drift, cam->last_offset, cam->last_exposure, cam->readout_delay, 
        cam->last_delay, exposure_time, cam_offset, lidar_offset);

    // Check if LiDAR and camera time offset is within the defined tolerance.
    // The second condition is to deal with drift across LiDAR cycle boundary
    // to avoid unnecessary programming.
    if ((cam->last_exposure != 0) &&
        (abs(lidar_offset - cam_offset) < _lidar.drift ||
         LIDAR_ROT_INTERVAL - abs(lidar_offset - cam_offset) < _lidar.drift)) {
        return;
    }

	cam_offset = lidar_offset;

    // The readout delay is the delay from trigger to frame middle line readout.
    // The image formation is considered to be the half exposure time of the
    // frame middle line
    trigger_delay = (_lidar.offset + cam_offset - cam->readout_delay + exposure_time/2 +
        LIDAR_ROT_INTERVAL) % cam->interval;

    VELO_LOG_INFO("%s@%d: host_time %ld.%06lds:  %s, exposure_time=%u, "
        "lidar_offset = %d, trigger_delay=%u, last_trigger_delay=%u\n",
        __func__, __LINE__, tv.tv_sec, tv.tv_usec,
        cam->vdev.c_str(), exposure_time, lidar_offset, trigger_delay, cam->last_delay);

    // Set camera trigger delay
    adv_trigger_delay_ctl(cam->vdev.c_str(), 1, &trigger_delay, &exposure_time);

    cam->last_offset = cam_offset;
    cam->last_exposure = exposure_time;
    cam->last_delay = trigger_delay;
}

int VeloProc::sync_all(const uint8_t *data)
{
    unsigned int i;

    for (i = 0; i < _lidar.cameras.size(); i++) {
        struct Camera* cam = &_lidar.cameras[i];
        if (cam->fps > 0) {
            sync_trigger(data, cam);
        }
    }

    return 0;
}

int VeloProc::proc_firing_data(const uint8_t *data, const uint8_t *pos)
{
    const VeloPktData *raw = (const VeloPktData *)data;

    process_timestamp(raw->s.gps_timestamp);

    proc_positioning_data(pos);

    return sync_all(data);
}

VeloProc64e::VeloProc64e(struct Lidar &lidar) :
    VeloProc(lidar), _update_on_year(false) {}

void VeloProc64e::on_hours_received()
{
    //std::cout << "hour: " << _time_current.hour << "." << _time_current.min <<
    //"." << _time_current.sec << std::endl;
    if (_time_current.hour < _time_previous.hour) {
        if (_time_previous.hour == _time_current.hour + 1) {
            // Likely to be out-of-order packet, we reject this.
	    //
            // We invalidate previous hour (and thus previous base time), so
	    // base time will be updated next time hour is received -- this
	    // enables us to recover if we don't get time update for 23 hours.
	    //
            // This is not 100% bullet-proof: we will get the wrong time for
	    // one cycle if we got 2 out-of-order packets that were right
	    // before hour was advanced; however, the probability of that is
	    // much smaller and the damage is limited to one cycle only.
            _time_previous.hour = -1;
            return;
        }
        if (!_update_on_year) {
            // Hour wraps around, which will invalidate day, and possibly month
	    // and year. We shall wait until we get updated values of those to
	    // update time. Set _update_on_year to true so time can be updated
	    // as soon as possible.
	    //
	    // The order of time info arriving: hour, min, sec, day, mon, year.
            _update_on_year = true;
            _time_current.year = -1;
            _time_current.mon = -1;
            _time_current.day = -1;
            return;
        }
    }
    // Update base time if anything changed --
    // so we will catch it if time jumps for any reason.
    if (_time_current.hour != _time_previous.hour
            || _time_current.year != _time_previous.year
            || _time_current.mon != _time_previous.mon
            || _time_current.day != _time_previous.day) {
        if (_time_current.year > 0 &&
            _time_current.mon > 0 &&
            _time_current.day > 0) {
            // Make sure we got all key elements of time.
            update_base_time();
        }
    }
}

void VeloProc64e::on_year_received()
{
    if (_update_on_year) {
        if (_time_current.mon > 0 && _time_current.day > 0) {
            update_base_time();
        }
        // Clear flag so year (and mon, day) won't be invalidated
	// when hour is received next time.
        _update_on_year = false;
    }
}

void VeloProc64e::proc_positioning_data(const uint8_t *bytes)
{
    const VeloPktData *raw = (const VeloPktData *)bytes;
    StatusType status_type = StatusType(raw->s.status_type);
    unsigned char status_value = raw->s.status_value;

    switch (status_type) {
    case YEAR:
        _time_current.year = status_value + 2000;
        on_year_received();
        break;
    case MONTH:
        // month: 1-12
        if (status_value <= 12) {
            _time_current.mon = status_value;
        } else {
            ++_invalid_date_val;
            VELO_LOG_DEBUG("%s: invalid month value of %d\n",
                __func__, status_value);
        }
        break;
    case DATE:
        if (status_value <= 31) {
            _time_current.day = status_value;
        } else {
            ++_invalid_date_val;
            VELO_LOG_DEBUG("%s: invalid day value of %d\n",
                __func__, status_value);
        }
        break;
    case HOURS:
        if (status_value <= 23) {
            _time_current.hour = status_value;
            on_hours_received();
        } else {
            ++_invalid_date_val;
            VELO_LOG_DEBUG("%s: invalid hour value of %d\n",
                __func__, status_value);
        }
        break;
    case MINUTES:
        if (status_value <= 59) {
            _time_current.min = status_value;
        } else {
            ++_invalid_date_val;
            VELO_LOG_DEBUG("%s: invalid minute value of %d\n",
                __func__, status_value);
        }
        break;
    case SECONDS:
        if (status_value <= 59) {
            _time_current.sec = status_value;
        } else {
            ++_invalid_date_val;
            VELO_LOG_DEBUG("%s: invalid second value of %d\n",
                __func__, status_value);
        }
        break;
    case GPS_STATUS:
        _gps_status = status_value;
        break;
    default:
        break;
    }
}

std::string &VeloProc64e::get_status_dbg(std::string &info) const
{
    uint64_t base_time_sec = _base_time_sec;
    unsigned int last_gps_timestamp = _last_gps_timestamp;

    int seconds = last_gps_timestamp / 1000000;
    struct timeval tv = {
        (time_t)base_time_sec + seconds,
        last_gps_timestamp - seconds * 1000000
    };
    struct tm *ti = localtime(&tv.tv_sec);

    std::stringstream ss;
    string gps_status;
    ss << "Velodyne-64E: " << get_gps_status(gps_status)
        << ", time="
        << _time_current.year << "/"
        << _time_current.mon << "/"
        << _time_current.day << ":"
        << _time_current.hour << ":"
        << _time_current.min << ":"
        << _time_current.sec
        << ", base_time (sec)=" << base_time_sec
        << ", last ts=" << last_gps_timestamp
        << ", last pkt @"
        << ti->tm_hour << ":"
        << ti->tm_min << ":"
        << ti->tm_sec << "."
        << tv.tv_usec;

    info = ss.str();
    return info;
}

VeloProc32eVlp16::VeloProc32eVlp16(struct Lidar &lidar) :
    VeloProc(lidar) {}

void VeloProc32eVlp16::proc_positioning_data(const uint8_t *bytes)
{
    int gprmc_index = 206;
    int gprmc_end = 278;
    int field_count = 0;
    int time_field_index = 0;
    int validity_field_index = 0;
    int date_field_index = 0;

    while ((gprmc_index < gprmc_end) && (bytes[gprmc_index++] != '*')) {
        if (bytes[gprmc_index] == ',') {
            ++field_count;
            if (field_count == 1 && time_field_index == 0) {
                time_field_index = gprmc_index + 1;
            } else if (field_count == 2 && validity_field_index == 0) {
                validity_field_index = gprmc_index + 1;
                if (bytes[validity_field_index] == 'V') {
                    VELO_LOG_DEBUG("%s: GPS info is invalid!\n", __func__);
                    _gps_status = GPS_NOT_CONNECTED;
                    return;
                }
            } else if (field_count == 9 && date_field_index == 0) {
                date_field_index = gprmc_index + 1;
                break;
            }
        }
    }

    if (gprmc_index == gprmc_end) {
        VELO_LOG_DEBUG("%s: No GPS data!\n", __func__);
        _gps_status = GPS_NOT_CONNECTED;
        return;
    }

    NMEATime nmea_time;
    nmea_time.year = (bytes[date_field_index + 4] - '0') * 10 +
        (bytes[date_field_index + 5] - '0') + 2000;
    nmea_time.mon  = (bytes[date_field_index + 2] - '0') * 10 +
        (bytes[date_field_index + 3] - '0');
    nmea_time.day  = (bytes[date_field_index] - '0') * 10 +
        (bytes[date_field_index + 1] - '0');
    nmea_time.hour = (bytes[time_field_index] - '0') * 10 +
        (bytes[time_field_index + 1] - '0');
    nmea_time.min  = (bytes[time_field_index + 2] - '0') * 10 +
        (bytes[time_field_index + 3] - '0');
    nmea_time.sec  = (bytes[time_field_index + 4] - '0') * 10 +
        (bytes[time_field_index + 5] - '0');

    if (nmea_time.mon > 12 || nmea_time.mon < 1 ||
        nmea_time.day > 31 || nmea_time.day < 1 ||
        nmea_time.hour > 23 || nmea_time.hour < 0 ||
        nmea_time.min > 59 || nmea_time.min < 0 ||
        nmea_time.sec > 59 || nmea_time.sec < 0) {
        ++_invalid_date_val;
        VELO_LOG_ERR(
            "%s: invalid GPS time: %d-%02d-%02d %02d:%02d:%02d, ignored\n",
            __func__, nmea_time.year, nmea_time.mon, nmea_time.day,
            nmea_time.hour, nmea_time.min, nmea_time.sec);
    } else {
        _time_current = nmea_time;
        _gps_status = GPS_OK;
        update_base_time();
    }
}

Pandar40Line::Pandar40Line(struct Lidar &lidar) : VeloProc(lidar)
{}

int Pandar40Line::proc_firing_data(const uint8_t *data, const uint8_t *pos)
{
    const PandarPktData *raw = (const PandarPktData *)data;

    process_timestamp(raw->s.gps_timestamp);

    proc_positioning_data(pos);

    return sync_all(data);
}

void Pandar40Line::process_timestamp(uint32_t gps_timestamp)
{
    if ((gps_timestamp < _last_gps_timestamp) && (_base_time_sec > 0)) {
        // Timestamp wrap-around
        if (_base_time_new > 0) {
            // GPRMC new base time was received
            _base_time_sec = _base_time_new;
            _base_time_new = 0;
        } else {
            // Advance 1 s, it is for velodyne
            _base_time_sec += 1;
            _timestamp_rollback = true;
        }
    }

    _last_gps_timestamp = gps_timestamp;

}

int Pandar40Line::get_lidar_offset(const uint8_t *data, struct Camera *cam) 
{
    unsigned int lidar_ts;
    int angle_offset;
    unsigned int i;
    const PandarPktData *raw = (const PandarPktData *)data;
    const struct PandarRawBlock *rb;
    // Find the first LiDAR data block matching camera's mounting angle
    for (i = 0; i < PANDAR_BLOCKS_PER_PACKET; i++) {
        rb = &raw->s.blocks[i];

        /*
         * Note that (some blocks of) 2 or even 3 successive packets might have
         * the same rotational angle. Ideally we should catch this and not go
         * through trigger adjustment, but since a packet shows up about every
         * 350us it's not possible for all of them to pass the drift tolerance
         * check below.
         */
        angle_offset = rb->rotation - (cam->angle * 100);
        if (abs(angle_offset) < MAX_ANGLE_OFFSET) {
            break;
        } else if ((36000 - abs(angle_offset)) < MAX_ANGLE_OFFSET) {
            if (angle_offset < 0) {
                angle_offset = angle_offset + 36000;
	    } else {
                angle_offset = angle_offset - 36000;
	    }
            break;
        }
    }

    // This block did not match mounting angle
    if (i == PANDAR_BLOCKS_PER_PACKET)  {
        return -1;
    }

    /*
     * Calculate the lidar's timestamp right at camera's angle
     */
    lidar_ts = _last_gps_timestamp -
        (LIDAR_ROT_INTERVAL * angle_offset / 36000);
    VELO_LOG_DEBUG("%s: @%04u.%06u, "
        "rotation=%d, cam_angle=%d, angle_offset=%d, lidar_ts=%u\n", __func__,
        _last_gps_timestamp / SEC_TO_USECS, _last_gps_timestamp % SEC_TO_USECS,
        rb->rotation, cam->angle, angle_offset,
        lidar_ts);

    // Get LiDAR time offset within LiDAR's rotation cycle
	return lidar_ts % LIDAR_ROT_INTERVAL;
}

void Pandar40Line::proc_positioning_data(const uint8_t *bytes)
{
    const union PandarGpsPktData* raw = (const union PandarGpsPktData*)bytes;

    if (ntohs(raw->s.flag) != 0xFFEE) {
        VELO_LOG_DEBUG("%s@%d: rx gps flag is 0x%04x, expected 0xFFEE, just wait\n",
        __func__, __LINE__, ntohs(raw->s.flag));
    }
    if (raw->s.gps_status != 'A') {
        VELO_LOG_DEBUG("%s@%d: GPS Unlock\n",__func__, __LINE__);
    }

    if (!raw->s.pps_status) {
        VELO_LOG_DEBUG("%s@%d: PPS Unlock\n",__func__, __LINE__);
	}
    struct tm time;
    memset(&time, 0, sizeof(time));
    time.tm_year = (raw->s.year[1] - '0') * 10 + (raw->s.year[0] - '0');
    time.tm_mon = (raw->s.month[1] - '0') * 10 + (raw->s.month[0] - '0');
    time.tm_mday = (raw->s.day[1] - '0') * 10 + (raw->s.day[0] - '0');
    time.tm_hour = (raw->s.hour[1] - '0') * 10 + (raw->s.hour[0] - '0');
    time.tm_min = (raw->s.min[1] - '0') * 10 + (raw->s.min[0] - '0');
    time.tm_sec = (raw->s.sec[1] - '0') * 10 + (raw->s.sec[0] - '0');

	_time_current.year = time.tm_year + 1900;

	if (time.tm_mon <= 12)
		_time_current.mon = time.tm_mon + 1;
	else
		VELO_LOG_ERR("%s: invalid month value of %d\n", __func__, time.tm_mon);

	if (time.tm_mday <= 31)
		_time_current.day = time.tm_mday;
	else
		VELO_LOG_ERR("%s: invalid day value of %d\n", __func__, time.tm_mday);
	if (time.tm_hour <= 23)
		_time_current.hour = time.tm_hour;
	else
		VELO_LOG_ERR("%s: invalid hour value of %d\n", __func__, time.tm_hour);
	if (time.tm_min <= 59)
		_time_current.min = time.tm_min;
	else
		VELO_LOG_ERR("%s: invalid minutes value of %d\n", __func__, time.tm_min);
	if (time.tm_sec <= 59)
		_time_current.sec = time.tm_sec;
	else
		VELO_LOG_ERR("%s: invalid second value of %d\n", __func__, time.tm_sec);

    if (_time_current.hour != _time_previous.hour
            || _time_current.year != _time_previous.year
            || _time_current.mon != _time_previous.mon
            || _time_current.day != _time_previous.day
            || _time_current.hour != _time_previous.hour
            || _time_current.min != _time_previous.min
            || _time_current.sec != _time_previous.sec){

		VELO_LOG_DEBUG("%s@%d: cur/pre y:%hd/%hd,m:%hd/%hd,d:%hd/%dh,h:%hd/%hd,m:%hd:%hd,s:%hd/%hd \n",
				__func__, __LINE__,
				_time_current.year, _time_previous.year,
				_time_current.mon, _time_previous.mon,
				_time_current.day, _time_previous.day,
				_time_current.hour, _time_previous.hour,
				_time_current.min, _time_previous.min,
				_time_current.sec, _time_previous.sec
				);
//		update_base_time();
        _time_previous = _time_current;
	}

    _gps_status = GPS_OK;
}


}
