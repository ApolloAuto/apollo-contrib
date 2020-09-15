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
 * @file v2x_traffic_light.cc
 * @brief The traffic light program for the v2x module
 */

#include "v2x_traffic_light.h"

// namespace apollo {
namespace v2x {

Trafficlight::Trafficlight() {}
void Trafficlight::Init() { current_year_moy_ = GetCurrentYearMoy(); }

ErrorInfo Trafficlight::TrafficLightApp(
    const MapData_t* map, const SPAT_t* spat,
    const std::shared_ptr<apollo::v2x::CarStatus>& car_status_receive,
    std::shared_ptr<apollo::v2x::obu::ObuTrafficLight>&
        intersection_trafficlight_msg) {
  apollo::v2x::PolicyData policy_data_pb;
  apollo::v2x::Map* map_pb = policy_data_pb.mutable_map();
  apollo::v2x::Spat* spat_pb = policy_data_pb.mutable_spat();
  bool read_map_flag = ReadMap(map, map_pb);
  if (!read_map_flag) {
    return ReadMapFailed;
  }
  bool read_spat_flag = ReadSpat(spat, spat_pb);
  if (!read_spat_flag) {
    return ReadSpatFailed;
  }
  if (debug_flag_) {
    LOG(INFO) << policy_data_pb.DebugString();
  }
  bool traffic_light_policy_flag = TrafficLightPolicy(
      policy_data_pb, *car_status_receive, intersection_trafficlight_msg);
  if (!traffic_light_policy_flag) {
    return PolicyFailed;
  }
  return Successful;
}

bool Trafficlight::TrafficLightPolicy(
    const apollo::v2x::PolicyData& policy_data,
    const apollo::v2x::CarStatus& car_status,
    std::shared_ptr<apollo::v2x::obu::ObuTrafficLight>&
        intersection_trafficlights) {
  self_position_x_ = car_status.localization().pose().position().x();
  self_position_y_ = car_status.localization().pose().position().y();
  double linear_velocity_x =
      car_status.localization().pose().linear_velocity().x();
  double linear_velocity_y =
      car_status.localization().pose().linear_velocity().y();
  if (fabs(linear_velocity_x) < 0.01) {
    linear_velocity_x = 0.0;
  }
  if (fabs(linear_velocity_y) < 0.01) {
    linear_velocity_y = 0.0;
  }
  if (!self_position_x_ || !self_position_y_) {
    LOG(ERROR) << "ERROR: Failed to get carstatus position";
    return false;
  }
  const apollo::v2x::Map& map_msg = policy_data.map();
  const apollo::v2x::Spat& spat_msg = policy_data.spat();
  // Orientation to the current lane
  distance_among_two_min_ = kReferenceValue;
  int intersection_index = 0;
  for (int i = 0; i < map_msg.intersections_size(); i++) {
    const apollo::v2x::Intersection& intersection_msg =
        map_msg.intersections(i);
    intersection_position_x_ = intersection_msg.position().x();
    intersection_position_y_ = intersection_msg.position().y();
    if ((intersection_position_x_ - self_position_x_) * linear_velocity_x >=
            0.0 &&
        (intersection_position_y_ - self_position_y_) * linear_velocity_y >=
            0.0) {
      distance_among_two_ =
          sqrt(pow(intersection_position_x_ - self_position_x_, 2) +
               pow(intersection_position_y_ - self_position_y_, 2));
      if (distance_among_two_ < distance_among_two_min_) {
        distance_among_two_min_ = distance_among_two_;
        intersection_index = i;
      }
    }
  }
  const apollo::v2x::Intersection& intersection_current =
      map_msg.intersections(intersection_index);
  distance_among_two_min_ = kReferenceValue;
  int road_index = 0;
  int lane_index = 0;
  if (intersection_current.roads_size() == 0) {
    LOG(ERROR) << "ERROR: map info deficiency, no road info";
    return false;
  }
  for (int j = 0; j < intersection_current.roads_size(); j++) {
    const apollo::v2x::Road& road_msg = intersection_current.roads(j);
    if (road_msg.lanes_size() == 0) {
      LOG(ERROR) << "ERROR: map info deficiency, no lane info";
      return false;
    }
    for (int k = 0; k < road_msg.lanes_size(); k++) {
      const apollo::v2x::Lane& lane_msg = road_msg.lanes(k);
      if (lane_msg.position_offset_size() == 0) {
        LOG(ERROR)
            << "ERROR: map info deficiency, no lane position_offset info";
        return false;
      }
      const apollo::v2x::Position2D& lane_stop_position =
          lane_msg.position_offset(lane_msg.position_offset_size() - 1);
      distance_among_two_ =
          sqrt(pow(lane_stop_position.x() - self_position_x_, 2) +
               pow(lane_stop_position.y() - self_position_y_, 2));
      if (distance_among_two_ < distance_among_two_min_) {
        distance_among_two_min_ = distance_among_two_;
        road_index = j;
        lane_index = k;
      }
    }
  }
  const apollo::v2x::Lane& lane_current =
      intersection_current.roads(road_index).lanes(lane_index);
  // Matching phase information of the current lane
  for (int m = 0; m < spat_msg.intersections_size(); m++) {
    const apollo::v2x::IntersectionState& intersection_state =
        spat_msg.intersections(m);
    if (intersection_state.intersection_id() == intersection_current.id()) {
      if (debug_flag_) {
        std::cout << "The match of intersection is successful " << std::endl;
      }
      // To determine whether a timestamp is lag
      if (intersection_state.has_moy() &&
          intersection_state.has_time_stamp_dsecond()) {
        present_moy_ = intersection_state.moy();
        present_dsecond_ = intersection_state.time_stamp_dsecond();
        timestamp_ = (present_moy_ + current_year_moy_) * 60 +
                     static_cast<double>(present_dsecond_ / 1000.0);
        if (!(present_moy_ >= previous_moy_ &&
              present_dsecond_ >= previous_dsecond_)) {
          previous_moy_ = present_moy_;
          previous_dsecond_ = present_dsecond_;
          LOG(ERROR) << "Message delay, Timestamp smaller";
          return false;
        }
      } else {
        timestamp_ = GetSystemTime();
      }

      // Add Header
      apollo::common::Header* msg_header =
          intersection_trafficlights->mutable_header();
      msg_header->set_timestamp_sec(timestamp_);
      msg_header->set_sequence_num(sequence_num_);
      msg_header->set_module_name("v2x");
      // Add current_lane_trafficlight information
      auto* road_tl1 = intersection_trafficlights->add_road_traffic_light();
      for (int n = 0; n < intersection_state.phases_size(); n++) {
        const ::apollo::v2x::Phase& intersection_phase =
            intersection_state.phases(n);
        for (int p = 0; p < lane_current.connections_size(); p++) {
          const ::apollo::v2x::Connection& connections_data =
              lane_current.connections(p);
          // bool matching_flag = false;
          // switch (connections_data.allow_driving_behavior()) {
          // case ::apollo::v2x::Connection::STRAIGHT:
          //     if (car_status.chassis_detail().light().turn_light_type() ==
          //         apollo::canbus::Light::TURN_LIGHT_OFF) {
          //         matching_flag = true;
          //     };
          //     break;
          // case ::apollo::v2x::Connection::LEFT:
          //     if (car_status.chassis_detail().light().turn_light_type() ==
          //         apollo::canbus::Light::TURN_LEFT_ON) {
          //         matching_flag = true;
          //     };
          //     break;
          // case ::apollo::v2x::Connection::RIGHT:
          //     if (car_status.chassis_detail().light().turn_light_type() ==
          //         apollo::canbus::Light::TURN_RIGHT_ON) {
          //         matching_flag = true;
          //     };
          //     break;
          // case ::apollo::v2x::Connection::U_TURN:
          //     if (car_status.chassis_detail().light().turn_light_type() ==
          //         apollo::canbus::Light::TURN_LEFT_ON) {
          //         matching_flag = true;
          //     };
          //     break;
          // default:
          //     break;
          // }
          if (intersection_phase.id() == connections_data.phase_id()) {
            if (debug_flag_) {
              std::cout << "The match of current lane phase is successful "
                        << std::endl;
            }
            // Add single_trafficlight information
            auto* lane_tl1 = road_tl1->add_lane_traffic_light();
            lane_tl1->set_gps_x_m(
                lane_current
                    .position_offset(lane_current.position_offset_size() - 1)
                    .x());
            lane_tl1->set_gps_y_m(
                lane_current
                    .position_offset(lane_current.position_offset_size() - 1)
                    .y());
            auto* single_trafficlight = lane_tl1->add_single_traffic_light();
            // switch (connections_data.allow_driving_behavior()) {
            // case ::apollo::v2x::Connection::STRAIGHT:
            //     single_trafficlight->add_trafficlight_type(
            //         apollo::v2x::SingleTrafficLight::STRAIGHT);
            //     break;
            // case ::apollo::v2x::Connection::LEFT:
            //     single_trafficlight->add_trafficlight_type(
            //         apollo::v2x::SingleTrafficLight::LEFT);
            //     break;
            // case ::apollo::v2x::Connection::RIGHT:
            //     single_trafficlight->add_trafficlight_type(
            //         apollo::v2x::SingleTrafficLight::RIGHT);
            //     break;
            // case ::apollo::v2x::Connection::U_TURN:
            //     single_trafficlight->add_trafficlight_type(
            //         apollo::v2x::SingleTrafficLight::U_TURN);
            //     break;
            // default:
            //     break;
            // }
            switch (intersection_phase.color()) {
              case ::apollo::v2x::Phase::UNKNOWN:
                single_trafficlight->set_color(
                    apollo::v2x::SingleTrafficLight::UNKNOWN);
                break;
              case ::apollo::v2x::Phase::RED:
                single_trafficlight->set_color(
                    apollo::v2x::SingleTrafficLight::RED);
                break;
              case ::apollo::v2x::Phase::YELLOW:
                single_trafficlight->set_color(
                    apollo::v2x::SingleTrafficLight::YELLOW);
                break;
              case ::apollo::v2x::Phase::GREEN:
                single_trafficlight->set_color(
                    apollo::v2x::SingleTrafficLight::GREEN);
                break;
              case ::apollo::v2x::Phase::BLACK:
                single_trafficlight->set_color(
                    apollo::v2x::SingleTrafficLight::BLACK);
                break;
              case ::apollo::v2x::Phase::FLASH_GREEN:
                single_trafficlight->set_color(
                    apollo::v2x::SingleTrafficLight::FLASH_GREEN);
                break;
              default:
                break;
            }
            // single_trafficlight->set_color_remaining_time_s(
            //     intersection_phase.color_remaining_time_s());
          }
        }
      }
      sequence_num_++;
      return true;
    }
  }
  return false;
}

bool Trafficlight::ReadMapPosition(const RoadPoint_t* road_point,
                                   apollo::v2x::Position2D* points) {
  if (road_point == nullptr) {
    LOG(ERROR) << "Error: Map node info is empty!";
    return false;
  }
  switch (road_point->posOffset.offsetLL.present) {
    case 1:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL1.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL1.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    case 2:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL2.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL2.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    case 3:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL3.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL3.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    case 4:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL4.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL4.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    case 5:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL5.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL5.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    case 6:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL6.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LL6.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    case 7:
      latitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LatLon.lat) /
          kResolutionRatio;
      longitude_ =
          (double)(road_point->posOffset.offsetLL.choice.position_LatLon.lon) /
          kResolutionRatio;
      coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                          point_y_);
      points->set_x(point_x_);
      points->set_y(point_y_);
      break;

