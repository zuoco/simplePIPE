#include <gtest/gtest.h>

#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/TransactionManager.h"
#include "engine/RecomputeEngine.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Segment.h"

#include <gp_Pnt.hxx>
#include <memory>

// ============================================================
// Document 测试
// ============================================================

TEST(Document, AddAndFindObject) {
    app::Document doc;
    auto spec = std::make_shared<model::PipeSpec>("Spec1");
    foundation::UUID id = spec->id();

    doc.addObject(spec);
    EXPECT_EQ(doc.objectCount(), 1u);
    EXPECT_NE(doc.findObject(id), nullptr);
    EXPECT_EQ(doc.findObject(id)->name(), "Spec1");
}

TEST(Document, RemoveObject) {
    app::Document doc;
    auto spec = std::make_shared<model::PipeSpec>("S");
    foundation::UUID id = spec->id();
    doc.addObject(spec);

    EXPECT_TRUE(doc.removeObject(id));
    EXPECT_EQ(doc.objectCount(), 0u);
    EXPECT_EQ(doc.findObject(id), nullptr);
}

TEST(Document, FindNonExistent) {
    app::Document doc;
    foundation::UUID fakeId = foundation::UUID::generate();
    EXPECT_EQ(doc.findObject(fakeId), nullptr);
}

TEST(Document, FindByType) {
    app::Document doc;
    doc.addObject(std::make_shared<model::PipeSpec>("Spec1"));
    doc.addObject(std::make_shared<model::PipeSpec>("Spec2"));
    doc.addObject(std::make_shared<model::PipePoint>("PP1"));

    auto specs = doc.findByType<model::PipeSpec>();
    auto points = doc.findByType<model::PipePoint>();
    EXPECT_EQ(specs.size(), 2u);
    EXPECT_EQ(points.size(), 1u);
}

TEST(Document, AllPipePoints) {
    app::Document doc;
    doc.addObject(std::make_shared<model::PipePoint>("P1"));
    doc.addObject(std::make_shared<model::PipePoint>("P2"));
    doc.addObject(std::make_shared<model::PipeSpec>("Spec"));
    EXPECT_EQ(doc.allPipePoints().size(), 2u);
}

TEST(Document, AllSegments) {
    app::Document doc;
    doc.addObject(std::make_shared<model::Segment>("Seg1"));
    doc.addObject(std::make_shared<model::PipePoint>("P1"));
    EXPECT_EQ(doc.allSegments().size(), 1u);
}

TEST(Document, AllPipeSpecs) {
    app::Document doc;
    doc.addObject(std::make_shared<model::PipeSpec>("S1"));
    doc.addObject(std::make_shared<model::PipeSpec>("S2"));
    EXPECT_EQ(doc.allPipeSpecs().size(), 2u);
}

TEST(Document, ForEach) {
    app::Document doc;
    doc.addObject(std::make_shared<model::PipeSpec>("A"));
    doc.addObject(std::make_shared<model::PipeSpec>("B"));

    int count = 0;
    doc.forEach([&](model::DocumentObject&) { ++count; });
    EXPECT_EQ(count, 2);
}

// ============================================================
// DependencyGraph 测试
// ============================================================

TEST(DependencyGraph, MarkDirtyPropagates) {
    app::DependencyGraph graph;
    foundation::UUID spec = foundation::UUID::generate();
    foundation::UUID pp1  = foundation::UUID::generate();
    foundation::UUID pp2  = foundation::UUID::generate();

    // pp1 和 pp2 依赖 spec
    graph.addDependency(pp1, spec);
    graph.addDependency(pp2, spec);

    graph.markDirty(spec);

    EXPECT_TRUE(graph.isDirty(spec));
    EXPECT_TRUE(graph.isDirty(pp1));
    EXPECT_TRUE(graph.isDirty(pp2));
}

