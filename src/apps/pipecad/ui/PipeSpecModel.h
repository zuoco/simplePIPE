// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "command/CommandStack.h"

#include <QAbstractTableModel>

#include <vector>

namespace model {
class PipeSpec;
}

namespace ui {

class PipeSpecModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        NameColumn = 0,
        OdColumn,
        WallThicknessColumn,
        MaterialColumn,
        ColumnCount
    };
    Q_ENUM(Column)

    PipeSpecModel(app::Document& document,
                  command::CommandStack& commandStack,
                  QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    Q_INVOKABLE void refresh();

private:
    app::Document& document_;
    command::CommandStack& commandStack_;
    std::vector<model::PipeSpec*> rows_;

    void rebuildRows();
    model::PipeSpec* specAt(int row) const;
};

} // namespace ui
