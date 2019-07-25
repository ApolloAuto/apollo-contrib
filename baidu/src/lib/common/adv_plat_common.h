/************************************************************************
*
* Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
*
************************************************************************/

/**
* @file adv_plat_common.h
* @author feiaiguo@
* @date 2016/07/18
*
* Baidu ADV (Autonomous Driving Vehicle) platform common definitions.
**/

#ifndef ADU_PLAT_SW_LIB_COMMON_ADV_PLAT_COMMON_H
#define ADU_PLAT_SW_LIB_COMMON_ADV_PLAT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

enum adv_platform {
	ADV_PLATFORM_UNKNOWN,
	ADV_PLATFORM_HW2_COMPUTE,
	ADV_PLATFORM_HW2_CONTROL,
	ADV_PLATFORM_HW3_COMPUTE,
	ADV_PLATFORM_HW3_CONTROL,
	ADV_PLATFORM_SENSORBOX,
	ADV_PLATFORM_HW3P1,
	ADV_PLATFORM_NUM
};

/**
 * @brief Type of log function used by various tools to write out log messages.
 *
 * A function of this type has the same signature as C standard printf():
 * takes the same kind of arguments, and same return type.
 * Note this is defined as a pointer type so there is no need to do like "AdvPlatLogFn *".
 */
typedef int (*AdvPlatLogFn)(const char *format, ... );

/** @brief "Dummy" function that can be used to completely disable logging. */
int adv_plat_log_null(const char *format, ... );

/** @brief Sets log function to be used by adv plat library.
 *
 * This has global effect: once set, all subsequent adv plat library function calls will use the
 * given log function.
 *
 * @return old log function.
 *
 * @code {.c}
 * Examples:
 * adv_plat_set_log(adv_plat_log_null);  // Disable all logging -- default behavior.
 * AdvPlatLogFn old_log_fn = adv_plat_set_log(printf);  // Use printf to log to console.
 * @endcode
 */
AdvPlatLogFn adv_plat_set_log(AdvPlatLogFn log_fn);

enum adv_platform adv_plat_id();
const char *adv_plat_name();

#ifdef __cplusplus
}
#endif

#endif  // ADU_PLAT_SW_LIB_COMMON_ADV_PLAT_COMMON_H
