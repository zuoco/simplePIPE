/// test_loadcase.cpp — T33: LoadCase 与 LoadCombination 单元测试

#include <gtest/gtest.h>

#include "model/LoadCase.h"
#include "model/LoadCombination.h"
#include "foundation/Types.h"

using namespace model;
using foundation::UUID;

// ============================================================
// LoadEntry / LoadCase 基本操作
// ============================================================

TEST(LoadCaseTest, ConstructDefault) {
    LoadCase lc("W");
    EXPECT_EQ(lc.caseName(), "W");
    EXPECT_TRUE(lc.entries().empty());
    EXPECT_FALSE(lc.id().isNull());
}

TEST(LoadCaseTest, AddEntry) {
    LoadCase lc("T1");
    UUID loadId = UUID::generate();
    LoadEntry entry{loadId, 1.0};

    lc.addEntry(entry);
    ASSERT_EQ(lc.entries().size(), 1u);
    EXPECT_EQ(lc.entries()[0].loadId, loadId);
    EXPECT_DOUBLE_EQ(lc.entries()[0].factor, 1.0);
}

TEST(LoadCaseTest, AddEntryDuplicateIgnored) {
    LoadCase lc("P1");
    UUID loadId = UUID::generate();

    lc.addEntry({loadId, 1.0});
    lc.addEntry({loadId, 2.0}); // 同一 loadId，应被忽略

    EXPECT_EQ(lc.entries().size(), 1u);
    EXPECT_DOUBLE_EQ(lc.entries()[0].factor, 1.0); // 第一次的值保留
}

TEST(LoadCaseTest, AddMultipleEntries) {
    LoadCase lc("OPE");
    UUID id1 = UUID::generate();
    UUID id2 = UUID::generate();
    UUID id3 = UUID::generate();

    lc.addEntry({id1, 1.0});
    lc.addEntry({id2, 1.0});
    lc.addEntry({id3, 1.0});

    EXPECT_EQ(lc.entries().size(), 3u);
}

TEST(LoadCaseTest, RemoveEntry) {
    LoadCase lc("WN");
    UUID id1 = UUID::generate();
    UUID id2 = UUID::generate();

    lc.addEntry({id1, 1.0});
    lc.addEntry({id2, 1.0});

    bool removed = lc.removeEntry(id1);
    EXPECT_TRUE(removed);
    ASSERT_EQ(lc.entries().size(), 1u);
    EXPECT_EQ(lc.entries()[0].loadId, id2);
}

TEST(LoadCaseTest, RemoveNonExistentEntry) {
    LoadCase lc("EQ");
    UUID id = UUID::generate();

    bool removed = lc.removeEntry(id);
    EXPECT_FALSE(removed);
    EXPECT_TRUE(lc.entries().empty());
}

TEST(LoadCaseTest, ChangedSignalFired) {
    LoadCase lc("W");
    int signalCount = 0;
    lc.changed.connect([&] { ++signalCount; });

    UUID id = UUID::generate();
    lc.addEntry({id, 1.0});
    EXPECT_EQ(signalCount, 1);

    lc.removeEntry(id);
    EXPECT_EQ(signalCount, 2);

    // 重复 remove 不触发信号
    lc.removeEntry(id);
    EXPECT_EQ(signalCount, 2);
}

TEST(LoadCaseTest, UUIDUnique) {
    LoadCase lc1("A");
    LoadCase lc2("B");
    EXPECT_NE(lc1.id(), lc2.id());
}

// ============================================================
// CaseEntry / LoadCombination 基本操作
// ============================================================

TEST(LoadCombinationTest, ConstructDefault) {
    LoadCombination combo("SUS", StressCategory::Sustained, CombineMethod::Algebraic);
    EXPECT_EQ(combo.name(), "SUS");
    EXPECT_EQ(combo.category(), StressCategory::Sustained);
    EXPECT_EQ(combo.method(), CombineMethod::Algebraic);
    EXPECT_TRUE(combo.caseEntries().empty());
    EXPECT_FALSE(combo.id().isNull());
}

TEST(LoadCombinationTest, SetCategoryAndMethod) {
    LoadCombination combo("OCC", StressCategory::Sustained, CombineMethod::Algebraic);

    combo.setCategory(StressCategory::Occasional);
    combo.setMethod(CombineMethod::SRSS);

    EXPECT_EQ(combo.category(), StressCategory::Occasional);
    EXPECT_EQ(combo.method(), CombineMethod::SRSS);
}

