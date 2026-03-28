#pragma once

#include <TopoDS_Shape.hxx>
#include <string>
#include <unordered_map>

namespace engine {

/// 参数化构件的通用参数表
/// 由管径 OD 驱动，所有尺寸都是 OD 的函数
struct ComponentParams {
    double od = 0.0;             // 管外径 (mm) — 主驱动参数
    double wallThickness = 0.0;  // 壁厚 (mm)
    double bodyLength = 0.0;     // 构件长度
    double bodyWidth = 0.0;      // 构件宽度
    double bodyHeight = 0.0;     // 构件高度

    // 扩展参数映射，各模板按需使用
    std::unordered_map<std::string, double> extra;

    double get(const std::string& key, double fallback = 0.0) const {
        auto it = extra.find(key);
        return it != extra.end() ? it->second : fallback;
    }
    void set(const std::string& key, double value) { extra[key] = value; }
};

/// 参数化构件模板基类
/// 每个模板定义: 唯一ID、参数推导规则、固定拓扑的几何生成
class ComponentTemplate {
public:
    virtual ~ComponentTemplate() = default;

    /// 模板唯一标识符 (如 "Pipe", "Elbow", "GateValve" 等)
    virtual std::string templateId() const = 0;

    /// 从管径 OD 和壁厚推导所有构件参数
    virtual ComponentParams deriveParams(double od, double wt) const = 0;

    /// 根据参数生成 BRep 几何体 (原点居中，轴线沿 Z)
    virtual TopoDS_Shape buildShape(const ComponentParams& p) const = 0;
};

} // namespace engine
