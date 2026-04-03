// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/AppController.h"

#include "model/PipePoint.h"
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
                             app::TransactionManager& transactionManager,
                             app::SelectionManager& selectionManager,
                             QObject* parent)
    : QObject(parent)
    , document_(document)
    , transactionManager_(transactionManager)
    , selectionManager_(selectionManager)
    , pipePointTableModel_(std::make_unique<PipePointTableModel>(document_, transactionManager_, selectionManager_))
    , segmentTreeModel_(std::make_unique<SegmentTreeModel>(document_, selectionManager_))
    , propertyModel_(std::make_unique<PropertyModel>(document_, selectionManager_))
    , pipeSpecModel_(std::make_unique<PipeSpecModel>(document_, transactionManager_))
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
    return transactionManager_.canUndo();
}

bool AppController::canRedo() const
{
    return transactionManager_.canRedo();
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
    if (!transactionManager_.canUndo()) {
        return;
    }
    transactionManager_.undo();
    emit transactionStateChanged();
}

void AppController::redo()
{
    if (!transactionManager_.canRedo()) {
        return;
    }
    transactionManager_.redo();
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
    emit deleteRequested(selectedUuids());
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
    emit insertComponentRequested(componentType);
}

void AppController::wireCallbacks()
{
    selectionManager_.addSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        emit selectionChanged();
    });
}

} // namespace ui