TEST(DependencyGraph, ClearDirty) {
    app::DependencyGraph graph;
    foundation::UUID id = foundation::UUID::generate();
    graph.markDirty(id);
    EXPECT_TRUE(graph.isDirty(id));
    graph.clearDirty();
    EXPECT_FALSE(graph.isDirty(id));
}

TEST(DependencyGraph, CollectDirtyOrdering) {
    app::DependencyGraph graph;
    foundation::UUID spec = foundation::UUID::generate();
    foundation::UUID pp   = foundation::UUID::generate();

    // pp 依赖 spec → spec 先
    graph.addDependency(pp, spec);
    graph.markDirty(spec);
    graph.markDirty(pp);

    auto dirty = graph.collectDirty();
    ASSERT_EQ(dirty.size(), 2u);
    // spec 应在 pp 之前
    bool specFirst = (dirty[0] == spec);
    EXPECT_TRUE(specFirst);
}

TEST(DependencyGraph, RemoveDependency) {
    app::DependencyGraph graph;
    foundation::UUID a = foundation::UUID::generate();
    foundation::UUID b = foundation::UUID::generate();

    graph.addDependency(b, a);
    graph.removeDependency(b, a);
    graph.markDirty(a);

    EXPECT_TRUE(graph.isDirty(a));
    EXPECT_FALSE(graph.isDirty(b)); // 依赖已移除
}

TEST(DependencyGraph, RemoveObject) {
    app::DependencyGraph graph;
    foundation::UUID a = foundation::UUID::generate();
    foundation::UUID b = foundation::UUID::generate();

    graph.addDependency(b, a);
    graph.removeObject(a);
    graph.markDirty(a);

    // b 不再依赖 a，不应被标脏
    EXPECT_FALSE(graph.isDirty(b));
}

TEST(DependencyGraph, ChainPropagation) {
    app::DependencyGraph graph;
    foundation::UUID a = foundation::UUID::generate();
    foundation::UUID b = foundation::UUID::generate();
    foundation::UUID c = foundation::UUID::generate();

    // c 依赖 b 依赖 a
    graph.addDependency(b, a);
    graph.addDependency(c, b);

    graph.markDirty(a);
    EXPECT_TRUE(graph.isDirty(a));
    EXPECT_TRUE(graph.isDirty(b));
    EXPECT_TRUE(graph.isDirty(c));
}

// ============================================================
// TransactionManager 测试
// ============================================================

class TxnTest : public ::testing::Test {
protected:
    app::Document        doc;
    app::DependencyGraph graph;
    std::unique_ptr<app::TransactionManager> tm;

    std::vector<foundation::UUID> lastDirty;
    int recomputeCount = 0;

    void SetUp() override {
        tm = std::make_unique<app::TransactionManager>(doc, graph);
        tm->setRecomputeCallback([this](const std::vector<foundation::UUID>& dirty) {
            lastDirty = dirty;
            ++recomputeCount;
        });
    }

    // 向 doc 加入一个 PipeSpec 并返回其 UUID
    foundation::UUID addSpec(const std::string& name, double od) {
        auto spec = std::make_shared<model::PipeSpec>(name);
        spec->setOd(od);
        foundation::UUID id = spec->id();
        doc.addObject(spec);
        return id;
    }
};

TEST_F(TxnTest, CommitAppliesChange) {
    foundation::UUID specId = addSpec("S", 100.0);

    tm->open("修改OD");
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    ASSERT_NE(obj, nullptr);
    double oldOD = obj->od();
    obj->setOd(219.1);
    tm->recordChange(specId, "OD", oldOD, 219.1);
    tm->commit();

    EXPECT_DOUBLE_EQ(obj->od(), 219.1);
    EXPECT_EQ(recomputeCount, 1);
    EXPECT_FALSE(tm->isOpen());
}

