// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "model/DocumentObject.h"
#include "foundation/Types.h"

#include <string>
#include <vector>
#include <algorithm>

namespace model {

/// 组合方法：如何将多个基本工况结果合并
enum class CombineMethod {
    Algebraic,   ///< 代数叠加: Σ(factor × result)
    Absolute,    ///< 绝对值叠加: Σ|factor × result|（保守估计）
    SRSS,        ///< 平方和开方: √Σ(result²)（不相关载荷，如地震）
    Envelope     ///< 包络：取各工况最大/最小值
};

/// 应力类别：B31.3 规范校核依据
enum class StressCategory {
    Sustained,    ///< 持续应力 (SUS) — 自重+内压，不得超过 Sh
    Expansion,    ///< 热膨胀应力 (EXP) — 热态-冷态，不得超过 Sa
    Occasional,   ///< 偶然应力 (OCC) — 持续+风/地震，不得超过 1.33Sh
    Operating,    ///< 操作工况 (OPE) — 检查位移和力
    Hydrotest     ///< 水压试验 (HYD)
};

/// 工况条目：组合工况中单个基本工况的引用和系数
struct CaseEntry {
    foundation::UUID caseId;   ///< 指向 LoadCase 文档对象
    double factor = 1.0;       ///< 组合系数
};

/// 组合工况：按规范公式将多个基本工况结果进行代数运算
///
/// 依赖关系（严格 DAG）: LoadCombination → LoadCase → Load
///
/// 示例（B31.3 典型配置）:
///   SUS (Sustained,  Algebraic) = W×1.0 + P1×1.0
///   EXP (Expansion,  Algebraic) = T1×1.0
///   OPE (Operating,  Algebraic) = W×1.0 + T1×1.0 + P1×1.0
///   OCC (Occasional, SRSS)      = W×1.0 + P1×1.0 + EQ×1.0
class LoadCombination : public DocumentObject {
public:
    explicit LoadCombination(const std::string& name = "",
                             StressCategory category = StressCategory::Sustained,
                             CombineMethod method = CombineMethod::Algebraic)
        : DocumentObject(name), category_(category), method_(method) {}

    ~LoadCombination() override = default;

    StressCategory category() const { return category_; }
    void setCategory(StressCategory cat) {
        if (category_ != cat) { category_ = cat; changed.emit(); }
    }

    CombineMethod method() const { return method_; }
    void setMethod(CombineMethod m) {
        if (method_ != m) { method_ = m; changed.emit(); }
    }

    /// 添加工况条目。若已存在相同 caseId，则忽略
    void addCaseEntry(const CaseEntry& entry) {
        auto it = std::find_if(caseEntries_.begin(), caseEntries_.end(),
            [&](const CaseEntry& e) { return e.caseId == entry.caseId; });
        if (it == caseEntries_.end()) {
            caseEntries_.push_back(entry);
            changed.emit();
        }
    }

    /// 移除指定 caseId 的条目。成功返回 true，未找到返回 false
    bool removeCaseEntry(const foundation::UUID& caseId) {
        auto it = std::find_if(caseEntries_.begin(), caseEntries_.end(),
            [&](const CaseEntry& e) { return e.caseId == caseId; });
        if (it != caseEntries_.end()) {
            caseEntries_.erase(it);
            changed.emit();
            return true;
        }
        return false;
    }

    const std::vector<CaseEntry>& caseEntries() const { return caseEntries_; }

private:
    StressCategory category_;
    CombineMethod  method_;
    std::vector<CaseEntry> caseEntries_;
};

} // namespace model
