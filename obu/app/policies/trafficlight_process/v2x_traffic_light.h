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

/**
 * @file v2x_traffic_light.h
 * @brief The traffic light program for the v2x module
 */

#ifndef APP_POLICIES_TRAFFICLIGHT_V2X_TRAFFIC_LIGHT_H
#define APP_POLICIES_TRAFFICLIGHT_V2X_TRAFFIC_LIGHT_H

#include <math.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "MapData.h"
#include "SPAT.h"
#include "coordinate_transition/coordinate_transition.h"
#include "glog/logging.h"

#include "modules/v2x/proto/v2x_car_status.pb.h"
#include "modules/v2x/proto/v2x_obu_traffic_light.pb.h"
#include "modules/v2x/proto/v2x_traffic_light.pb.h"
#include "modules/v2x/proto/v2x_traffic_light_policy.pb.h"

// namespace apollo {
namespace v2x {

typedef enum ErrorInfo {
  Successful,
  ReadMapFailed,
  ReadSpatFailed,
  PolicyFailed
} ErrorInfo;

class Trafficlight {
 public:
  Trafficlight();
  ~Trafficlight() {}
  // Function that init
  void Init();
  // interface Function for traffic light app. The return value represents
  // the meaning of : 0 for true ; 1 for decode map error ;
  // 2 for decode spat error; 3 for policy error.
  ErrorInfo TrafficLightApp(
      const MapData_t* map, const SPAT_t* spat,
      const std::shared_ptr<apollo::v2x::CarStatus>& car_status_receive,
      std::shared_ptr<apollo::v2x::obu::ObuTrafficLight>&
          intersection_trafficlight_msg);
  // Function that orientation to the current lane traffic light info
  bool TrafficLightPolicy(const apollo::v2x::PolicyData& policy_data,
                          const apollo::v2x::CarStatus& car_status,
                          std::shared_ptr<apollo::v2x::obu::ObuTrafficLight>&
                              intersection_trafficlights);
  // Function that acquisition system timestamp
  double GetSystemTime() {
    struct timeb t;
    ftime(&t);
    return t.time + t.millitm / 1000.0;
  }
  // Function that get the current year
  int GetCurrentYearMoy() {
    struct tm* local;
    time_t now;
    now = time(NULL);
    local = localtime(&now);
    return (local->tm_year + 1900 - 2018) * 525600 + 25245600;
  }
  // Constant PI
  const double kPi = 3.14159265358979323846;
  // Longitude and Latitude resolution ratio
  const double kResolutionRatio = 10000000.0;
  // The reference value of minimum distance
  const double kReferenceValue = 5000.0;
  double timestamp_ = 0.0;
  int sequence_num_ = 0;

 private:
  // Function that extract Map information
  bool ReadMap(const MapData_t* rsu_map, apollo::v2x::Map* map);
  // Function that extract Map intersection information
  bool ReadMapIntersection(const Node_t* nodes,
                           apollo::v2x::Intersection* intersections);
  // Function that extract Map road information
  bool ReadMapRoad(const Link_t* pInlink_j, apollo::v2x::Road* roads);
  // Function that extract Map lane information
  bool ReadMapLane(const Lane_t* plane, apollo::v2x::Lane* lanes);
  // Function that extract Map connects to information
  bool ReadMapConnectsTo(const Connection_t* connects_to,
                         apollo::v2x::Connection* connections);
  // Function that extract Map position information
  bool ReadMapPosition(const RoadPoint_t* road_point,
                       apollo::v2x::Position2D* points);
  // Function that extract Spat information
  bool ReadSpat(const SPAT_t* rsu_spat, apollo::v2x::Spat* spat);
  // Invocation coordinate ransitions
  v2x::CoordinateTransition coordinate_ransition_;
  double self_position_x_ = 0.0;
  double self_position_y_ = 0.0;
  double intersection_position_x_ = 0.0;
  double intersection_position_y_ = 0.0;
  double distance_among_two_ = 0.0;
  double distance_among_two_min_ = 0.0;
  double longitude_ = 0.0;
  double latitude_ = 0.0;
  double point_x_ = 0.0;
  double point_y_ = 0.0;
  int previous_moy_ = 0;
  int present_moy_ = 0;
  int previous_dsecond_ = 0;
  int present_dsecond_ = 0;
  int current_year_moy_ = 0;
  bool debug_flag_ = FLAGS_debug_flag;
};

}  // namespace v2x
//}  // namespace apollo

#endif  // APP_POLICIES_TRAFFICLIGHT_V2X_TRAFFIC_LIGHT_H
