// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "app/SelectionManager.h"
#include "model/PipePoint.h"

#include <QAbstractListModel>

#include <string>
#include <vector>

namespace model {
class PipeSpec;
}

namespace ui {

class PropertyModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        GroupRole = Qt::UserRole + 1,
        KeyRole,
        ValueRole,
        EditableRole
    };

    PropertyModel(app::Document& document,
                  app::SelectionManager& selectionManager,
                  QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();

private:
    struct Row {
        std::string group;
        std::string key;
        std::string value;
        bool editable = false;
    };

    app::Document& document_;
    app::SelectionManager& selectionManager_;
    std::vector<Row> rows_;

    void rebuild();
    void pushPipePointRows(const model::PipePoint& point);
    void pushPipeSpecRows(const model::PipeSpec& spec);
    static std::string pipePointTypeName(model::PipePointType type);
};

} // namespace ui
