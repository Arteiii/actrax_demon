#include "TaskScheduler.hpp"
#include "Logger.hpp"

template<typename ResultType>
TaskScheduler<ResultType>::TaskScheduler()
  : running_(true)
  , schedulerThread_(nullptr)
{
  LOG_INFO("Task Scheduler started.");

  InitializeCriticalSection(&tasksCriticalSection_);

  schedulerThread_ =
    CreateThread(nullptr,                    // default security attributes
                 0,                          // default stack size
                 &TaskScheduler::ThreadProc, // function to run in the thread
                 this,                       // thread parameter
                 0,                          // start immediately
                 nullptr);                   // no thread ID needed
}

template<typename ResultType>
TaskScheduler<ResultType>::~TaskScheduler()
{
  Stop();
  if (schedulerThread_ != nullptr) {
    WaitForSingleObject(schedulerThread_, INFINITE);
    CloseHandle(schedulerThread_);
  }

  DeleteCriticalSection(&tasksCriticalSection_);

  LOG_INFO("Task Scheduler stopped.");
}

template<typename ResultType>
auto
TaskScheduler<ResultType>::AddTask(task task,
                                   const std::chrono::milliseconds* interval,
                                   ResultType* result) -> void
{
  EnterCriticalSection(&tasksCriticalSection_);
  {
    tasks_.push_back({ std::move(task), interval, result });
  }
  LeaveCriticalSection(&tasksCriticalSection_);

  LOG_INFO("Task added to scheduler.");
}

template<typename ResultType>
auto WINAPI
TaskScheduler<ResultType>::ThreadProc(LPVOID param) -> DWORD
{
  auto* scheduler = static_cast<TaskScheduler*>(param);
  scheduler->Run();
  return 0;
}

template<typename ResultType>
auto
TaskScheduler<ResultType>::Run() -> void
{
  while (running_) {
    EnterCriticalSection(&tasksCriticalSection_);

    for (auto it = tasks_.begin(); it != tasks_.end();) {
      auto& [task, interval, result] = *it;

      if (running_) {
        try {
          if constexpr (std::is_void_v<ResultType>) {
            task();
          } else {
            if (result) {
              *result = task();
            }
          }

          LOG_DEBUG("Task executed successfully.");
        } catch (const std::exception& e) {
          LOG_ERROR(std::string("Task failed: ") + e.what());
        }
      }

      if (interval) {
        Sleep(interval->count());
        ++it;
      } else {
        it = tasks_.erase(it);
      }
    }

    LeaveCriticalSection(&tasksCriticalSection_);

    Sleep(10);
  }
}

template<typename ResultType>
auto
TaskScheduler<ResultType>::Stop() -> void
{
  running_ = false;
}

template<typename ResultType>
auto
TaskScheduler<ResultType>::WaitUntilStopped() const -> void
{

  if (schedulerThread_ != nullptr) {
    WaitForSingleObject(schedulerThread_, INFINITE);
  }
}

template class TaskScheduler<void>;
template class TaskScheduler<int>;
template class TaskScheduler<std::string>;
template class TaskScheduler<float>;
