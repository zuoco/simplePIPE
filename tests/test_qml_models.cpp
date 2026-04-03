// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "app/DependencyGraph.h"
#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/TransactionManager.h"
#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Route.h"
#include "model/Segment.h"
#include "ui/PipePointTableModel.h"
#include "ui/PipeSpecModel.h"
#include "ui/PropertyModel.h"
#include "ui/SegmentTreeModel.h"

#include <QCoreApplication>

#include <memory>

static QCoreApplication* app_ = nullptr;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    app_ = new QCoreApplication(argc, argv);
    const int result = RUN_ALL_TESTS();
    delete app_;
    return result;
}

namespace {

struct Fixture {
    app::Document document;
    app::DependencyGraph graph;
    app::TransactionManager transactionManager{document, graph};
    app::SelectionManager selectionManager;

    std::shared_ptr<model::PipeSpec> spec;
    std::shared_ptr<model::Route> route;
    std::shared_ptr<model::Segment> segment;
    std::shared_ptr<model::PipePoint> runPoint;
    std::shared_ptr<model::PipePoint> bendPoint;
    std::shared_ptr<model::PipePoint> bendHelperPoint;

    Fixture()
    {
        spec = std::make_shared<model::PipeSpec>("Spec-6inch");
        spec->setOd(168.3);
        spec->setWallThickness(7.11);
        spec->setMaterial("A106-B");

        route = std::make_shared<model::Route>("Route-A");
        segment = std::make_shared<model::Segment>("Segment-A");

        runPoint = std::make_shared<model::PipePoint>("A00", model::PipePointType::Run, gp_Pnt(0.0, 0.0, 0.0));
        runPoint->setPipeSpec(spec);

        bendPoint = std::make_shared<model::PipePoint>("A01", model::PipePointType::Bend, gp_Pnt(1000.0, 0.0, 0.0));
        bendPoint->setPipeSpec(spec);
        bendPoint->setParam("bendMultiplier", 1.5);

        bendHelperPoint = std::make_shared<model::PipePoint>("A01N", model::PipePointType::Bend, gp_Pnt(950.0, 0.0, 0.0));
        bendHelperPoint->setPipeSpec(spec);
        bendHelperPoint->setParam("bendMultiplier", 1.5);

        segment->addPoint(runPoint);
        segment->addPoint(bendPoint);
        segment->addPoint(bendHelperPoint);
        route->addSegment(segment);

        document.addObject(spec);
        document.addObject(route);
        document.addObject(segment);
        document.addObject(runPoint);
        document.addObject(bendPoint);
        document.addObject(bendHelperPoint);
    }
};

} // namespace

TEST(PipePointTableModelTest, ShowsPipePointsAndEditableTransaction)
{
    Fixture f;
    ui::PipePointTableModel model(f.document, f.transactionManager, f.selectionManager);

    ASSERT_EQ(model.rowCount(), 3);
    const QModelIndex xCell = model.index(0, ui::PipePointTableModel::XColumn);
    ASSERT_TRUE(xCell.isValid());

    EXPECT_TRUE(model.setData(xCell, 123.5, Qt::EditRole));
    EXPECT_DOUBLE_EQ(f.runPoint->position().X(), 123.5);
    EXPECT_TRUE(f.transactionManager.canUndo());
}

TEST(PipePointTableModelTest, BendHelperRowIsReadOnlyWithGrayBackground)
{
    Fixture f;
    ui::PipePointTableModel model(f.document, f.transactionManager, f.selectionManager);

    const QModelIndex helperName = model.index(2, ui::PipePointTableModel::NameColumn);
    ASSERT_TRUE(helperName.isValid());

    const Qt::ItemFlags flags = model.flags(helperName);
    EXPECT_FALSE(flags.testFlag(Qt::ItemIsEditable));
    EXPECT_TRUE(model.data(helperName, Qt::BackgroundRole).isValid());
}

TEST(SegmentTreeModelTest, BuildsRouteSegmentPipePointHierarchyAndSelection)
{
    Fixture f;
    ui::SegmentTreeModel model(f.document, f.selectionManager);

    ASSERT_EQ(model.rowCount(), 1);

    const QModelIndex routeNode = model.index(0, 0);
    ASSERT_TRUE(routeNode.isValid());
    EXPECT_EQ(model.data(routeNode, Qt::DisplayRole).toString().toStdString(), "Route-A");

    ASSERT_EQ(model.rowCount(routeNode), 1);
    const QModelIndex segmentNode = model.index(0, 0, routeNode);
    ASSERT_TRUE(segmentNode.isValid());
    EXPECT_EQ(model.data(segmentNode, Qt::DisplayRole).toString().toStdString(), "Segment-A");

    ASSERT_EQ(model.rowCount(segmentNode), 3);
    const QModelIndex pointNode = model.index(1, 0, segmentNode);
    const QString uuid = model.data(pointNode, ui::SegmentTreeModel::UuidRole).toString();

    EXPECT_TRUE(model.selectNodeByUuid(uuid));
    ASSERT_EQ(f.selectionManager.size(), 1u);
    EXPECT_EQ(f.selectionManager.selected().front(), f.bendPoint->id());
}

TEST(PropertyModelTest, ReflectsSelectedObjectTypeSpecificRows)
{
    Fixture f;
    ui::PropertyModel model(f.document, f.selectionManager);

    ASSERT_EQ(model.rowCount(), 0);
    ASSERT_TRUE(f.selectionManager.select(f.bendPoint->id()));

    EXPECT_GT(model.rowCount(), 0);

    bool hasBendMultiplier = false;
    for (int i = 0; i < model.rowCount(); ++i) {
        const QModelIndex idx = model.index(i, 0);
        const QString key = model.data(idx, ui::PropertyModel::KeyRole).toString();
        if (key == "bendMultiplier") {
            hasBendMultiplier = true;
            break;
        }
    }

    EXPECT_TRUE(hasBendMultiplier);
}

TEST(PipeSpecModelTest, EditFieldThroughTransaction)
{
    Fixture f;
    ui::PipeSpecModel model(f.document, f.transactionManager);

    ASSERT_EQ(model.rowCount(), 1);

    const QModelIndex odCell = model.index(0, ui::PipeSpecModel::OdColumn);
    ASSERT_TRUE(odCell.isValid());

    EXPECT_TRUE(model.setData(odCell, 219.1, Qt::EditRole));
    EXPECT_DOUBLE_EQ(f.spec->od(), 219.1);
    EXPECT_TRUE(f.transactionManager.canUndo());
}