TEST_F(TxnTest, AbortRollsBack) {
    foundation::UUID specId = addSpec("S", 100.0);
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    ASSERT_NE(obj, nullptr);

    tm->open("修改OD");
    double oldOD = obj->od();
    obj->setOd(500.0);
    tm->recordChange(specId, "OD", oldOD, 500.0);
    tm->abort();

    // abort 应将属性恢复为 oldOD
    EXPECT_DOUBLE_EQ(obj->od(), 100.0);
    EXPECT_EQ(recomputeCount, 0); // abort 不触发重算
    EXPECT_FALSE(tm->isOpen());
}

TEST_F(TxnTest, UndoRestoresPreviousValue) {
    foundation::UUID specId = addSpec("S", 100.0);
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    ASSERT_NE(obj, nullptr);

    tm->open("修改OD");
    obj->setOd(200.0);
    tm->recordChange(specId, "OD", 100.0, 200.0);
    tm->commit();

    EXPECT_DOUBLE_EQ(obj->od(), 200.0);

    tm->undo();
    EXPECT_DOUBLE_EQ(obj->od(), 100.0);
    EXPECT_EQ(recomputeCount, 2); // commit + undo 各一次
}

TEST_F(TxnTest, RedoRestoresNewValue) {
    foundation::UUID specId = addSpec("S", 100.0);
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    ASSERT_NE(obj, nullptr);

    tm->open("修改OD");
    obj->setOd(200.0);
    tm->recordChange(specId, "OD", 100.0, 200.0);
    tm->commit();

    tm->undo();
    EXPECT_DOUBLE_EQ(obj->od(), 100.0);

    tm->redo();
    EXPECT_DOUBLE_EQ(obj->od(), 200.0);
    EXPECT_EQ(recomputeCount, 3); // commit + undo + redo
}

TEST_F(TxnTest, MultipleUndo) {
    foundation::UUID specId = addSpec("S", 100.0);
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    ASSERT_NE(obj, nullptr);

    tm->open("Step1");
    obj->setOd(200.0);
    tm->recordChange(specId, "OD", 100.0, 200.0);
    tm->commit();

    tm->open("Step2");
    obj->setOd(300.0);
    tm->recordChange(specId, "OD", 200.0, 300.0);
    tm->commit();

    EXPECT_EQ(tm->undoStackSize(), 2u);

    tm->undo(); // Step2 → 200.0
    EXPECT_DOUBLE_EQ(obj->od(), 200.0);

    tm->undo(); // Step1 → 100.0
    EXPECT_DOUBLE_EQ(obj->od(), 100.0);

    EXPECT_FALSE(tm->canUndo());
}

TEST_F(TxnTest, NewCommitClearsRedo) {
    foundation::UUID specId = addSpec("S", 100.0);
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    ASSERT_NE(obj, nullptr);

    tm->open("Step1");
    obj->setOd(200.0);
    tm->recordChange(specId, "OD", 100.0, 200.0);
    tm->commit();

    tm->undo();
    EXPECT_EQ(tm->redoStackSize(), 1u);

    // 新提交后 redo 栈应清空
    tm->open("Step2");
    obj->setOd(999.0);
    tm->recordChange(specId, "OD", 100.0, 999.0);
    tm->commit();

    EXPECT_EQ(tm->redoStackSize(), 0u);
}

TEST_F(TxnTest, CanUndoRedo) {
    EXPECT_FALSE(tm->canUndo());
    EXPECT_FALSE(tm->canRedo());

    foundation::UUID specId = addSpec("S", 1.0);
    auto* obj = dynamic_cast<model::PipeSpec*>(doc.findObject(specId));
    tm->open("x");
    obj->setOd(2.0);
    tm->recordChange(specId, "OD", 1.0, 2.0);
    tm->commit();

    EXPECT_TRUE(tm->canUndo());
    EXPECT_FALSE(tm->canRedo());

    tm->undo();
    EXPECT_FALSE(tm->canUndo());
    EXPECT_TRUE(tm->canRedo());
}