    default:
      LOG(ERROR) << "points.present" << road_point->posOffset.offsetLL.present;
      return false;
      break;
  }
  return true;
}

bool Trafficlight::ReadMapConnectsTo(const Connection_t* connects_to,
                                     apollo::v2x::Connection* connections) {
  if (connects_to == nullptr) {
    LOG(ERROR) << "Error: Map connectsTo info empty !";
    return false;
  }
  // connectingLane maneuver
  if (connects_to->connectingLane != nullptr) {
    int maneuvers = 0;
    memcpy(&maneuvers, connects_to->connectingLane->maneuver->buf,
           sizeof(uint8_t) * connects_to->connectingLane->maneuver->size);
    if (connects_to->connectingLane->maneuver->size > 1) {
      maneuvers = ntohs(maneuvers);
    }
    switch (maneuvers) {
      case 0:
        connections->set_allow_driving_behavior(
            ::apollo::v2x::Connection::STRAIGHT);
        break;
      case 1:
        connections->set_allow_driving_behavior(
            ::apollo::v2x::Connection::LEFT);
        break;
      case 2:
        connections->set_allow_driving_behavior(
            ::apollo::v2x::Connection::RIGHT);
        break;
      case 3:
        connections->set_allow_driving_behavior(
            ::apollo::v2x::Connection::U_TURN);
        break;

      default:
        LOG(ERROR) << "AllowedManeuvers" << maneuvers;
        return false;
        break;
    }
  }
  // phaseID
  if (connects_to->phaseId != nullptr) {
    connections->set_phase_id(*(connects_to->phaseId));
  }
  return true;
}

