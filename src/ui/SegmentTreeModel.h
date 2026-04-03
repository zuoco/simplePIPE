// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "app/SelectionManager.h"

#include <QAbstractItemModel>
#include <QString>

#include <memory>
#include <unordered_map>
#include <vector>

namespace model {
class DocumentObject;
}

namespace ui {

class SegmentTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        KindRole,
        UuidRole,
        SelectedRole
    };

    SegmentTreeModel(app::Document& document,
                     app::SelectionManager& selectionManager,
                     QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool selectNodeByUuid(const QString& uuid);

private:
    enum class NodeKind {
        Root,
        Route,
        Segment,
        PipePoint
    };

    struct Node {
        NodeKind kind = NodeKind::Root;
        model::DocumentObject* object = nullptr;
        Node* parent = nullptr;
        std::vector<std::unique_ptr<Node>> children;
    };

    app::Document& document_;
    app::SelectionManager& selectionManager_;
    std::unique_ptr<Node> root_;
    std::unordered_map<std::string, Node*> uuidToNode_;

    void rebuildTree();
    const Node* nodeFromIndex(const QModelIndex& index) const;
    Node* nodeFromIndex(const QModelIndex& index);
    QModelIndex indexForNode(const Node* node) const;
    bool isNodeSelected(const Node& node) const;
};

} // namespace ui