TEST_F(TxnTest, DependencyPropagation) {
    // PipeSpec → PipePoint 依赖关系
    auto spec = std::make_shared<model::PipeSpec>("Spec");
    auto pp   = std::make_shared<model::PipePoint>("PP1");
    pp->setPipeSpec(spec);

    foundation::UUID specId = spec->id();
    foundation::UUID ppId   = pp->id();

    doc.addObject(spec);
    doc.addObject(pp);

    // 注册依赖：pp 依赖 spec
    graph.addDependency(ppId, specId);

    tm->open("修改Spec.OD");
    spec->setOd(219.1);
    tm->recordChange(specId, "OD", 0.0, 219.1);
    tm->commit();

    // spec 和 pp 都应在 dirty 列表中
    ASSERT_FALSE(lastDirty.empty());
    bool hasSpec = false, hasPP = false;
    for (const auto& id : lastDirty) {
        if (id == specId) hasSpec = true;
        if (id == ppId)   hasPP  = true;
    }
    EXPECT_TRUE(hasSpec);
    EXPECT_TRUE(hasPP);
}

// ============================================================
// RecomputeEngine 测试
// ============================================================

TEST(RecomputeEngine, RecomputeAll_NoSegments) {
    app::Document        doc;
    app::DependencyGraph graph;
    engine::RecomputeEngine engine(doc, graph);

    // 文档中没有管点，不应崩溃
    EXPECT_NO_THROW(engine.recomputeAll());
}

TEST(RecomputeEngine, RecomputeWithPoints) {
    app::Document        doc;
    app::DependencyGraph graph;
    engine::RecomputeEngine eng(doc, graph);

    auto spec = std::make_shared<model::PipeSpec>("Spec");
    spec->setOd(114.3);
    spec->setWallThickness(6.0);

    auto pp1 = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run,
                                                   gp_Pnt(0, 0, 0));
    auto pp2 = std::make_shared<model::PipePoint>("P2", model::PipePointType::Run,
                                                   gp_Pnt(1000, 0, 0));
    pp1->setPipeSpec(spec);
    pp2->setPipeSpec(spec);

    auto seg = std::make_shared<model::Segment>("Seg");
    seg->addPoint(pp1);
    seg->addPoint(pp2);

    foundation::UUID specId = spec->id();
    foundation::UUID pp1Id  = pp1->id();
    foundation::UUID pp2Id  = pp2->id();
    foundation::UUID segId  = seg->id();

    doc.addObject(spec);
    doc.addObject(pp1);
    doc.addObject(pp2);
    doc.addObject(seg);

    // 注册依赖
    graph.addDependency(pp1Id, specId);
    graph.addDependency(pp2Id, specId);

    // 全量重算不应崩溃
    EXPECT_NO_THROW(eng.recomputeAll());
}

TEST(RecomputeEngine, RecomputeDirty) {
    app::Document        doc;
    app::DependencyGraph graph;
    engine::RecomputeEngine eng(doc, graph);

    auto spec = std::make_shared<model::PipeSpec>("Spec");
    spec->setOd(114.3);
    spec->setWallThickness(6.0);

    auto pp1 = std::make_shared<model::PipePoint>("P1", model::PipePointType::Run,
                                                   gp_Pnt(0, 0, 0));
    auto pp2 = std::make_shared<model::PipePoint>("P2", model::PipePointType::Run,
                                                   gp_Pnt(500, 0, 0));
    pp1->setPipeSpec(spec);
    pp2->setPipeSpec(spec);

    auto seg = std::make_shared<model::Segment>("Seg");
    seg->addPoint(pp1);
    seg->addPoint(pp2);

    foundation::UUID pp1Id = pp1->id();

    doc.addObject(spec);
    doc.addObject(pp1);
    doc.addObject(pp2);
    doc.addObject(seg);

    // 只重算 pp1
    EXPECT_NO_THROW(eng.recompute({pp1Id}));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
