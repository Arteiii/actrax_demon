//
// Created by arteii on 11/28/24.
//

#include "../include/AntiAnalysis.hpp"
#include "../include/StringEncryptionUtils.hpp"

#include <array>
#include <string>
#include <vector>

#include <windows.h>

#include <tlhelp32.h>

auto
Integrity::AntiAnalysis::AnalysisToolsProcess() -> std::vector<std::wstring>*
{
  const std::array processes = {
    ENCRYPT_STR("ollydbg.exe"),          // OllyDebug debugger
    ENCRYPT_STR("ollyice.exe"),          // OllyICE debugger
    ENCRYPT_STR("ProcessHacker.exe"),    // Process Hacker
    ENCRYPT_STR("tcpview.exe"),          // Part of Sysinternals Suite
    ENCRYPT_STR("autoruns.exe"),         // Part of Sysinternals Suite
    ENCRYPT_STR("autorunsc.exe"),        // Part of Sysinternals Suite
    ENCRYPT_STR("filemon.exe"),          // Part of Sysinternals Suite
    ENCRYPT_STR("procmon.exe"),          // Part of Sysinternals Suite
    ENCRYPT_STR("regmon.exe"),           // Part of Sysinternals Suite
    ENCRYPT_STR("procexp.exe"),          // Part of Sysinternals Suite
    ENCRYPT_STR("idaq.exe"),             // IDA Pro Interactive Disassembler
    ENCRYPT_STR("idaq64.exe"),           // IDA Pro Interactive Disassembler
    ENCRYPT_STR("ImmunityDebugger.exe"), // ImmunityDebugger
    ENCRYPT_STR("Wireshark.exe"),        // Wireshark packet sniffer
    ENCRYPT_STR("dumpcap.exe"),          // Network traffic dump tool
    ENCRYPT_STR("HookExplorer.exe"),     // Find various types of runtime hooks
    ENCRYPT_STR("ImportREC.exe"),        // Import Reconstructor
    ENCRYPT_STR("PETools.exe"),          // PE Tool
    ENCRYPT_STR("LordPE.exe"),           // LordPE
    ENCRYPT_STR("SysInspector.exe"),     // ESET SysInspector
    ENCRYPT_STR("proc_analyzer.exe"),    // Part of SysAnalyzer iDefense
    ENCRYPT_STR("sysAnalyzer.exe"),      // Part of SysAnalyzer iDefense
    ENCRYPT_STR("sniff_hit.exe"),        // Part of SysAnalyzer iDefense
    ENCRYPT_STR("windbg.exe"),           // Microsoft WinDbg
    ENCRYPT_STR("joeboxcontrol.exe"),    // Part of Joe Sandbox
    ENCRYPT_STR("joeboxserver.exe"),     // Part of Joe Sandbox
    ENCRYPT_STR("ResourceHacker.exe"),   // Resource Hacker
    ENCRYPT_STR("x32dbg.exe"),           // x32dbg
    ENCRYPT_STR("x64dbg.exe"),           // x64dbg
    ENCRYPT_STR("Fiddler.exe"),          // Fiddler
    ENCRYPT_STR("httpdebugger.exe"),     // Http Debugger
    ENCRYPT_STR("cheatengine-i386.exe"), // Cheat Engine
    ENCRYPT_STR("cheatengine-x86_64.exe"),           // Cheat Engine
    ENCRYPT_STR("cheatengine-x86_64-SSE4-AVX2.exe"), // Cheat Engine
    ENCRYPT_STR("frida-helper-32.exe"),              // Frida
    ENCRYPT_STR("frida-helper-64.exe"),              // Frida
  };
  auto* foundTools = new std::vector<std::wstring>();

  const HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    return nullptr;
  }

  PROCESSENTRY32W pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32W);

  if (Process32FirstW(hProcessSnap, &pe32) == 0) {
    CloseHandle(hProcessSnap);
    return nullptr;
  }

  do {
    for (const auto& encryptedProcessName : processes) {
      if (std::string decryptedProcessName =
            DecryptString(encryptedProcessName);
          _wcsicmp(pe32.szExeFile,
                   std::wstring(decryptedProcessName.begin(),
                                decryptedProcessName.end())
                     .c_str()) == 0) {
        foundTools->emplace_back(pe32.szExeFile);
        break;
      }
    }
  } while (Process32NextW(hProcessSnap, &pe32) != 0);
  CloseHandle(hProcessSnap);

  if (foundTools->empty()) {
    delete foundTools;
    return nullptr;
  }

  return foundTools;
}