bool Trafficlight::ReadMapLane(const Lane_t* plane, apollo::v2x::Lane* lanes) {
  if (plane == nullptr) {
    LOG(ERROR) << "Error: Map lane info is empty!";
    return false;
  }
  lanes->set_lane_id(plane->laneID);
  if (plane->maneuvers != nullptr) {
    int lane_maneuvers = 0;
    memcpy(&lane_maneuvers, plane->maneuvers->buf,
           sizeof(uint8_t) * plane->maneuvers->size);
    if (plane->maneuvers->size > 1) {
      lane_maneuvers = ntohs(lane_maneuvers);
    }
  }
  if (plane->connectsTo != nullptr) {
    for (int m = 0; m < plane->connectsTo->list.count; m++) {
      bool connect_falg = ReadMapConnectsTo(plane->connectsTo->list.array[m],
                                            lanes->add_connections());

      if (!connect_falg) {
        LOG(ERROR) << "Error: Map_connects_Fill failed ";
        return false;
      }
    }
  }
  if (plane->points != nullptr) {
    for (int n = 0; n < plane->points->list.count; n++) {
      bool points_flag = ReadMapPosition(plane->points->list.array[n],
                                         lanes->add_position_offset());

      if (!points_flag) {
        LOG(ERROR) << "Error: Map_connects_Fill failed ";
        return false;
      }
    }
  }
  return true;
}

