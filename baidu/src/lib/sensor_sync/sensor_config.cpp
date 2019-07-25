/************************************************************************
 *
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * *
 ************************************************************************/

/*
 * @file   sensor_config.cpp
 * @author youxiangtao@ (May 2018)
 */

#include <exception>
#include <fstream>
#include <iostream>

#include "sensor_sync.h"

namespace velodyne_driver {

static const std::string DefCfgFile =
    "/home/caros/plat-sw/configs/sensor_sync/sensor_sync.conf";
static const std::string PropModel = "model";
static const std::string PropInput = "input";
static const std::string PropLive = "live";
static const std::string PropSocket = "socket";
static const std::string PropDrift = "drift";
static const std::string PropOffset = "offset";
static const std::string PropPort = "port";
static const std::string PropPosPort = "pos_port";
static const std::string PropCameras = "cameras";
static const std::string PropName = "name";
static const std::string PropAngle = "angle";
static const std::string PropFPS = "fps";

void SensorConfig::query_cam_caps(struct Camera &cam)
{
	zynq_cam_caps_t caps;
	int fd;
	float t_row;

	cam.frame_len_lines = DEFAULT_FRAME_LINES_230;
	cam.line_len_pck = DEFAULT_LINE_PCK_230;
	cam.pixel_clock = DEFAULT_PIXCLK_230;

	fd = open(cam.vdev.c_str(), O_RDWR);
	if (fd < 0) {
		VELO_LOG_DEBUG("Failed to open device %s\n", cam.vdev.c_str());
		goto end;
	}

	memset(&caps, 0, sizeof(caps));

	if (ioctl(fd, ZYNQ_IOC_CAM_CAPS, &caps)) {
		VELO_LOG_DEBUG("Failed to get camera caps of %s\n",
		    cam.vdev.c_str());
		goto end;
	}

	if (caps.major_version == (unsigned short)-1) {
		VELO_LOG_DEBUG("Invalid camera caps of %s\n", cam.vdev.c_str());
		goto end;
	}

	cam.frame_len_lines = caps.frame_len_lines;
	cam.line_len_pck = caps.line_len_pck;
	cam.pixel_clock = caps.pixel_clock;
end:
	t_row = (float)cam.line_len_pck * SEC_TO_USECS / cam.pixel_clock;
	// The readout delay is the delay from trigger to readout of frame
	// middle line. It is a fixed value for slave shutter sync mode.
	cam.readout_delay = (unsigned int)
	    (t_row * (cam.frame_len_lines + (DEFAULT_HEIGHT / 2)));

	if (fd >= 0) {
		close(fd);
	}
}

void SensorConfig::query_cam_trigger(struct Camera &cam)
{
	zynq_trigger_t *t;
	char path[16];
	unsigned int z, i;

	for (z = 0; z < ZYNQ_TRIGGER_DEV_NUM; z++) {
		if (triggers.status[z].zdev_name[0] == '\0') {
			continue;
		}
		for (i = 0; i < ZYNQ_FPD_TRIG_NUM; i++) {
			t = &triggers.status[z].fpd_triggers[i];
			if (t->vnum < 0) {
				continue;
			}
			if (cam.name.compare(t->name) == 0) {
				snprintf(path, sizeof(path),
				    "/dev/video%d", t->vnum);
				cam.vdev = path;
				if (cam.fps == 0) {
					cam.fps = t->fps;
				}
				if (cam.fps > 0) {
					cam.interval = SEC_TO_USECS / cam.fps;
				}
				return;
			}
		}
	}
}

void SensorConfig::get_lidar_params(struct Lidar &lidar)
{
	if ((lidar.model == "64E_S2") ||
	    (lidar.model == "64E_S2.1") ||
	    (lidar.model == "64E_S3S")) {
		// generates 1333312 points per second
		// 1 packet holds 384 points
		lidar.type = VELO_64E;
		lidar.rate = 3472.17;	// 1333312 / 384
	} else if ((lidar.model == "64E_S3D_STRONGEST") ||
	    (lidar.model == "64E_S3D_LAST") ||
	    (lidar.model == "HDL64E_S3D") ||
	    (lidar.model == "64E_S3D_DUAL")) {
		lidar.type = VELO_64E;
		lidar.rate = 5789.0;
	} else if (lidar.model == "64E") {
		lidar.type = VELO_64E;
		lidar.rate = 2600.0;
	} else if (lidar.model == "32E") {
		lidar.type = VELO_32E;
		lidar.rate = 1808.0;
	} else if (lidar.model == "VLP16") {
		lidar.type = VELO_VLP16;
		lidar.rate = 754;	// refer to the manual
	} else if (lidar.model == "VLS128") {
		lidar.type = VELO_VLS128;
		lidar.rate = 6253.9;
	} else if (lidar.model == "PANDORA") {
		lidar.type = PANDORA_40LINE;
		lidar.rate = 1000;
	} else {
		lidar.rate = -1;
	}
}

int SensorConfig::parse_camera(const Json::Value &val,
		struct Camera &cam)
{
	int ret = 0;

