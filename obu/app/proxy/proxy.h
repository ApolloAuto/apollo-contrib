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
 * @file proxy.h
 * @brief define Proxy class
 */

#ifndef APP_PROXY_PROXY_H_
#define APP_PROXY_PROXY_H_

#include <grpc/grpc.h>
#include <grpc/support/log.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "app/proto/include/v2x_service_car_to_obu.grpc.pb.h"
#include "app/proto/include/v2x_service_obu_to_car.grpc.pb.h"
#include "macro.h"
#include "cmn_queue.h"
#include "singleton.h"

namespace v2x {

class V2xProxy {
 public:
  V2xProxy(const std::string& rsu_or_obu_address,
           const std::string& svb_address)
      : service_(rsu_or_obu_address), client_(svb_address) {}

  ~V2xProxy() {
    StopAll();
  }

  void StartAll() {
    ServerStart();
    ClientStart();
  }
  void StopAll() {
    service_.Stop();
    client_.Stop();
  }

  void ServerStart() {
    service_.Start();
  }
  void ClientStart() {
    client_.Start();
  }

  std::shared_ptr<apollo::perception::PerceptionObstacles> GetObstacles();
  std::shared_ptr<apollo::v2x::CarStatus> GetCarStatus();

  void SendObstacles(const apollo::perception::PerceptionObstacles& request);
  void SendTrafficLights(
      const apollo::v2x::IntersectionTrafficLightData& request);

 private:
  class CarToObuService final : public apollo::v2x::CarToObu::Service {
   public:
    explicit CarToObuService(const std::string& address)
        : address_(address), server_thread_(nullptr), stop_flag_(false) {}

    grpc::Status PushCarStatus(grpc::ServerContext* context,
                               const apollo::v2x::CarStatus* request,
                               apollo::v2x::UpdateStatus* response) override;

    grpc::Status PushPerceptionResult(
        grpc::ServerContext* context,
        const apollo::perception::PerceptionObstacles* request,
        apollo::v2x::UpdateStatus* response) override;

    void Start();
    void Stop();

    std::shared_ptr<apollo::perception::PerceptionObstacles> pop_obstacles();
    std::shared_ptr<apollo::v2x::CarStatus> pop_carstatus();

   private:
    void ServerThread();

    std::string address_;
    std::unique_ptr<std::thread> server_thread_ = nullptr;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool stop_flag_ = false;

    CmnQueue<apollo::v2x::CarStatus> car_status_;
    CmnQueue<apollo::perception::PerceptionObstacles> perception_obstacles_;

    DISALLOW_COPY_AND_ASSIGN(CarToObuService);
  };

  class ObuToCarClient {
   public:
    explicit ObuToCarClient(const std::string& address)
        : address_(address), client_thread_(nullptr), cq_(nullptr) {
      channel_ =
          grpc::CreateChannel(address_, grpc::InsecureChannelCredentials());
      stub_ = apollo::v2x::ObuToCar::NewStub(channel_);
    }

    void SendPerceptionObstacles(
        const apollo::perception::PerceptionObstacles& request);

    void SendPerceptionTrafficLight(
        const apollo::perception::TrafficLightDetection& request);

    void SendV2xTrafficLight(
        const apollo::v2x::IntersectionTrafficLightData& request);

    void Start();
    void Stop();

   private:
    struct AsyncClientCall {
      apollo::v2x::StatusResponse reply;
      grpc::ClientContext context;
      grpc::Status status;

      std::unique_ptr<
          grpc::ClientAsyncResponseReader<apollo::v2x::StatusResponse>>
          response_reader;
    };

    void ClientThread();

    std::string address_;
    std::unique_ptr<std::thread> client_thread_ = nullptr;
    std::shared_ptr<grpc::Channel> channel_ = nullptr;
    std::unique_ptr<apollo::v2x::ObuToCar::Stub> stub_ = nullptr;
    std::unique_ptr<grpc::CompletionQueue> cq_ = nullptr;
  };

  CarToObuService service_;
  ObuToCarClient client_;

  DISALLOW_COPY_AND_ASSIGN(V2xProxy);
};
typedef SingletonService<V2xProxy> V2xProxySingleton;
}  // namespace v2x

#endif  // APP_PROXY_PROXY_H_
