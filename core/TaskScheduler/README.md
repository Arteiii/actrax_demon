# TaskScheduler

TaskScheduler is a lightweight and efficient C++ class for scheduling and running periodic tasks.
It uses the Windows API for thread synchronization and provides robust logging with customizable log levels.

## Features

- **Thread-safe task management** using Windows `CRITICAL_SECTION`.
- **Periodic task execution** with configurable intervals.
- **Comprehensive logging** using custom `Log::` macros.
- **Graceful shutdown** ensures running tasks complete before stopping.

## Installation

1. Include `TaskScheduler.hpp` and `TaskScheduler.cpp` in your project.
2. Ensure `Logger.hpp` is available for logging functionality.
3. Link your project with required libraries if applicable.

## Usage Example

Here’s a simple example demonstrating how to use the `TaskScheduler`:

```cpp
#include "TaskScheduler.hpp"
#include <iostream>

int main()
{
    TaskScheduler scheduler;

    scheduler.AddTask([] {
        std::cout << "Task 1 executed." << std::endl;
        LOG_INFO("Task 1 executed.");
    }, std::chrono::milliseconds(1000));

    scheduler.AddTask([] {
        std::cout << "Task 2 executed." << std::endl;
        LOG_INFO("Task 2 executed.");
    }, std::chrono::milliseconds(2000));

    std::thread schedulerThread([&scheduler] {
        scheduler.Run();
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    scheduler.Stop();

    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }

    return 0;
}
```

### Output

```plaintext
Task 1 executed.
Task 2 executed.
Task 1 executed.
Task 1 executed.
Task 2 executed.
...
```

## Logging Example

The following logs will be generated based on the execution:

```
[INFO] TaskScheduler initialized. (TaskScheduler.cpp:12)
[INFO] Task 1 executed. (main.cpp:12)
[INFO] Task 2 executed. (main.cpp:17)
...
[INFO] TaskScheduler main loop stopped. (TaskScheduler.cpp:56)
```