	try {
		cam.name = val[PropName].asString();
		if (cam.name.empty()) {
			VELO_LOG_ERR("ERROR! camera 'name' not configured\n");
			return -EINVAL;
		}
		cam.angle = val[PropAngle].asUInt();
		cam.angle %= MAX_ANGLE;
		cam.fps = val[PropFPS].asUInt();

		query_cam_trigger(cam);
		query_cam_caps(cam);

	} catch (std::exception &e) {
		VELO_LOG_ERR("ERROR! failed to parse entry %s: %s\n",
		    val.toStyledString().c_str(), e.what());
		ret = -EINVAL;
	} catch (...) {
		VELO_LOG_ERR("ERROR! failed to parse entry %s\n",
		    val.toStyledString().c_str());
		ret = -EINVAL;
	}

	return ret;
}

int SensorConfig::parse_lidar(const Json::Value &val,
		struct Lidar &lidar)
{
	int ret = 0;

	try {
		lidar.model = val[PropModel].asString();
		if (lidar.model.empty()) {
			VELO_LOG_ERR("ERROR! lidar 'model' not configured\n");
			return -EINVAL;
		}
		if (val[PropLive].isNull()) {
			lidar.live = true;
		} else {
			lidar.live = val[PropLive].asBool();
		}
		if (val[PropSocket].isNull()) {
			lidar.socket = false;
		} else {
			lidar.socket = val[PropSocket].asBool();
		}
		lidar.input = val[PropInput].asString();
		if (!lidar.socket && lidar.input.empty()) {
			VELO_LOG_ERR("ERROR! 'input' not configured for '%s'\n",
			    lidar.model.c_str());
			return -EINVAL;
		}
		if (val[PropDrift].isNull()) {
			lidar.drift = DRIFT_TOLERANCE;
		} else {
			lidar.drift = val[PropDrift].asInt();
		}

		if (val[PropOffset].isNull()) {

			if (lidar.model == "PANDORA") {
				lidar.offset = 5000;
			} else {
				lidar.offset = -10000;
			}
		} else {
			lidar.offset = val[PropOffset].asInt();
		}

		lidar.port = val[PropPort].asUInt();
		if ((lidar.port == 0) || (lidar.port > 65535)) {
			lidar.port = FIRING_DATA_PORT;
		}
		lidar.pos_port = val[PropPosPort].asUInt();
		if ((lidar.pos_port == 0) || (lidar.pos_port > 65535)) {
			lidar.pos_port = POSITIONING_DATA_PORT;
		}
		get_lidar_params(lidar);

		int num = val[PropCameras].size();
		for (int i = 0; i < num; i++) {
			lidar.cameras.push_back(Camera());
			if (parse_camera(val[PropCameras][i],
			    lidar.cameras.back())) {
				lidar.cameras.pop_back();
			}
		}
	} catch (std::exception &e) {
		VELO_LOG_ERR("ERROR! failed to parse entry %s: %s\n",
		    val.toStyledString().c_str(), e.what());
		ret = -EINVAL;
	} catch (...) {
		VELO_LOG_ERR("ERROR! failed to parse entry %s\n",
		    val.toStyledString().c_str());
		ret = -EINVAL;
	}

