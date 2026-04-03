// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/PipePointTableModel.h"

#include "model/PipePoint.h"
#include "model/PipeSpec.h"
#include "model/Route.h"
#include "model/Segment.h"

#include <QStringList>

#include <algorithm>
#include <set>
#include <string>

namespace ui {

namespace {

bool endsWithBendSuffix(const std::string& name)
{
    if (name.empty()) {
        return false;
    }

    const char suffix = name.back();
    return suffix == 'N' || suffix == 'M' || suffix == 'F';
}

} // namespace

PipePointTableModel::PipePointTableModel(app::Document& document,
                                         app::TransactionManager& transactionManager,
                                         app::SelectionManager& selectionManager,
                                         QObject* parent)
    : QAbstractTableModel(parent)
    , document_(document)
    , transactionManager_(transactionManager)
    , selectionManager_(selectionManager)
{
    rebuildRows();
    selectionManager_.addSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        syncSelectionFromManager();
    });
    syncSelectionFromManager();
}

int PipePointTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(rows_.size());
}

int PipePointTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant PipePointTableModel::data(const QModelIndex& index, int role) const
{
    const model::PipePoint* point = pipePointAt(index.row());
    if (!point || index.column() < 0 || index.column() >= ColumnCount) {
        return {};
    }

    if (role == Qt::BackgroundRole && isBendHelperRow(*point)) {
        return QColor(QStringLiteral("#E6E6E6"));
    }

    if (role == Qt::TextAlignmentRole &&
        (index.column() == XColumn || index.column() == YColumn || index.column() == ZColumn ||
         index.column() == BendMultiplierColumn)) {
        return QVariant::fromValue<int>(Qt::AlignVCenter | Qt::AlignRight);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    switch (index.column()) {
    case NameColumn:
        return QString::fromStdString(point->name());
    case XColumn:
        return point->position().X();
    case YColumn:
        return point->position().Y();
    case ZColumn:
        return point->position().Z();
    case TypeColumn:
        return typeToString(point->type());
    case PipeSpecColumn:
        if (point->pipeSpec()) {
            return QString::fromStdString(point->pipeSpec()->name());
        }
        if (point->hasParam("pipeSpecId")) {
            return QString::fromStdString(foundation::variantToString(point->param("pipeSpecId")));
        }
        return {};
    case BendMultiplierColumn:
        if (point->hasParam("bendMultiplier")) {
            return foundation::variantToDouble(point->param("bendMultiplier"));
        }
        return {};
    default:
        return {};
    }
}

QVariant PipePointTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case NameColumn:
        return QStringLiteral("Name");
    case XColumn:
        return QStringLiteral("X");
    case YColumn:
        return QStringLiteral("Y");
    case ZColumn:
        return QStringLiteral("Z");
    case TypeColumn:
        return QStringLiteral("Type");
    case PipeSpecColumn:
        return QStringLiteral("PipeSpec");
    case BendMultiplierColumn:
        return QStringLiteral("bendMultiplier");
    default:
        return {};
    }
}

Qt::ItemFlags PipePointTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    const model::PipePoint* point = pipePointAt(index.row());
    if (!point) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (isEditableCell(*point, index.column())) {
        itemFlags |= Qt::ItemIsEditable;
    }
    return itemFlags;
}

bool PipePointTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || !index.isValid()) {
        return false;
    }

    model::PipePoint* point = pipePointAt(index.row());
    if (!point || !isEditableCell(*point, index.column())) {
        return false;
    }

    transactionManager_.open("Edit PipePoint Cell");
    bool changed = false;

    switch (index.column()) {
    case NameColumn: {
        const std::string oldValue = point->name();
        const std::string newValue = value.toString().toStdString();
        if (oldValue != newValue) {
            point->setName(newValue);
            transactionManager_.recordChange(point->id(), "name", oldValue, newValue);
            changed = true;
        }
        break;
    }
    case XColumn:
    case YColumn:
    case ZColumn: {
        bool ok = false;
        const double newCoord = value.toDouble(&ok);
        if (!ok) {
            transactionManager_.abort();
            return false;
        }

        const gp_Pnt current = point->position();
        const double oldCoord = (index.column() == XColumn) ? current.X()
            : (index.column() == YColumn)                     ? current.Y()
                                                              : current.Z();
        if (oldCoord != newCoord) {
            if (index.column() == XColumn) {
                point->setPosition(gp_Pnt(newCoord, current.Y(), current.Z()));
                transactionManager_.recordChange(point->id(), "x", oldCoord, newCoord);
            } else if (index.column() == YColumn) {
                point->setPosition(gp_Pnt(current.X(), newCoord, current.Z()));
                transactionManager_.recordChange(point->id(), "y", oldCoord, newCoord);
            } else {
                point->setPosition(gp_Pnt(current.X(), current.Y(), newCoord));
                transactionManager_.recordChange(point->id(), "z", oldCoord, newCoord);
            }
            changed = true;
        }
        break;
    }
    case TypeColumn: {
        model::PipePointType oldType = point->type();
        model::PipePointType newType = oldType;
        if (!tryParseType(value, newType)) {
            transactionManager_.abort();
            return false;
        }

        if (oldType != newType) {
            point->setType(newType);
            transactionManager_.recordChange(point->id(), "type",
                                             static_cast<int>(oldType),
                                             static_cast<int>(newType));
            changed = true;
        }
        break;
    }
    case PipeSpecColumn: {
        const std::string oldSpec = point->hasParam("pipeSpecId")
            ? foundation::variantToString(point->param("pipeSpecId"))
            : std::string{};
        const std::string newSpec = value.toString().toStdString();
        if (oldSpec != newSpec) {
            point->setParam("pipeSpecId", newSpec);
            transactionManager_.recordChange(point->id(), "pipeSpecId", oldSpec, newSpec);
            changed = true;
        }
        break;
    }
    case BendMultiplierColumn: {
        bool ok = false;
        const double nextMultiplier = value.toDouble(&ok);
        if (!ok) {
            transactionManager_.abort();
            return false;
        }

        const double oldMultiplier = point->hasParam("bendMultiplier")
            ? foundation::variantToDouble(point->param("bendMultiplier"))
            : 0.0;
        if (!point->hasParam("bendMultiplier") || oldMultiplier != nextMultiplier) {
            point->setParam("bendMultiplier", nextMultiplier);
            transactionManager_.recordChange(point->id(), "bendMultiplier", oldMultiplier, nextMultiplier);
            changed = true;
        }
        break;
    }
    default:
        transactionManager_.abort();
        return false;
    }

    if (!changed) {
        transactionManager_.abort();
        return false;
    }

    transactionManager_.commit();
    emit dataChanged(index, index);
    rebuildRows();
    return true;
}

