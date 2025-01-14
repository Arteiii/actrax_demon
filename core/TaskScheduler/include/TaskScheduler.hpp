#ifndef TASKSCHEDULER_HPP
#define TASKSCHEDULER_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <vector>
#include <windows.h>

// TaskScheduler class definition
template<typename ResultType = void>
class TaskScheduler
{
public:
  using task = std::function<ResultType()>;

  TaskScheduler();
  ~
  TaskScheduler();

  /// Adds a task to the scheduler.
  /// @param task Function to be executed.
  /// @param interval Delay between task executions. If `nullptr`, the task runs
  /// only once.
  /// @param result Pointer to store the task result (can be `nullptr` if no
  /// result is needed).
  auto
  AddTask(task task,
          const std::chrono::milliseconds* interval,
          ResultType* result = nullptr) -> void;

  /// Stops the scheduler.
  auto
  Stop() -> void;

  /// Waits until all tasks have finished and scheduler stops.
  auto
  WaitUntilStopped() const -> void;

private:
  struct TimedTask
  {
    task task;
    const std::chrono::milliseconds* interval;
    ResultType* result; // Pointer to store result
  };

  static auto WINAPI
  ThreadProc(LPVOID param) -> DWORD;
  auto
  Run() -> void;

  std::atomic<bool> running_;
  CRITICAL_SECTION tasksCriticalSection_; // Windows Mutex
  HANDLE schedulerThread_;
  std::vector<TimedTask> tasks_;
};

#endif // TASKSCHEDULER_HPP
