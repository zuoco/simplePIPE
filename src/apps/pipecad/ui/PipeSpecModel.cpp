// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/PipeSpecModel.h"

#include "command/CommandContext.h"
#include "command/SetPropertyCommand.h"
#include "model/PipeSpec.h"

#include <algorithm>

namespace ui {

PipeSpecModel::PipeSpecModel(app::Document& document,
                             command::CommandStack& commandStack,
                             QObject* parent)
    : QAbstractTableModel(parent)
    , document_(document)
    , commandStack_(commandStack)
{
    rebuildRows();
}

int PipeSpecModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(rows_.size());
}

int PipeSpecModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant PipeSpecModel::data(const QModelIndex& index, int role) const
{
    model::PipeSpec* spec = specAt(index.row());
    if (!spec || index.column() < 0 || index.column() >= ColumnCount) {
        return {};
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    switch (index.column()) {
    case NameColumn:
        return QString::fromStdString(spec->name());
    case OdColumn:
        return spec->od();
    case WallThicknessColumn:
        return spec->wallThickness();
    case MaterialColumn:
        return QString::fromStdString(spec->material());
    default:
        return {};
    }
}

QVariant PipeSpecModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case NameColumn:
        return QStringLiteral("Name");
    case OdColumn:
        return QStringLiteral("OD");
    case WallThicknessColumn:
        return QStringLiteral("wallThickness");
    case MaterialColumn:
        return QStringLiteral("material");
    default:
        return {};
    }
}

Qt::ItemFlags PipeSpecModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool PipeSpecModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole || !index.isValid()) {
        return false;
    }

    model::PipeSpec* spec = specAt(index.row());
    if (!spec) {
        return false;
    }

    foundation::Variant oldVal;
    foundation::Variant newVal;
    std::string key;

    switch (index.column()) {
    case NameColumn: {
        const std::string oldStr = spec->name();
        const std::string newStr = value.toString().toStdString();
        if (oldStr == newStr) return false;
        key = "name";
        oldVal = oldStr;
        newVal = newStr;
        break;
    }
    case OdColumn: {
        bool ok = false;
        const double nv = value.toDouble(&ok);
        if (!ok) return false;
        const double ov = spec->od();
        if (ov == nv) return false;
        key = "OD";
        oldVal = ov;
        newVal = nv;
        break;
    }
    case WallThicknessColumn: {
        bool ok = false;
        const double nv = value.toDouble(&ok);
        if (!ok) return false;
        const double ov = spec->wallThickness();
        if (ov == nv) return false;
        key = "wallThickness";
        oldVal = ov;
        newVal = nv;
        break;
    }
    case MaterialColumn: {
        const std::string ov = spec->material();
        const std::string nv = value.toString().toStdString();
        if (ov == nv) return false;
        key = "material";
        oldVal = ov;
        newVal = nv;
        break;
    }
    default:
        return false;
    }

    auto cmd = command::SetPropertyCommand::createWithOldValue(spec->id(), key, oldVal, newVal);
    command::CommandContext ctx{&document_, nullptr, nullptr};
    commandStack_.execute(std::move(cmd), ctx);

    emit dataChanged(index, index);
    rebuildRows();
    return true;
}

void PipeSpecModel::refresh()
{
    rebuildRows();
}

void PipeSpecModel::rebuildRows()
{
    beginResetModel();
    rows_ = document_.allPipeSpecs();
    std::sort(rows_.begin(), rows_.end(), [](const model::PipeSpec* lhs, const model::PipeSpec* rhs) {
        return lhs->name() < rhs->name();
    });
    endResetModel();
}

model::PipeSpec* PipeSpecModel::specAt(int row) const
{
    if (row < 0 || static_cast<std::size_t>(row) >= rows_.size()) {
        return nullptr;
    }
    return rows_[static_cast<std::size_t>(row)];
}

} // namespace ui
