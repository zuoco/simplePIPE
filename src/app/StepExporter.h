#pragma once

#include "app/Document.h"

#include <string>

namespace app {

/// STEP 导出器：将 Document 中的管点几何导出为带层级的 STEP 装配。
/// 层级结构: Route -> Segment -> Component(PipePoint)
class StepExporter {
public:
    /// 导出文档中全部可推导几何到 STEP 文件。
    /// @return true 表示导出成功（至少导出一个几何体）
    static bool exportAll(const Document& document, const std::string& filePath);
};

} // namespace app