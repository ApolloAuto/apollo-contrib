#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "adv_plat_common.h"

int adv_plat_log_null(const char* UNUSED(format), ... )
{
    return 0;
}

AdvPlatLogFn adv_plat_log_fn = adv_plat_log_null;

AdvPlatLogFn adv_plat_set_log(AdvPlatLogFn log_fn)
{
    AdvPlatLogFn old_fn = adv_plat_log_fn;
    adv_plat_log_fn = log_fn;
    return old_fn;
}

#define	PATH_LEN			256
#define	SYSFS_PCI_DEVICE_PATH		"/sys/bus/pci/devices"
#define	SYSFS_FILE_VENDOR		"vendor"
#define	SYSFS_FILE_DEVICE		"device"

#define	PCI_VENDOR_ID_BAIDU		0x1D22
#define	PCI_DEVICE_ID_HERCULES		0x2080
#define	PCI_DEVICE_ID_HERMES		0x20A0
#define	PCI_DEVICE_ID_DRAGONFLY_PRI_0	0x2081
#define	PCI_DEVICE_ID_DRAGONFLY_PRI_1	0x20A1
#define	PCI_DEVICE_ID_DRAGONFLY_SEC	0x2082
#define	PCI_DEVICE_ID_MOONROVER		0x2083
#define	PCI_DEVICE_ID_DRAGONFLY_3P1	0x2084

static const char adv_platform_names[ADV_PLATFORM_NUM][64] = {
	"Unknown",
	"HW2 Compute Server",
	"HW2 Control Server",
	"HW3 Compute Server",
	"HW3 Control Server",
	"SensorBox",
	"HW3.1"
};

static uint16_t adv_plat_find_device(char *pcidev)
{
	char path[PATH_LEN];
	FILE *file;
	uint16_t vendor;
	uint16_t device;
	int ret;

	(void) snprintf(path, PATH_LEN, "%s/%s/%s",
	    SYSFS_PCI_DEVICE_PATH, pcidev, SYSFS_FILE_VENDOR);
	file = fopen(path, "r");
	if (!file) {
		adv_plat_log_fn("failed to open file %s! %s\n",
		    path, strerror(errno));
		return 0;
	}
	ret = fscanf(file, "0x%hx", &vendor);
	(void) fclose(file);

	if (ret != 1) {
		adv_plat_log_fn("failed to scan file %s! %s\n",
		    path, strerror(errno));
		return 0;
	}

	if (vendor != PCI_VENDOR_ID_BAIDU) {
		return 0;
	}

	(void) snprintf(path, PATH_LEN, "%s/%s/%s",
	    SYSFS_PCI_DEVICE_PATH, pcidev, SYSFS_FILE_DEVICE);
	file = fopen(path, "r");
	if (!file) {
		adv_plat_log_fn("failed to open file %s! %s\n",
		    path, strerror(errno));
		return 0;
	}
	ret = fscanf(file, "0x%hx", &device);
	(void) fclose(file);

	if (ret != 1) {
		adv_plat_log_fn("failed to scan file %s! %s\n",
		    path, strerror(errno));
		return 0;
	}

	return device;
}

enum adv_platform adv_plat_id()
{
	enum adv_platform plat = ADV_PLATFORM_UNKNOWN;
	char *cwd;
	DIR *dp;
	struct dirent *ent;
	struct stat st;
	uint16_t device;

	dp = opendir(SYSFS_PCI_DEVICE_PATH);
	if (!dp) {
		adv_plat_log_fn("failed to open dir %s! %s\n",
		    SYSFS_PCI_DEVICE_PATH, strerror(errno));
		return ADV_PLATFORM_UNKNOWN;
	}

	cwd = getcwd(NULL, 0);
	if (chdir(SYSFS_PCI_DEVICE_PATH)) {
		adv_plat_log_fn("failed to change dir to %s! %s\n",
		    SYSFS_PCI_DEVICE_PATH, strerror(errno));
		goto end;
	}

	while ((ent = readdir(dp))) {
		(void) lstat(ent->d_name, &st);
		if (S_ISLNK(st.st_mode)) {
			device = adv_plat_find_device(ent->d_name);
			switch (device) {
			case PCI_DEVICE_ID_HERCULES:
				plat = ADV_PLATFORM_HW2_COMPUTE;
				goto end;
			case PCI_DEVICE_ID_HERMES:
				plat = ADV_PLATFORM_HW2_CONTROL;
				goto end;
			case PCI_DEVICE_ID_DRAGONFLY_PRI_0:
			case PCI_DEVICE_ID_DRAGONFLY_SEC:
				plat = ADV_PLATFORM_HW3_COMPUTE;
				goto end;
			case PCI_DEVICE_ID_DRAGONFLY_PRI_1:
				plat = ADV_PLATFORM_HW3_CONTROL;
				goto end;
			case PCI_DEVICE_ID_MOONROVER:
				plat = ADV_PLATFORM_SENSORBOX;
				goto end;
			case PCI_DEVICE_ID_DRAGONFLY_3P1:
				plat = ADV_PLATFORM_HW3P1;
				goto end;
			default:
				break;
			}
		}
	}

end:
	if (cwd) {
		chdir(cwd);
		free(cwd);
	}
	(void) closedir(dp);

	return plat;
}

const char *adv_plat_name()
{
	return adv_platform_names[adv_plat_id()];
}