TEST(LoadCombinationTest, AddCaseEntry) {
    LoadCombination combo("EXP", StressCategory::Expansion, CombineMethod::Algebraic);
    UUID caseId = UUID::generate();

    combo.addCaseEntry({caseId, 1.0});
    ASSERT_EQ(combo.caseEntries().size(), 1u);
    EXPECT_EQ(combo.caseEntries()[0].caseId, caseId);
    EXPECT_DOUBLE_EQ(combo.caseEntries()[0].factor, 1.0);
}

TEST(LoadCombinationTest, AddCaseEntryDuplicateIgnored) {
    LoadCombination combo("SUS", StressCategory::Sustained, CombineMethod::Algebraic);
    UUID caseId = UUID::generate();

    combo.addCaseEntry({caseId, 1.0});
    combo.addCaseEntry({caseId, 2.0}); // 重复，忽略

    EXPECT_EQ(combo.caseEntries().size(), 1u);
    EXPECT_DOUBLE_EQ(combo.caseEntries()[0].factor, 1.0);
}

TEST(LoadCombinationTest, RemoveCaseEntry) {
    LoadCombination combo("OPE", StressCategory::Operating, CombineMethod::Algebraic);
    UUID id1 = UUID::generate();
    UUID id2 = UUID::generate();

    combo.addCaseEntry({id1, 1.0});
    combo.addCaseEntry({id2, 1.0});

    bool removed = combo.removeCaseEntry(id1);
    EXPECT_TRUE(removed);
    ASSERT_EQ(combo.caseEntries().size(), 1u);
    EXPECT_EQ(combo.caseEntries()[0].caseId, id2);
}

TEST(LoadCombinationTest, RemoveNonExistentCaseEntry) {
    LoadCombination combo("HYD", StressCategory::Hydrotest, CombineMethod::Algebraic);
    UUID id = UUID::generate();

    bool removed = combo.removeCaseEntry(id);
    EXPECT_FALSE(removed);
}

TEST(LoadCombinationTest, ChangedSignalFired) {
    LoadCombination combo("SUS", StressCategory::Sustained, CombineMethod::Algebraic);
    int signalCount = 0;
    combo.changed.connect([&] { ++signalCount; });

    UUID caseId = UUID::generate();
    combo.addCaseEntry({caseId, 1.0});
    EXPECT_EQ(signalCount, 1);

    combo.setCategory(StressCategory::Occasional);
    EXPECT_EQ(signalCount, 2);

    combo.setMethod(CombineMethod::SRSS);
    EXPECT_EQ(signalCount, 3);

    combo.removeCaseEntry(caseId);
    EXPECT_EQ(signalCount, 4);

    // 无信号：值未变化
    combo.setCategory(StressCategory::Occasional);
    EXPECT_EQ(signalCount, 4);
}

// ============================================================
// B31.3 典型场景验证
// ============================================================

TEST(B31_3_TypicalConfig, SUSConfiguration) {
    // SUS (Sustained) = W×1.0 + P1×1.0
    UUID caseW  = UUID::generate();
    UUID caseP1 = UUID::generate();

    LoadCase sus_case("SUS");
    LoadEntry w_entry{caseW, 1.0};
    LoadEntry p_entry{caseP1, 1.0};
    sus_case.addEntry(w_entry);
    sus_case.addEntry(p_entry);

    ASSERT_EQ(sus_case.entries().size(), 2u);

    LoadCombination sus_combo("SUS_OPE", StressCategory::Sustained, CombineMethod::Algebraic);
    UUID susId = sus_case.id();
    sus_combo.addCaseEntry({susId, 1.0});

    EXPECT_EQ(sus_combo.category(), StressCategory::Sustained);
    EXPECT_EQ(sus_combo.method(), CombineMethod::Algebraic);
    EXPECT_EQ(sus_combo.caseEntries().size(), 1u);
}

TEST(B31_3_TypicalConfig, EXPConfiguration) {
    // EXP (Expansion) = T1×1.0
    UUID caseT1 = UUID::generate();

    LoadCase exp_case("EXP");
    exp_case.addEntry({caseT1, 1.0});

    LoadCombination exp_combo("EXP_COMBO", StressCategory::Expansion, CombineMethod::Algebraic);
    exp_combo.addCaseEntry({exp_case.id(), 1.0});

    EXPECT_EQ(exp_combo.category(), StressCategory::Expansion);
    EXPECT_EQ(exp_combo.caseEntries().size(), 1u);
}

