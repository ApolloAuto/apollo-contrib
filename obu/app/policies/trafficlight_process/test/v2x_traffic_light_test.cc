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
 * @file v2x_traffic_light_test.cc
 * @brief The traffic light program for the v2x module
 */

#include "policies/trafficlight_process/v2x_traffic_light.h"
#include "proxy/proxy.h"
#include "gflags/gflags.h"

int g_color_remaining_time = 30;

void set_policy(apollo::v2x::PolicyData& policy_data_pb) {
    apollo::v2x::Map* map_pb = policy_data_pb.mutable_map();
    apollo::v2x::Spat* spat_pb = policy_data_pb.mutable_spat();
    map_pb->set_msg_cnt(0);

    apollo::v2x::Intersection* intersection0 = map_pb->add_intersections();
    intersection0->set_id(0);
    intersection0->mutable_position()->set_x(437544.060390);
    intersection0->mutable_position()->set_y(4432885.512208);

    apollo::v2x::Road* road0 = intersection0->add_roads();
    apollo::v2x::Lane* lane0_0 = road0->add_lanes();
    lane0_0->set_lane_id(0);
    apollo::v2x::Position2D* point0 = lane0_0->add_position_offset();
    point0->set_x(437563.244215);
    point0->set_y(4432817.089782);
    apollo::v2x::Position2D* point1 = lane0_0->add_position_offset();
    point1->set_x(437556.260441);
    point1->set_y(4432849.670223);
    apollo::v2x::Position2D* point2 = lane0_0->add_position_offset();
    point2->set_x(437552.949001);
    point2->set_y(4432872.341472);
    apollo::v2x::Connection* connection0_0 = lane0_0->add_connections();
    connection0_0->set_allow_driving_behavior(apollo::v2x::Connection::STRAIGHT);
    connection0_0->set_phase_id(2);

    apollo::v2x::Road* road1 = intersection0->add_roads();
    apollo::v2x::Lane* lane1_0 = road1->add_lanes();
    lane1_0->set_lane_id(1);
    apollo::v2x::Position2D* point3 = lane1_0->add_position_offset();
    point3->set_x(437633.397056);
    point3->set_y(4432913.749621);
    apollo::v2x::Position2D* point4 = lane1_0->add_position_offset();
    point4->set_x(437589.964083);
    point4->set_y(4432902.229316);
    apollo::v2x::Position2D* point5 = lane1_0->add_position_offset();
    point5->set_x(437561.583178);
    point5->set_y(4432895.247529);
    apollo::v2x::Connection* connection1_0 = lane1_0->add_connections();
    connection1_0->set_allow_driving_behavior(apollo::v2x::Connection::STRAIGHT);
    connection1_0->set_phase_id(1);

    apollo::v2x::Road* road2 = intersection0->add_roads();
    apollo::v2x::Lane* lane2_0 = road2->add_lanes();
    lane2_0->set_lane_id(2);
    apollo::v2x::Position2D* point6 = lane2_0->add_position_offset();
    point6->set_x(437525.616799);
    point6->set_y(4432992.890378);
    apollo::v2x::Position2D* point7 = lane2_0->add_position_offset();
    point7->set_x(437533.279508);
    point7->set_y(4432949.537297);
    apollo::v2x::Position2D* point8 = lane2_0->add_position_offset();
    point8->set_x(437540.543593);
    point8->set_y(4432899.194496);
    apollo::v2x::Connection* connection2_0 = lane2_0->add_connections();
    connection2_0->set_allow_driving_behavior(apollo::v2x::Connection::STRAIGHT);
    connection2_0->set_phase_id(2);

    apollo::v2x::Road* road3 = intersection0->add_roads();
    apollo::v2x::Lane* lane3_0 = road3->add_lanes();
    lane3_0->set_lane_id(3);
    apollo::v2x::Position2D* point9 = lane3_0->add_position_offset();
    point9->set_x(437465.898447);
    point9->set_y(4432858.072548);
    apollo::v2x::Position2D* point10 = lane3_0->add_position_offset();
    point10->set_x(437506.914988);
    point10->set_y(4432866.171025);
    apollo::v2x::Position2D* point11 = lane3_0->add_position_offset();
    point11->set_x(437539.151270);
    point11->set_y(4432875.118913);
    apollo::v2x::Connection* connection3_0 = lane3_0->add_connections();
    connection3_0->set_allow_driving_behavior(apollo::v2x::Connection::STRAIGHT);
    connection3_0->set_phase_id(1);

    // spat
    spat_pb->set_msg_cnt(0);
    apollo::v2x::IntersectionState* intersection_state0 =
        spat_pb->add_intersections();
    intersection_state0->set_intersection_id(0);
    apollo::v2x::Phase* phase_date1 = intersection_state0->add_phases();
    phase_date1->set_id(1);
    phase_date1->set_color(apollo::v2x::Phase::GREEN);
    phase_date1->set_color_remaining_time_s(g_color_remaining_time);

    apollo::v2x::Phase* phase_date2 = intersection_state0->add_phases();
    phase_date2->set_id(2);
    phase_date2->set_color(apollo::v2x::Phase::RED);
    phase_date2->set_color_remaining_time_s(g_color_remaining_time);
    g_color_remaining_time--;
    if (g_color_remaining_time < 0) {
        g_color_remaining_time = 60;
    }
}

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    google::ParseCommandLineFlags(&argc, &argv, true);
    v2x::V2xProxy proxy("192.168.10.121:50100", "192.168.10.6:50101");
    v2x::Trafficlight tl;
    while (true) {
        std::shared_ptr<apollo::v2x::IntersectionTrafficLightData>
        intersection_trafficlights(
            new apollo::v2x::IntersectionTrafficLightData);
        apollo::v2x::PolicyData policy_data;
        // SPAT_t* spat = new SPAT_t;
        // SPAT_t* spat = (SPAT_t*)malloc(sizeof(SPAT_t));
        // MapData_t* map = new MapData_t;
        set_policy(policy_data);
        // std::cout << policy_data.DebugString() << std::endl;
        double x = 0.0;
        double y = 0.0;
        std::cout.unsetf(std::ios::scientific);
        std::cout.setf(std::ios::fixed);
        std::cout.precision(11);
        std::cout << "x: " << x << std::endl;
        std::cout << "y: " << y << std::endl;
        std::shared_ptr<apollo::v2x::CarStatus> car_status = proxy.GetCarStatus();
        bool policy_falg = tl.TrafficLightPolicy(policy_data, *car_status,
                           intersection_trafficlights);
        if (!policy_falg) {
            std::cout << "policy error or msg : " << std::endl;
        }
        proxy.SendTrafficLights(*intersection_trafficlights);
        //
        sleep(1);
    }
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
