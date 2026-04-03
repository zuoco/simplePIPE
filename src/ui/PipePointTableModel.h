// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/TransactionManager.h"

#include <QAbstractTableModel>
#include <QColor>
#include <QString>

#include <vector>

namespace model {
class PipePoint;
}

namespace ui {

class PipePointTableModel : public QAbstractTableModel {
    Q_OBJECT
    Q_PROPERTY(int selectedRow READ selectedRow NOTIFY selectedRowChanged)

public:
    enum Column {
        NameColumn = 0,
        XColumn,
        YColumn,
        ZColumn,
        TypeColumn,
        PipeSpecColumn,
        BendMultiplierColumn,
        ColumnCount
    };
    Q_ENUM(Column)

    PipePointTableModel(app::Document& document,
                        app::TransactionManager& transactionManager,
                        app::SelectionManager& selectionManager,
                        QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    int selectedRow() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool selectRow(int row);

signals:
    void selectedRowChanged();

private:
    app::Document& document_;
    app::TransactionManager& transactionManager_;
    app::SelectionManager& selectionManager_;
    std::vector<model::PipePoint*> rows_;
    int selectedRow_ = -1;

    void rebuildRows();
    void syncSelectionFromManager();

    model::PipePoint* pipePointAt(int row) const;
    bool isBendHelperRow(const model::PipePoint& point) const;
    bool isEditableCell(const model::PipePoint& point, int column) const;

    QString typeToString(model::PipePointType type) const;
    bool tryParseType(const QVariant& value, model::PipePointType& outType) const;
};

} // namespace ui
