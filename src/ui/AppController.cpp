// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/AppController.h"

#include "app/Application.h"
#include "command/InsertComponentCommand.h"
#include "command/DeletePipePointCommand.h"
#include "command/MacroCommand.h"
#include "model/PipePoint.h"
#include "model/Route.h"
#include "model/Segment.h"
#include "ui/PipePointTableModel.h"
#include "ui/PipeSpecModel.h"
#include "ui/PropertyModel.h"
#include "ui/SegmentTreeModel.h"
#include "ui/UuidUtil.h"

#include <cmath>

namespace ui {

static QString pipePointTypeName(model::PipePointType type) {
    switch (type) {
        case model::PipePointType::Run:       return QStringLiteral("Run");
        case model::PipePointType::Bend:      return QStringLiteral("Bend");
        case model::PipePointType::Reducer:   return QStringLiteral("Reducer");
        case model::PipePointType::Tee:       return QStringLiteral("Tee");
        case model::PipePointType::Valve:     return QStringLiteral("Valve");
        case model::PipePointType::FlexJoint: return QStringLiteral("FlexJoint");
    }
    return QStringLiteral("Unknown");
}

AppController::AppController(app::Document& document,
                             command::CommandStack& commandStack,
                             app::SelectionManager& selectionManager,
                             QObject* parent)
    : QObject(parent)
    , document_(document)
    , commandStack_(commandStack)
    , selectionManager_(selectionManager)
    , pipePointTableModel_(std::make_unique<PipePointTableModel>(document_, commandStack_, selectionManager_))
    , segmentTreeModel_(std::make_unique<SegmentTreeModel>(document_, selectionManager_))
    , propertyModel_(std::make_unique<PropertyModel>(document_, selectionManager_))
    , pipeSpecModel_(std::make_unique<PipeSpecModel>(document_, commandStack_))
{
    wireCallbacks();
}

AppController::~AppController() = default;

QString AppController::documentName() const
{
    return QString::fromStdString(document_.name());
}

void AppController::setDocumentName(const QString& name)
{
    const std::string next = name.toStdString();
    if (document_.name() == next) {
        return;
    }

    document_.setName(next);
    emit documentNameChanged();
}

int AppController::selectedCount() const
{
    return static_cast<int>(selectionManager_.size());
}

QStringList AppController::selectedUuids() const
{
    QStringList ids;
    ids.reserve(static_cast<qsizetype>(selectionManager_.selected().size()));

    for (const auto& id : selectionManager_.selected()) {
        ids.push_back(QString::fromStdString(id.toString()));
    }
    return ids;
}

bool AppController::canUndo() const
{
    return commandStack_.canUndo();
}

bool AppController::canRedo() const
{
    return commandStack_.canRedo();
}

QObject* AppController::pipePointTableModel() const
{
    return pipePointTableModel_.get();
}

QObject* AppController::segmentTreeModel() const
{
    return segmentTreeModel_.get();
}

QObject* AppController::propertyModel() const
{
    return propertyModel_.get();
}

QObject* AppController::pipeSpecModel() const
{
    return pipeSpecModel_.get();
}

QString AppController::selectionInfo() const
{
    if (selectionManager_.selected().empty()) {
        auto points = document_.allPipePoints();
        int pointCount = static_cast<int>(points.size());
        return QString("管点: %1").arg(pointCount);
    }

    const auto& firstId = selectionManager_.selected().front();
    auto* obj = document_.findObject(firstId);
    if (!obj) {
        return QString("已选: %1").arg(static_cast<int>(selectionManager_.size()));
    }

    auto* pp = dynamic_cast<model::PipePoint*>(obj);
    if (pp) {
        const auto& pos = pp->position();
        return QString("%1 %2 (%3, %4, %5)")
            .arg(QString::fromStdString(pp->name()))
            .arg(pipePointTypeName(pp->type()))
            .arg(pos.X(), 0, 'f', 1)
            .arg(pos.Y(), 0, 'f', 1)
            .arg(pos.Z(), 0, 'f', 1);
    }

    return QString("%1").arg(QString::fromStdString(obj->name()));
}

QString AppController::mouseCoord() const
{
    return QString("(%1, %2, %3)")
        .arg(mouseX_, 0, 'f', 1)
        .arg(mouseY_, 0, 'f', 1)
        .arg(mouseZ_, 0, 'f', 1);
}

double AppController::zoomLevel() const
{
    return zoomLevel_;
}

void AppController::setZoomLevel(double level)
{
    if (std::abs(zoomLevel_ - level) < 0.01) {
        return;
    }
    zoomLevel_ = level;
    emit zoomLevelChanged();
}

bool AppController::hasSelection() const
{
    return !selectionManager_.selected().empty();
}

void AppController::clearSelection()
{
    selectionManager_.clear();
}

void AppController::undo()
{
    if (!commandStack_.canUndo()) {
        return;
    }
    auto ctx = app::Application::instance().createCommandContext();
    commandStack_.undo(ctx);
    emit transactionStateChanged();
}

void AppController::redo()
{
    if (!commandStack_.canRedo()) {
        return;
    }
    auto ctx = app::Application::instance().createCommandContext();
    commandStack_.redo(ctx);
    emit transactionStateChanged();
}

void AppController::updateMouseCoord(double x, double y, double z)
{
    mouseX_ = x;
    mouseY_ = y;
    mouseZ_ = z;
    emit mouseCoordChanged();
}

void AppController::deleteSelected()
{
    if (selectionManager_.selected().empty()) {
        return;
    }

    auto ctx = app::Application::instance().createCommandContext();
    const auto& selected = selectionManager_.selected();

    if (selected.size() == 1) {
        // 单点删除
        auto cmd = command::DeletePipePointCommand::create(selected.front());
        commandStack_.execute(std::move(cmd), ctx);
    } else {
        // 多点删除：包装为 MacroCommand
        auto macro = std::make_unique<command::MacroCommand>("删除选中对象");
        for (const auto& id : selected) {
            macro->addCommand(command::DeletePipePointCommand::create(id));
        }
        commandStack_.execute(std::move(macro), ctx);
    }

    selectionManager_.clear();
    emit transactionStateChanged();
}

void AppController::selectByUuid(const QString& uuid)
{
    selectionManager_.clear();
    foundation::UUID id;
    if (uuidFromString(uuid.toStdString(), id)) {
        selectionManager_.select(id);
    }
}

void AppController::multiSelect(const QStringList& uuids, bool append)
{
    if (!append) {
        selectionManager_.clear();
    }
    for (const auto& u : uuids) {
        foundation::UUID id;
        if (uuidFromString(u.toStdString(), id)) {
            selectionManager_.select(id);
        }
    }
}

void AppController::insertComponent(const QString& componentType)
{
    const std::string compType = componentType.toStdString();

    // 检查是否为已支持的管点组件类型
    try {
        command::InsertComponentCommand::mapComponentType(compType);
    } catch (...) {
        // 不支持的组件类型（如 insert-beam, insert-rigid-support），暂不处理
        return;
    }

    // 查找目标路由和段
    auto routes = document_.findByType<model::Route>();
    if (routes.empty()) {
        return;  // 无路由，无法插入
    }

    model::Route* targetRoute = nullptr;
    model::Segment* targetSeg = nullptr;

    // 优先从选中对象推导路由/段
    if (!selectionManager_.selected().empty()) {
        const auto& selId = selectionManager_.selected().front();
        auto* selObj = document_.findObject(selId);
        if (auto* pp = dynamic_cast<model::PipePoint*>(selObj)) {
            // 查找包含此管点的路由和段
            for (auto* r : routes) {
                for (const auto& s : r->segments()) {
                    for (const auto& p : s->points()) {
                        if (p->id() == selId) {
                            targetRoute = r;
                            targetSeg = s.get();
                            break;
                        }
                    }
                    if (targetSeg) break;
                }
                if (targetRoute) break;
            }
        }
    }

    // 回退：使用第一个路由的第一个段
    if (!targetRoute) {
        targetRoute = routes.front();
        if (!targetRoute->segments().empty()) {
            targetSeg = targetRoute->segments().front().get();
        } else {
            return;  // 路由无段，无法插入
        }
    }

    // 确定插入坐标：若段有管点，偏移最后一个管点；否则原点
    double x = 0.0, y = 0.0, z = 0.0;
    if (targetSeg->pointCount() > 0) {
        auto* lastPt = targetSeg->pointAt(targetSeg->pointCount() - 1);
        if (lastPt) {
            x = lastPt->position().X() + 1000.0;
            y = lastPt->position().Y();
            z = lastPt->position().Z();
        }
    }

    auto ctx = app::Application::instance().createCommandContext();
    auto cmd = command::InsertComponentCommand::create(
        compType, targetRoute->id(), targetSeg->id(),
        x, y, z);
    commandStack_.execute(std::move(cmd), ctx);
    emit transactionStateChanged();
}

void AppController::wireCallbacks()
{
    selectionManager_.addSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        emit selectionChanged();
    });

    commandStack_.stackChanged.connect([this]() {
        emit transactionStateChanged();
    });
}

} // namespace ui
