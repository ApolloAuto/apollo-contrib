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
 * @file proxy.cc
 * @brief define Proxy class
 */

#include "app/proxy/proxy.h"

#include "glog/logging.h"
namespace v2x {

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::CompletionQueue;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;

using apollo::perception::PerceptionObstacles;
using apollo::v2x::CarStatus;
using apollo::v2x::IntersectionTrafficLightData;
using apollo::v2x::StatusResponse;
using apollo::v2x::UpdateStatus;

void V2xProxy::CarToObuService::Start() {
  if (server_thread_ == nullptr) {
    server_thread_.reset(new std::thread(
        std::bind(&V2xProxy::CarToObuService::ServerThread, this)));
    stop_flag_ = false;
  }
}

void V2xProxy::CarToObuService::ServerThread() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address_, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

  LOG(INFO) << "Grpc server listening on " << address_;

  std::unique_lock<std::mutex> lk(mutex_);
  condition_.wait(lk, [&]() { return stop_flag_; });
  server->Shutdown();
}

void V2xProxy::ObuToCarClient::Start() {
  if (client_thread_ == nullptr) {
    cq_.reset(new grpc::CompletionQueue);
    client_thread_.reset(new std::thread(
        std::bind(&V2xProxy::ObuToCarClient::ClientThread, this)));
  }
}

void V2xProxy::ObuToCarClient::ClientThread() {
  void* got_tag;
  bool ok = false;
  while (cq_->Next(&got_tag, &ok)) {
    AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

    GPR_ASSERT(ok);

    if (!call->status.ok()) {
      LOG_EVERY_N(ERROR, 10) << "Client send failed";
    }

    delete call;
  }
  cq_.reset(nullptr);
}

void V2xProxy::CarToObuService::Stop() {
  if (server_thread_ != nullptr) {
    {
      std::lock_guard<std::mutex> lk(mutex_);
      stop_flag_ = true;
    }
    condition_.notify_one();
    server_thread_->join();
    server_thread_.reset(nullptr);
  }
}

void V2xProxy::ObuToCarClient::Stop() {
  if (client_thread_ != nullptr) {
    if (cq_ != nullptr) {
      cq_->Shutdown();
    }
    client_thread_->join();
    client_thread_.reset(nullptr);
  }
}

grpc::Status V2xProxy::CarToObuService::PushCarStatus(ServerContext* context,
                                                      const CarStatus* request,
                                                      UpdateStatus* response) {
  car_status_.push(*request);
  response->set_updated(true);

  return grpc::Status::OK;
}
/*
grpc::Status V2xProxy::CarToObuService::PushPerceptionResult(
    ServerContext* context, const PerceptionObstacles* request,
    UpdateStatus* response) {
  perception_obstacles_.push(*request);
  response->set_updated(false);

  return grpc::Status::OK;
}
*/
std::shared_ptr<CarStatus> V2xProxy::CarToObuService::pop_carstatus() {
  return car_status_.try_pop();
}

std::shared_ptr<PerceptionObstacles>
V2xProxy::CarToObuService::pop_obstacles() {
  return perception_obstacles_.try_pop();
}

std::shared_ptr<CarStatus> V2xProxy::GetCarStatus() {
  return service_.pop_carstatus();
}

std::shared_ptr<PerceptionObstacles> V2xProxy::GetObstacles() {
  return service_.pop_obstacles();
}

void V2xProxy::ObuToCarClient::SendPerceptionObstacles(
    const apollo::v2x::V2XObstacles& request) {
  AsyncClientCall* call = new AsyncClientCall;
  call->response_reader = stub_->PrepareAsyncSendPerceptionObstacles(
      &call->context, request, cq_.get());
  call->response_reader->StartCall();
  call->response_reader->Finish(&call->reply, &call->status,
                                static_cast<void*>(call));
}

void V2xProxy::SendObstacles(const apollo::v2x::V2XObstacles& request) {
  client_.SendPerceptionObstacles(request);
}

void V2xProxy::ObuToCarClient::SendV2xTrafficLight(
    const apollo::v2x::obu::ObuTrafficLight& request) {
  AsyncClientCall* call = new AsyncClientCall;
  call->response_reader = stub_->PrepareAsyncSendV2xTrafficLight(
      &call->context, request, cq_.get());
  call->response_reader->StartCall();
  call->response_reader->Finish(&call->reply, &call->status,
                                static_cast<void*>(call));
}

void V2xProxy::SendTrafficLights(
    const apollo::v2x::obu::ObuTrafficLight& request) {
  client_.SendV2xTrafficLight(request);
}

}  // namespace v2x
