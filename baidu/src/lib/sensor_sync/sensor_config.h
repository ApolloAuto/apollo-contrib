/*
 * @file   sensor_config.h
 * @author youxiangtao@
 * @brief  sensor_sync configuration definitions
 */

#ifndef SENSOR_CONFIG_H
#define	SENSOR_CONFIG_H

#include "data_type.h"
#include "json/reader.h"
#include <string>
#include <vector>

#include "adv_trigger.h"

namespace velodyne_driver {

#define	MAX_ANGLE		360
#define	MAX_ANGLE_OFFSET	50	/* 1/100 degree */
// How much time(us) we can tolerate the timestamp drift when
// laser passes a particular angle
#define	DRIFT_TOLERANCE		1000

#define	DEFAULT_HEIGHT		1080

enum LidarType {
	VELO_64E = 1,
	VELO_32E,
	VELO_VLP16,
	VELO_VLS128,
	PANDORA_40LINE
};

struct Camera {
	std::string		name;
	std::string		vdev;
	unsigned int		angle;
	unsigned int		fps;
	unsigned int		interval;
	unsigned short		frame_len_lines;
	unsigned short		line_len_pck;
	unsigned int		pixel_clock;
	unsigned int		readout_delay;
	unsigned int		last_exposure;
	unsigned int		last_offset;
	unsigned int		last_delay;
};

struct Lidar {
	std::string		model;
	enum LidarType	type;
	double			rate;
	int				drift;
	int				offset;
	bool			live;
	bool			socket;
	std::string		input;
	unsigned int	port;
	unsigned int	pos_port;
	std::vector<Camera>	cameras;
};

class SensorConfig {
public:
	std::string		cfgFile;
	std::vector<Lidar>	lidars;

	SensorConfig(std::string &cfgFileName);
	virtual ~SensorConfig() {};

	void dump();

private:
	struct adv_trigger_status	triggers;

	void query_cam_trigger(struct Camera &cam);
	void query_cam_caps(struct Camera &cam);
	void get_lidar_params(struct Lidar &lidar);
	int parse_camera(const Json::Value &val, struct Camera &cam);
	int parse_lidar(const Json::Value &val, struct Lidar &lidar);
	int parse_config();
};

}	/* namespace velodyne_driver */

#endif	/* SENSOR_CONFIG_H */