TEST(B31_3_TypicalConfig, OPEConfiguration) {
    // OPE (Operating) = W×1.0 + T1×1.0 + P1×1.0
    UUID caseW  = UUID::generate();
    UUID caseT1 = UUID::generate();
    UUID caseP1 = UUID::generate();

    LoadCase ope_case("OPE");
    ope_case.addEntry({caseW,  1.0});
    ope_case.addEntry({caseT1, 1.0});
    ope_case.addEntry({caseP1, 1.0});

    ASSERT_EQ(ope_case.entries().size(), 3u);

    LoadCombination ope_combo("OPE_COMBO", StressCategory::Operating, CombineMethod::Algebraic);
    ope_combo.addCaseEntry({ope_case.id(), 1.0});

    EXPECT_EQ(ope_combo.category(), StressCategory::Operating);
}

TEST(B31_3_TypicalConfig, OCC_SRSSConfiguration) {
    // OCC2 (Occasional+SRSS) = W×1.0 + P1×1.0 + EQ×1.0 （SRSS）
    UUID caseW  = UUID::generate();
    UUID caseP1 = UUID::generate();
    UUID caseEQ = UUID::generate();

    LoadCase occ_case("OCC_EQ");
    occ_case.addEntry({caseW,  1.0});
    occ_case.addEntry({caseP1, 1.0});
    occ_case.addEntry({caseEQ, 1.0});

    LoadCombination occ_combo("OCC_EQ_COMBO", StressCategory::Occasional, CombineMethod::SRSS);
    occ_combo.addCaseEntry({occ_case.id(), 1.0});

    EXPECT_EQ(occ_combo.method(), CombineMethod::SRSS);
    EXPECT_EQ(occ_combo.category(), StressCategory::Occasional);
}

TEST(B31_3_TypicalConfig, AllCombineMethods) {
    // 验证所有 CombineMethod 枚举值均可设置
    LoadCombination combo("test", StressCategory::Sustained, CombineMethod::Algebraic);

    combo.setMethod(CombineMethod::Algebraic);
    EXPECT_EQ(combo.method(), CombineMethod::Algebraic);

    combo.setMethod(CombineMethod::Absolute);
    EXPECT_EQ(combo.method(), CombineMethod::Absolute);

    combo.setMethod(CombineMethod::SRSS);
    EXPECT_EQ(combo.method(), CombineMethod::SRSS);

    combo.setMethod(CombineMethod::Envelope);
    EXPECT_EQ(combo.method(), CombineMethod::Envelope);
}

TEST(B31_3_TypicalConfig, AllStressCategories) {
    // 验证所有 StressCategory 枚举值均可设置
    LoadCombination combo("test", StressCategory::Sustained, CombineMethod::Algebraic);

    combo.setCategory(StressCategory::Sustained);
    EXPECT_EQ(combo.category(), StressCategory::Sustained);

    combo.setCategory(StressCategory::Expansion);
    EXPECT_EQ(combo.category(), StressCategory::Expansion);

    combo.setCategory(StressCategory::Occasional);
    EXPECT_EQ(combo.category(), StressCategory::Occasional);

    combo.setCategory(StressCategory::Operating);
    EXPECT_EQ(combo.category(), StressCategory::Operating);

    combo.setCategory(StressCategory::Hydrotest);
    EXPECT_EQ(combo.category(), StressCategory::Hydrotest);
}

// ============================================================
// DAG 依赖关系验证：LoadCombination → LoadCase → Load
// ============================================================

TEST(DagRelationTest, CombinationReferencesCase) {
    // LoadCombination 通过 caseId 引用 LoadCase
    LoadCase lc("W");
    UUID lcId = lc.id();

    LoadCombination combo("SUS", StressCategory::Sustained, CombineMethod::Algebraic);
    combo.addCaseEntry({lcId, 1.0});

    ASSERT_EQ(combo.caseEntries().size(), 1u);
    EXPECT_EQ(combo.caseEntries()[0].caseId, lcId);
}

TEST(DagRelationTest, CaseReferencesLoad) {
    // LoadCase 通过 loadId 引用 Load（UUID 模拟）
    UUID loadId = UUID::generate();
    LoadCase lc("P1");
    lc.addEntry({loadId, 1.0});

    ASSERT_EQ(lc.entries().size(), 1u);
    EXPECT_EQ(lc.entries()[0].loadId, loadId);
}

TEST(DagRelationTest, ThreeLevelDag) {
    // 三层 DAG: Combination → Case → Load
    UUID loadId = UUID::generate();

    LoadCase lc("T1");
    lc.addEntry({loadId, 1.0});

    LoadCombination combo("EXP", StressCategory::Expansion, CombineMethod::Algebraic);
    combo.addCaseEntry({lc.id(), 1.0});

    // 验证引用链
    EXPECT_EQ(combo.caseEntries()[0].caseId, lc.id());
    EXPECT_EQ(lc.entries()[0].loadId, loadId);
}
