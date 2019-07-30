#!/bin/bash

TOOL_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

GPS_TOOL="$TOOL_DIR/baidu_gps_time"
NTP_TOOL="/usr/sbin/ntpdate"
CTRL_NODE="192.168.20.9"
DF_DEVICE_ID="1d22:2081"

EPOCH_TIME=''

function need_root()
{
	if [[ $EUID -ne 0 ]]; then
		echo "This script must be run as root"
		exit 1
	fi
}

function check_gps()
{
	for (( i=0; i <10; i++ )); do
		logger -s "Query GPS/FPGA time"
		output=$($GPS_TOOL -p 2>&1)
		if [ $? -eq 0 ]; then
			break
		fi
		sleep 1
	done

	gps_time=$(echo "$output" | grep "GPS Time")
	if [ -z "$gps_time" ]; then
		fpga_time=$(echo "$output" | grep "FPGA Time")
		if [ -z "$fpga_time" ]; then
			logger -s "GPS/FPGA time not available"
			return
		fi
		EPOCH_TIME=$(echo $fpga_time | cut -d' ' -f3)
		logger -s "$fpga_time"
	else
		EPOCH_TIME=$(echo $gps_time | cut -d' ' -f3)
		logger -s "$gps_time"
	fi
}

function try_ntp()
{
	if [ -f $NTP_TOOL ]; then
		logger -s "Try updating system time from NTP"
		$NTP_TOOL -u ntp3.aliyun.com || $NTP_TOOL -u time5.cloud.tencent.com || $NTP_TOOL -u 192.168.10.5 ||
			$NTP_TOOL -b -u jp.ntp.org.cn  
		if [ $? -eq 0 ]; then
			EPOCH_TIME=$(date +%s.%N)
			logger -s "NTP time: $EPOCH_TIME"
		else
			logger -s "NTP time not available"
		fi
	fi
}


function try_ntp_to_control()
{

	if [ -f $NTP_TOOL ]; then
		logger -s "Try updating system time from control node"
                while true
                do
                $NTP_TOOL -b -u $CTRL_NODE
		if [ $? -eq 0 ]; then
			EPOCH_TIME=$(date +%s.%N)
			logger -s "control time: $EPOCH_TIME"
                        break 
                else
			logger -s "control time not available"
                        sleep 1
                fi
                done

        fi
}

function init_fpga()
{
	logger -s "Initialize FPGA time with system time"
	$GPS_TOOL -i
}

function main()
{

	check_gps

	if [ -n "$EPOCH_TIME" ]; then
		return
	fi

	try_ntp

	if [ -n "$EPOCH_TIME" ]; then
		init_fpga
		return
	fi

	device_no=$(lspci | grep -i $DF_DEVICE_ID | wc -l)
	logger -s "Check if E5: $device_no"

	if [ $device_no -eq 1 ]; then
        	try_ntp_to_control

		if [ -n "$EPOCH_TIME" ]; then
			init_fpga
			return
        	fi
	else
		logger -s "Use BIOS time to init fpga"
		init_fpga
		return
	fi

}

function print_usage()
{
	printf "$0: Initialize FPGA time with NTP when GPS is not available;\n"
}

while getopts "h" opt; do
	case "$opt" in
	h|\?)
		print_usage
		exit 0
		;;
	esac
done

need_root
main
