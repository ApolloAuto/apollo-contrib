#!/bin/bash

WARN_PRE="[\033[31m WARN \033[0m]"
OK_PRE="[ \033[32m OK \033[0m ]"

echo -e "-------------------------------------------------------------"
date

TOOL_PATH=/home/caros/cybertron/plat-release
CAM_NAME="`grep "FPD" $TOOL_PATH/configs/sensor_sync/sensor_sync.conf | awk -F':' '{print $2}' | sed 's/,//g'| sed 's/"//g'`"
echo -e "config camera name is $CAM_NAME"

ARRAY=(${CAM_NAME// / })
length=${#ARRAY[@]}
echo "the cam number in config file is $length"
echo "the param of the script  is $1"

TRI_CAM=$1
echo "the target sync number is $TRI_CAM"

DEVICE_PB_FILE=/home/caros/cybertron/params/device.pb.txt
SENSOR_CONF_FILE=$TOOL_PATH/configs/sensor_sync/sensor_sync.conf

function get_lidar_model()
{
	lidar_model=`cat $DEVICE_PB_FILE | grep "perception_lidar_model" | awk '{print $2}'`
	lidar_model2='"model" : "'${lidar_model}'",'
	model=`cat $SENSOR_CONF_FILE | grep "model" | cut -f 1`
	sed -i "s/$model/$lidar_model2/" $SENSOR_CONF_FILE
}

function before_sync()
{
    SYNC_NAME=()
    NUM=0

    for var in ${ARRAY[@]}
    do

        VIDEO_IND=`$TOOL_PATH/bin/adv_trigger -s | grep $var | sed 's/.*video\(.*\) FPD.*/\1/g'`
        RESULT1=video$VIDEO_IND
        FW_STATUS=`$TOOL_PATH/bin/adv_cam_tool -d /dev/$RESULT1 | grep "embedded_data:\ 1"`
        if [ -z "$FW_STATUS" ];then
            echo -e  "$WARN_PRE:$RESULT1($var) does not support sensor sync function!"
            continue
        else	
            echo -e  "$OK_PRE:$RESULT1($var) is waiting for next step"

            while [ 1 ];
            do
                TRIG_STATUS=`$TOOL_PATH/bin/adv_trigger -s | grep $RESULT1 | grep Enable`
                #echo $TRIG_STATUS
                if [ -z "$TRIG_STATUS" ];then
                    echo -e  "`date` : $WARN_PRE:waiting for $RESULT1 to be triggered!"
                    sleep 5
                else
                    echo -e  "$OK_PRE:$RESULT1 all ready, sensor sync can be started!"
                    NUM=$[NUM+1]
                    SYNC_NAME[($NUM)-1]=$var
                    break
                fi
            done
        fi	
    done

    echo "camera loop check finished!"
    echo "the cams support embedded data are ${SYNC_NAME[@]}"
    echo "the number of cams triggered is $NUM"
    echo "the target trigger cams is $TRI_CAM"

    echo -e "SYNC_NAME NUMBER is ${#SYNC_NAME[@]}"
    if [[ $NUM -eq $TRI_CAM ]] && [[ ${#SYNC_NAME[@]}  -gt 0 ]] ; then
        echo -e "$OK_PRE:prepared for sync"
        $TOOL_PATH/bin/sensor_sync -c $TOOL_PATH/configs/sensor_sync/sensor_sync.conf  >> /var/log/sensor_sync.log 2>&1 &
        echo -e "$OK_PRE:sync started!"

        while [ 1 ]
        do
            for var1 in ${SYNC_NAME[@]}
            do
                VIDEO_IND=`$TOOL_PATH/bin/adv_trigger -s | grep $var1 | sed 's/.*video\(.*\) FPD.*/\1/g'`
                V_NUM=video$VIDEO_IND
                TRIG_STATUS_R=`$TOOL_PATH/bin/adv_trigger -s | grep "$V_NUM" | grep Enable`
                if [ -z "$TRIG_STATUS_R" ];then
                    ps -ef | grep sensor_sync | awk '{print $2}' | xargs kill -9
                    echo -e "$V_NUM distriggered, so kill sensor_sync programe and montor camera state again"
                    before_sync 
                    break
                fi
                echo -e "$V_NUM sync is running..."
            done	
            date
            sleep 5
        done
    else
        echo -e "$NUM != $TRI_CAM, or  ${#SYNC_NAME[@]} <=0, will try again after 10s"
        sleep 10
        before_sync 
    fi

}
echo -e "get lidar model..."
get_lidar_model
echo -e "check and monitor camera status..."
before_sync 
echo -e "exit sync_monitor..."

