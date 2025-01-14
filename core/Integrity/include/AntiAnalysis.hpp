//
// Created by arteii on 11/28/24.
//

#ifndef ANTIANALYSIS_HPP
#define ANTIANALYSIS_HPP

#include <string>
#include <vector>

namespace Integrity::AntiAnalysis {

[[nodiscard]]
auto
AnalysisToolsProcess() -> std::vector<std::wstring>*;

} // namespace Integrity::AntiAnalysis

#endif // ANTIANALYSIS_HPP