	return ret;
}

int SensorConfig::parse_config()
{
	std::ifstream input(cfgFile);

	if (!input.is_open()) {
		return -ENOENT;
	}

	Json::Value root;
	int num = 0;
	int ret = 0;

	try {
		input >> root;
		num = root.size();
	} catch (std::exception &e) {
		VELO_LOG_ERR("ERROR! failed to parse config: %s\n", e.what());
		ret = -EINVAL;
	} catch (...) {
		VELO_LOG_ERR("ERROR! failed to parse config\n");
		ret = -EINVAL;
	}

	input.close();

	if (ret) {
		return ret;
	}

	int i;
	for (i = 0; i < num; i++) {
		lidars.push_back(Lidar());
		if (parse_lidar(root[i], lidars.back())) {
			lidars.pop_back();
		}
	}

	return ret;
}

SensorConfig::SensorConfig(std::string &cfgFileName) :
	cfgFile(cfgFileName)
{
	if (adv_trigger_get_status(&triggers)) {
		VELO_LOG_ERR("ERROR! adv_trigger_get_status error\n");
		return;
	}

	if (cfgFile.empty()) {
		cfgFile = DefCfgFile;
	}

	(void) parse_config();
}

void SensorConfig::dump()
{
	unsigned int i, j;

	for (i = 0; i < lidars.size(); i++) {
		std::cout << "Lidar " << i << ":\n";
		std::cout << "  model: " << lidars[i].model << "\n";
		std::cout << "  type: " << lidars[i].type << "\n";
		std::cout << "  rate: " << lidars[i].rate << "\n";
		std::cout << "  drift: " << lidars[i].drift << "\n";
		std::cout << "  offset: " << lidars[i].offset << "\n";
		std::cout << "  live: " << lidars[i].live << "\n";
		std::cout << "  socket: " << lidars[i].socket << "\n";
		std::cout << "  input: " << lidars[i].input << "\n";
		std::cout << "  port: " << lidars[i].port << "\n";
		std::cout << "  pos_port: " << lidars[i].pos_port << "\n";
		std::cout << "  cameras:\n";
		for (j = 0; j < lidars[i].cameras.size(); j++) {
			std::cout << "    name: " <<
			    lidars[i].cameras[j].name << ", ";
			std::cout << "angle: " <<
			    lidars[i].cameras[j].angle << ", ";
			std::cout << "vdev: " <<
			    lidars[i].cameras[j].vdev << ", ";
			std::cout << "fps: " <<
			    lidars[i].cameras[j].fps << ", ";
			std::cout << "interval: " <<
			    lidars[i].cameras[j].interval << ", \n";
			std::cout << "      frame_len_lines: " <<
			    lidars[i].cameras[j].frame_len_lines << ", ";
			std::cout << "line_len_pck: " <<
			    lidars[i].cameras[j].line_len_pck << ", ";
			std::cout << "pixel_clock: " <<
			    lidars[i].cameras[j].pixel_clock << ", ";
			std::cout << "readout_delay: " <<
			    lidars[i].cameras[j].readout_delay << ", \n";
			std::cout << "      exposure_time: " <<
			    lidars[i].cameras[j].last_exposure << ", ";
			std::cout << "offset: " <<
			    lidars[i].cameras[j].last_offset << ", ";
			std::cout << "trigger_delay: " <<
			    lidars[i].cameras[j].last_delay << "\n";
		}
	}
}

}
