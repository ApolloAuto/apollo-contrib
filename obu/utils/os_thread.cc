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
 * @file os_thread.cc
 * @brief implement thread class
 */

#include <errno.h>
#include <cstring>
#include "glog/logging.h"
#include <iostream>
#include "os_thread.h"

namespace v2x {

OsThread::OsThread(const char* pName, int prio, int stacksize, bool joinable)
    : name_(pName),
      prio_(prio),
      stack_size_(stacksize),
      joinable_(joinable),
      thread_id_(0) {}

OsThread::~OsThread() {
  if (thread_id_) {
    int rc = pthread_cancel(thread_id_);

    if (0 != rc) {
      LOG(ERROR) << "OsThread: Failed to terminate thread " << rc;
    }

    thread_id_ = 0;
  }
}

void OsThread::Start() {
  if (thread_id_) {
    // The thread is already started
    return;
  }

  int rc = 0;

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  if (joinable_) {
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (0 != rc) {
      LOG(ERROR) << "OsThread: set joinable attribute failed (" << rc << ")";
    }
  } else {
    rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 != rc) {
      LOG(ERROR) << "OsThread: set detached attribute failed (" << rc << ")";
    }
  }

  rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (0 != rc) {
     LOG(ERROR) << "OsThread: set inherit schedule failed (" << rc << ")";
  }

  rc = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  if (0 != rc) {
     LOG(ERROR) << "OsThread: set schedule policy failed (" << rc << ")";
  }

  sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = prio_;

  rc = pthread_attr_setschedparam(&attr, &param);
  if (0 != rc) {
     LOG(ERROR) << "OsThread: set priority failed (" << rc << ")";
  }

  rc = pthread_attr_setstacksize(&attr, stack_size_);
  if (0 != rc) {
     LOG(ERROR) << "OsThread: set stack size failed (" << rc << ")";
  }

  rc = pthread_create(&thread_id_, &attr, OsThread::ThreadProc, this);
  if (0 != rc) {
     LOG(ERROR) << "OsThread: spawn the os thread failed (" << rc << ")";
  }
}

const char* OsThread::GetName() const {
  return name_.c_str();
}

void* OsThread::ThreadProc(void* arg) {
  OsThread* pthread = (OsThread*)arg;

  try {
    pthread->Run();
  } catch (...) {
     LOG(ERROR) << "OsThread"
                 "Exception is thrown in OsThread::Run !!! "
              << std::endl;
  }

  if (!pthread->joinable_) {
    pthread->thread_id_ = 0;
    delete pthread;
  }

  pthread_exit(0);

  return nullptr;
}

int OsThread::Join(int waitMs) {
  int rc = 0;

  if (os_wait_forever == waitMs) {
    rc = pthread_join(thread_id_, NULL);
  } else {
    struct timespec ts;

    if (-1 == clock_gettime(CLOCK_REALTIME, &ts)) {
       LOG(ERROR) << "get time failed";
      return 0;
    }

    ts.tv_nsec += waitMs * 1000;

    rc = pthread_timedjoin_np(thread_id_, NULL, &ts);
  }

  if (0 != rc) {
     LOG(ERROR) << "Error in joining thread " << name_ << " (" << thread_id_
              << ") Error=" << rc;
  }

  return 1;
}
}  // namespace v2x
