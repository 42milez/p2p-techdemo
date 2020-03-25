#ifndef P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_
#define P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_

#include <atomic>
#include <functional>
#include <sstream>
#include <string>
#include <thread>

#include "logger.h"
#include "singleton.h"

namespace core
{
    class AsyncWorker
    {
      public:
        AsyncWorker(std::function<void()> &&task);

        inline void
        Run()
        {
            thread_ = std::thread([this]{
              auto id = std::this_thread::get_id();
              std::stringstream ss;
              ss << id;
              core::Singleton<core::Logger>::Instance().Debug("worker started (thread {0})", ss.str());
              while (!stopped_) {
                task_();
              }
              core::Singleton<core::Logger>::Instance().Debug("worker stopped (thread {0})", ss.str());
            });
        };

        inline void
        Stop()
        {
            stopped_ = true;

            if (thread_.joinable()) {
                thread_.join();
            }
        }

      private:
        std::atomic<bool> stopped_;
        std::function<void()> task_;
        std::thread thread_;
    };
} // namespace core

#endif // P2P_TECHDEMO_LIB_CORE_ASYNC_WORKER_H_