bool Trafficlight::ReadMapRoad(const Link_t* pInlink_j,
                               apollo::v2x::Road* roads) {
  roads->set_upstream_node_id(pInlink_j->upstreamNodeId.id);
  for (int k = 0; k < pInlink_j->lanes.list.count; k++) {
    bool lane_falg =
        ReadMapLane(pInlink_j->lanes.list.array[k], roads->add_lanes());
    if (!lane_falg) {
      LOG(ERROR) << "Error: Map_LaneInfo_Fill failed";
      return false;
    }
  }
  return true;
}

bool Trafficlight::ReadMapIntersection(
    const Node_t* nodes, apollo::v2x::Intersection* intersections) {
  intersections->set_id(nodes->id.id);
  latitude_ = (double)(nodes->refPos.lat) / kResolutionRatio;
  longitude_ = (double)(nodes->refPos.Long) / kResolutionRatio;
  apollo::v2x::Position2D* refpos = intersections->mutable_position();
  coordinate_ransition_.LatLonToUTMXY(latitude_, longitude_, point_x_,
                                      point_y_);
  refpos->set_x(point_x_);
  refpos->set_y(point_y_);
  for (int j = 0; j < nodes->inLinks->list.count; j++) {
    bool road_falg =
        ReadMapRoad(nodes->inLinks->list.array[j], intersections->add_roads());
    if (!road_falg) {
      LOG(ERROR) << "Error: Map_RoadInfo_Fill failed";
      return false;
    }
  }

  return true;
}