int PipePointTableModel::selectedRow() const
{
    return selectedRow_;
}

void PipePointTableModel::refresh()
{
    rebuildRows();
}

bool PipePointTableModel::selectRow(int row)
{
    model::PipePoint* point = pipePointAt(row);
    if (!point) {
        return false;
    }

    selectionManager_.clear();
    return selectionManager_.select(point->id());
}

void PipePointTableModel::rebuildRows()
{
    beginResetModel();
    rows_.clear();

    std::set<std::string> seen;

    document_.forEach([this, &seen](model::DocumentObject& object) {
        auto* route = dynamic_cast<model::Route*>(&object);
        if (!route) {
            return;
        }

        for (const auto& segment : route->segments()) {
            for (const auto& point : segment->points()) {
                const std::string id = point->id().toString();
                if (seen.insert(id).second) {
                    rows_.push_back(point.get());
                }
            }
        }
    });

    auto extras = document_.allPipePoints();
    std::sort(extras.begin(), extras.end(), [](const model::PipePoint* lhs, const model::PipePoint* rhs) {
        return lhs->name() < rhs->name();
    });

    for (model::PipePoint* point : extras) {
        const std::string id = point->id().toString();
        if (seen.insert(id).second) {
            rows_.push_back(point);
        }
    }

    endResetModel();
    syncSelectionFromManager();
}

void PipePointTableModel::syncSelectionFromManager()
{
    int nextRow = -1;
    if (!selectionManager_.selected().empty()) {
        const foundation::UUID selectedId = selectionManager_.selected().front();
        for (std::size_t i = 0; i < rows_.size(); ++i) {
            if (rows_[i] && rows_[i]->id() == selectedId) {
                nextRow = static_cast<int>(i);
                break;
            }
        }
    }

    if (nextRow != selectedRow_) {
        selectedRow_ = nextRow;
        emit selectedRowChanged();
    }
}

model::PipePoint* PipePointTableModel::pipePointAt(int row) const
{
    if (row < 0 || static_cast<std::size_t>(row) >= rows_.size()) {
        return nullptr;
    }
    return rows_[static_cast<std::size_t>(row)];
}

bool PipePointTableModel::isBendHelperRow(const model::PipePoint& point) const
{
    return point.type() == model::PipePointType::Bend && endsWithBendSuffix(point.name());
}

bool PipePointTableModel::isEditableCell(const model::PipePoint& point, int column) const
{
    if (isBendHelperRow(point)) {
        return false;
    }

    if (column < 0 || column >= ColumnCount) {
        return false;
    }

    if (column == BendMultiplierColumn) {
        return point.type() == model::PipePointType::Bend;
    }

    return true;
}

QString PipePointTableModel::typeToString(model::PipePointType type) const
{
    switch (type) {
    case model::PipePointType::Run:
        return QStringLiteral("Run");
    case model::PipePointType::Bend:
        return QStringLiteral("Bend");
    case model::PipePointType::Reducer:
        return QStringLiteral("Reducer");
    case model::PipePointType::Tee:
        return QStringLiteral("Tee");
    case model::PipePointType::Valve:
        return QStringLiteral("Valve");
    case model::PipePointType::FlexJoint:
        return QStringLiteral("FlexJoint");
    default:
        return {};
    }
}

bool PipePointTableModel::tryParseType(const QVariant& value, model::PipePointType& outType) const
{
    bool ok = false;
    const int asInt = value.toInt(&ok);
    if (ok && asInt >= static_cast<int>(model::PipePointType::Run) &&
        asInt <= static_cast<int>(model::PipePointType::FlexJoint)) {
        outType = static_cast<model::PipePointType>(asInt);
        return true;
    }

    const QString text = value.toString().trimmed();
    if (text.compare(QStringLiteral("Run"), Qt::CaseInsensitive) == 0) {
        outType = model::PipePointType::Run;
        return true;
    }
    if (text.compare(QStringLiteral("Bend"), Qt::CaseInsensitive) == 0) {
        outType = model::PipePointType::Bend;
        return true;
    }
    if (text.compare(QStringLiteral("Reducer"), Qt::CaseInsensitive) == 0) {
        outType = model::PipePointType::Reducer;
        return true;
    }
    if (text.compare(QStringLiteral("Tee"), Qt::CaseInsensitive) == 0) {
        outType = model::PipePointType::Tee;
        return true;
    }
    if (text.compare(QStringLiteral("Valve"), Qt::CaseInsensitive) == 0) {
        outType = model::PipePointType::Valve;
        return true;
    }
    if (text.compare(QStringLiteral("FlexJoint"), Qt::CaseInsensitive) == 0) {
        outType = model::PipePointType::FlexJoint;
        return true;
    }

    return false;
}

} // namespace ui
