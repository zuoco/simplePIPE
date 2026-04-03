// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#include "ui/PipeSpecModel.h"

#include "model/PipeSpec.h"

#include <algorithm>

namespace ui {

PipeSpecModel::PipeSpecModel(app::Document& document,
                             app::TransactionManager& transactionManager,
                             QObject* parent)
    : QAbstractTableModel(parent)
    , document_(document)
    , transactionManager_(transactionManager)
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

    transactionManager_.open("Edit PipeSpec Cell");
    bool changed = false;

    switch (index.column()) {
    case NameColumn: {
        const std::string oldValue = spec->name();
        const std::string newValue = value.toString().toStdString();
        if (oldValue != newValue) {
            spec->setName(newValue);
            transactionManager_.recordChange(spec->id(), "name", oldValue, newValue);
            changed = true;
        }
        break;
    }
    case OdColumn: {
        bool ok = false;
        const double newValue = value.toDouble(&ok);
        if (!ok) {
            transactionManager_.abort();
            return false;
        }

        const double oldValue = spec->od();
        if (oldValue != newValue) {
            spec->setOd(newValue);
            transactionManager_.recordChange(spec->id(), "OD", oldValue, newValue);
            changed = true;
        }
        break;
    }
    case WallThicknessColumn: {
        bool ok = false;
        const double newValue = value.toDouble(&ok);
        if (!ok) {
            transactionManager_.abort();
            return false;
        }

        const double oldValue = spec->wallThickness();
        if (oldValue != newValue) {
            spec->setWallThickness(newValue);
            transactionManager_.recordChange(spec->id(), "wallThickness", oldValue, newValue);
            changed = true;
        }
        break;
    }
    case MaterialColumn: {
        const std::string oldValue = spec->material();
        const std::string newValue = value.toString().toStdString();
        if (oldValue != newValue) {
            spec->setMaterial(newValue);
            transactionManager_.recordChange(spec->id(), "material", oldValue, newValue);
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