bool Trafficlight::ReadMap(const MapData_t* rsu_map, apollo::v2x::Map* map) {
  if (nullptr == rsu_map) {
    LOG(ERROR) << "Error: Map info is empty!";
    return false;
  }
  // nodes, means intersection or road segment
  map->set_msg_cnt(rsu_map->msgCnt);
  for (int i = 0; i < rsu_map->nodes.list.count; i++) {
    bool intersection_flag = ReadMapIntersection(rsu_map->nodes.list.array[i],
                                                 map->add_intersections());
    if (!intersection_flag) {
      LOG(ERROR) << "Error: Map_IntersectionInfo_Fill failed";
      return false;
    }
  }
  return true;
}

bool Trafficlight::ReadSpat(const SPAT_t* rsu_spat, apollo::v2x::Spat* spat) {
  IntersectionState_t* intersection_state = nullptr;
  Phase_t* phase = nullptr;
  PhaseState_t* phase_state = nullptr;
  if (rsu_spat == nullptr) {
    LOG(ERROR) << "Error: Spat info is empty!";
    return false;
  }
  spat->set_msg_cnt(rsu_spat->msgCnt);
  for (int i = 0; i < rsu_spat->intersections.list.count; i++) {
    if (rsu_spat->intersections.list.array[i] == nullptr) {
      LOG(ERROR) << "rsu_spat->intersections.list.array[i] == nullptr";
      return false;
    }
    intersection_state = rsu_spat->intersections.list.array[i];
    apollo::v2x::IntersectionState* pb_intersection_state =
        spat->add_intersections();
    pb_intersection_state->set_intersection_id(
        intersection_state->intersectionId.id);
    if (intersection_state->moy == nullptr ||
        intersection_state->timeStamp == nullptr) {
      LOG(WARNING) << "intersection_state->moy || "
                      "intersection_state->timeStamp == nullptr";
    } else {
      pb_intersection_state->set_moy(*(intersection_state->moy));
      pb_intersection_state->set_time_stamp_dsecond(
          *(intersection_state->timeStamp));
    }
    for (int j = 0; j < intersection_state->phases.list.count; j++) {
      if (intersection_state->phases.list.array[j] == nullptr) {
        LOG(ERROR) << "intersection_state->phases.list.array[j] == nullptr";
        return false;
      }
      phase = intersection_state->phases.list.array[j];
      ::apollo::v2x::Phase* pb_phase_state =
          pb_intersection_state->add_phases();
      pb_phase_state->set_id(phase->id);
      for (int k = 0; k < phase->phaseStates.list.count; k++) {
        if (phase->phaseStates.list.array[k] == nullptr) {
          LOG(ERROR) << "phase->phaseStates.list.array[%d] == nullptr";
          return false;
        }
        phase_state = phase->phaseStates.list.array[k];
        // Timing
        if (phase_state->timing->startTime == 0) {
          switch (phase_state->light) {
            case 0:
              pb_phase_state->set_color(::apollo::v2x::Phase::UNKNOWN);
              break;
            case 1:
              pb_phase_state->set_color(::apollo::v2x::Phase::BLACK);
              break;
            case 3:
              pb_phase_state->set_color(::apollo::v2x::Phase::RED);
              break;
            case 7:
              pb_phase_state->set_color(::apollo::v2x::Phase::YELLOW);
              break;
            case 5:
              pb_phase_state->set_color(::apollo::v2x::Phase::GREEN);
              break;
            case 6:
              pb_phase_state->set_color(::apollo::v2x::Phase::GREEN);
              break;
            case 8:
              pb_phase_state->set_color(::apollo::v2x::Phase::FLASH_GREEN);
              break;
            default:
              LOG(ERROR) << "phase_state->light: " << phase_state->light;
              return false;
              break;
          }
          // likelyTime
          int color_remaining_time_s =
              (int)(phase_state->timing->likelyEndTime / 10);
          pb_phase_state->set_color_remaining_time_s(color_remaining_time_s);
        }
      }
    }
  }
  return true;
}

}  // namespace v2x
//}  // namespace apollo
