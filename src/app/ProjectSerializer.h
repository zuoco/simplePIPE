#pragma once

#include "app/Document.h"

#include <memory>
#include <string>

namespace app {

/// 工程 JSON 序列化/反序列化。
class ProjectSerializer {
public:
    static bool save(const Document& document, const std::string& filePath);

    /// 读取 JSON 工程并重建对象关系，失败返回 nullptr。
    static std::unique_ptr<Document> load(const std::string& filePath);
};

} // namespace app
