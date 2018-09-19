/***************************************************************************
Copyright 2018 The Apollo Authors. All Rights Reserved                     /
                                                                            /
Licensed under the Apache License, Version 2.0 (the "License");             /
you may not use this file except in compliance with the License.            /
You may obtain a copy of the License at                                     /
                                                                            /
    http://www.apache.org/licenses/LICENSE-2.0                              /
                                                                            /
Unless required by applicable law or agreed to in writing, software         /
distributed under the License is distributed on an "AS IS" BASIS,           /
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.    /
See the License for the specific language governing permissions and         /
limitations under the License.                                              /
****************************************************************************/

#include <ros/ros.h>

#include <lslidar_decoder/lslidar_decoder.h>

int main(int argc, char** argv) {
    ros::init(argc, argv, "lslidar_decoder_node");
    ros::NodeHandle nh;
    ros::NodeHandle pnh("~");

    apollo::drivers::lslidar_decoder::LslidarDecoderPtr decoder(
                new apollo::drivers::lslidar_decoder::LslidarDecoder(nh, pnh));

    if (!decoder->initialize()) {
        ROS_INFO("Cannot initialize the decoder...");
        return -1;
    }
    ros::spin();
    return 0;
}
