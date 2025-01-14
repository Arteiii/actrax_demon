#ifndef TASKS_HPP
#define TASKS_HPP

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <variant>

struct Inject
{
  std::string dllName;
  uint32_t processId;
};

struct RunPowerShell
{
  std::string scriptPath;
};

struct ExecuteBatch
{
  std::string batchFilePath;
};

using operation_type = std::variant<Inject, RunPowerShell, ExecuteBatch>;

struct Task
{
  std::string id;
  std::string description;
  std::string status;
  std::string clientId;
  operation_type operation;
};

inline void
from_json(const nlohmann::json& j, Inject& op)
{
  j.at("dll_name").get_to(op.dllName);
  j.at("process_id").get_to(op.processId);
}

inline void
from_json(const nlohmann::json& j, RunPowerShell& op)
{
  j.at("script_path").get_to(op.scriptPath);
}

inline void
from_json(const nlohmann::json& j, ExecuteBatch& op)
{
  j.at("batch_file_path").get_to(op.batchFilePath);
}

inline void
from_json(const nlohmann::json& j, operation_type& op)
{
  if (j.contains("Inject")) {
    op = j.at("Inject").get<Inject>();
  } else if (j.contains("RunPowerShell")) {
    op = j.at("RunPowerShell").get<RunPowerShell>();
  } else if (j.contains("ExecuteBatch")) {
    op = j.at("ExecuteBatch").get<ExecuteBatch>();
  } else {
    throw std::runtime_error("Unknown OperationType");
  }
}

inline void
from_json(const nlohmann::json& j, Task& task)
{
  if (j.at("id").is_string()) {
    j.at("id").get_to(task.id);
  } else {
    task.id = std::to_string(j.at("id").get<uint64_t>());
  }

  if (j.at("client_id").is_string()) {
    j.at("client_id").get_to(task.clientId);
  } else {
    task.clientId = std::to_string(j.at("client_id").get<uint32_t>());
  }

  j.at("description").get_to(task.description);
  j.at("status").get_to(task.status);

  from_json(j.at("operation"), task.operation);
}

inline Task
fromJsonString(const std::string& jsonString)
{
  nlohmann::json j = nlohmann::json::parse(jsonString);
  Task task;
  from_json(j, task);
  return task;
}

#endif // TASKS_HPP
