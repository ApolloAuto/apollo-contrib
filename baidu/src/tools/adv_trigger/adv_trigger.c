/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "linux/zynq_api.h"
#include "adv_trigger.h"

#define VERSION "3.0.0.1"

static int check_status()
{
	struct adv_trigger_status s;
	zynq_trigger_t *t;
	char video_name[16];
	int i, j;
	int ret;

	if ((ret = adv_trigger_get_status(&s))) {
		return ret;
	}

	printf("\n");
	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; ++i) {
		if (s.status[i].zdev_name[0] == '\0') {
			continue;
		}
		printf("%s:\n", s.status[i].zdev_name);
		printf("   GPS: %s, PPS: %s\n",
		    (s.status[i].flags & FLAG_GPS_VALID) ? "Locked" : "No",
		    (s.status[i].flags & FLAG_PPS_VALID) ? "OK" : "No");
		for (j = 0; j < ZYNQ_FPD_TRIG_NUM; j++) {
			t = &s.status[i].fpd_triggers[j];
			if (t->vnum < 0) {
				continue;
			}
			sprintf(video_name, "video%d ", t->vnum);
			printf("%6d: FPS %2d, %s %s%s%s\n",
			    t->id, t->fps,
			    (t->enabled) ? " Enabled" : "Disabled",
			    (t->internal) ? "internal " : "",
			    video_name, t->name);
		}
	}

	return 0;
}

int main(int argc, char * const argv[])
{
	int opt;
	char *dev_path = NULL;
	int enable_trigger = 1;
	int internal = 0;
	int fps = 0;
	int ret;

	printf("software version: %s, adv_trigger lib version: %s\n",
	    VERSION, adv_trigger_version());

	while ((opt = getopt(argc, argv, "deishf:v")) != -1) {
		switch (opt) {
		case 'd': /* disable trigger */
			enable_trigger = 0;
			break;
		case 'e': /* enable trigger */
			enable_trigger = 1;
			break;
		case 'i':
			internal = 1;
			break;
		case 's':
			return check_status();
		case 'f':
			/* fps value is in decimal */
			fps = atoi(optarg);
			if (fps < 0) {
				printf("adv_trigger: invalid fps %d, "
				    "please run 'adv_trigger -h' for help.\n",
				    fps);
				return -1;
			}
			break;
		case 'h':
			printf("Usage: %s [options] [<video_path>]\n", argv[0]);
			printf("\t[<video_path>]: video device file path. All "
			    "video devices are operated if not specified.\n");
			printf("\t[-d]: disable trigger\n");
			printf("\t[-e]: enable trigger\n");
			printf("\t[-i]: use internal FPGA PPS\n");
			printf("\t[-s]: trigger status\n");
			printf("\t[-f <FPS>]: specify FPS (frame-per-second)\n"
			    "\t      Valid FPS (USB): 0 ~ 30 (default 30)\n"
			    "\t      Valid FPS (FPD-Link): 0 ~ 20 (default 10)\n"
			    "\t      (0 sets FPS to the default value)\n");
			printf("\t[-h]: this message\n");
			return 0;
		case 'v':
                        return 0;
		default:
			printf("adv_trigger: bad parameter\n");
			return -1;
		}
	}

	if (optind < argc) {
		dev_path = argv[optind];
	}

	if (enable_trigger) {
		ret = adv_trigger_enable(dev_path, fps, internal);
	} else {
		ret = adv_trigger_disable(dev_path);
	}

	return ret;
}
