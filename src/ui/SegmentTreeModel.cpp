#include "ui/SegmentTreeModel.h"

#include "model/PipePoint.h"
#include "model/Route.h"
#include "model/Segment.h"
#include "ui/UuidUtil.h"

#include <algorithm>
#include <set>

namespace ui {

SegmentTreeModel::SegmentTreeModel(app::Document& document,
                                   app::SelectionManager& selectionManager,
                                   QObject* parent)
    : QAbstractItemModel(parent)
    , document_(document)
    , selectionManager_(selectionManager)
{
    rebuildTree();
    selectionManager_.addSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        if (rowCount() <= 0) {
            return;
        }
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {SelectedRole});
    });
}

QModelIndex SegmentTreeModel::index(int row, int column, const QModelIndex& parentIndex) const
{
    if (column != 0 || row < 0 || (parentIndex.isValid() && parentIndex.column() != 0)) {
        return {};
    }

    const Node* parentNode = nodeFromIndex(parentIndex);
    if (!parentNode) {
        parentNode = root_.get();
    }

    if (static_cast<std::size_t>(row) >= parentNode->children.size()) {
        return {};
    }

    return createIndex(row, column, parentNode->children[static_cast<std::size_t>(row)].get());
}

QModelIndex SegmentTreeModel::parent(const QModelIndex& childIndex) const
{
    if (!childIndex.isValid()) {
        return {};
    }

    const auto* childNode = static_cast<const Node*>(childIndex.internalPointer());
    if (!childNode || !childNode->parent || childNode->parent == root_.get()) {
        return {};
    }

    return indexForNode(childNode->parent);
}

int SegmentTreeModel::rowCount(const QModelIndex& parentIndex) const
{
    const Node* parentNode = nodeFromIndex(parentIndex);
    if (!parentNode) {
        parentNode = root_.get();
    }

    return static_cast<int>(parentNode->children.size());
}

int SegmentTreeModel::columnCount(const QModelIndex& parent) const
{
    (void)parent;
    return 1;
}

QVariant SegmentTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const Node* node = static_cast<const Node*>(index.internalPointer());
    if (!node) {
        return {};
    }

    if (role == KindRole) {
        switch (node->kind) {
        case NodeKind::Route:
            return QStringLiteral("Route");
        case NodeKind::Segment:
            return QStringLiteral("Segment");
        case NodeKind::PipePoint:
            return QStringLiteral("PipePoint");
        default:
            return QStringLiteral("Root");
        }
    }

    if (role == UuidRole) {
        return node->object ? QString::fromStdString(node->object->id().toString()) : QString{};
    }

    if (role == SelectedRole) {
        return isNodeSelected(*node);
    }

    if (role != Qt::DisplayRole && role != NameRole) {
        return {};
    }

    if (!node->object) {
        return {};
    }

    return QString::fromStdString(node->object->name());
}

QHash<int, QByteArray> SegmentTreeModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {KindRole, "kind"},
        {UuidRole, "uuid"},
        {SelectedRole, "selected"},
    };
}

void SegmentTreeModel::refresh()
{
    rebuildTree();
}

bool SegmentTreeModel::selectNodeByUuid(const QString& uuid)
{
    foundation::UUID id;
    if (!uuidFromString(uuid.toStdString(), id)) {
        return false;
    }

    auto it = uuidToNode_.find(id.toString());
    if (it == uuidToNode_.end()) {
        return false;
    }

    if (it->second->kind != NodeKind::PipePoint) {
        return false;
    }

    selectionManager_.clear();
    return selectionManager_.select(id);
}

void SegmentTreeModel::rebuildTree()
{
    beginResetModel();
    root_ = std::make_unique<Node>();
    root_->kind = NodeKind::Root;
    uuidToNode_.clear();

    std::set<std::string> seenRoutes;
    std::set<std::string> seenSegments;
    std::set<std::string> seenPipePoints;

    document_.forEach([this, &seenRoutes, &seenSegments, &seenPipePoints](model::DocumentObject& object) {
        auto* route = dynamic_cast<model::Route*>(&object);
        if (!route) {
            return;
        }

        const std::string routeId = route->id().toString();
        if (!seenRoutes.insert(routeId).second) {
            return;
        }

        auto routeNode = std::make_unique<Node>();
        routeNode->kind = NodeKind::Route;
        routeNode->object = route;
        routeNode->parent = root_.get();
        Node* routeRaw = routeNode.get();
        uuidToNode_[routeId] = routeRaw;
        root_->children.push_back(std::move(routeNode));

        for (const auto& segment : route->segments()) {
            const std::string segmentId = segment->id().toString();
            if (!seenSegments.insert(segmentId).second) {
                continue;
            }

            auto segmentNode = std::make_unique<Node>();
            segmentNode->kind = NodeKind::Segment;
            segmentNode->object = segment.get();
            segmentNode->parent = routeRaw;
            Node* segmentRaw = segmentNode.get();
            uuidToNode_[segmentId] = segmentRaw;
            routeRaw->children.push_back(std::move(segmentNode));

            for (const auto& point : segment->points()) {
                const std::string pointId = point->id().toString();
                if (!seenPipePoints.insert(pointId).second) {
                    continue;
                }

                auto pointNode = std::make_unique<Node>();
                pointNode->kind = NodeKind::PipePoint;
                pointNode->object = point.get();
                pointNode->parent = segmentRaw;
                uuidToNode_[pointId] = pointNode.get();
                segmentRaw->children.push_back(std::move(pointNode));
            }
        }
    });

    auto looseSegments = document_.allSegments();
    std::sort(looseSegments.begin(), looseSegments.end(), [](const model::Segment* lhs, const model::Segment* rhs) {
        return lhs->name() < rhs->name();
    });

    for (model::Segment* segment : looseSegments) {
        const std::string segmentId = segment->id().toString();
        if (!seenSegments.insert(segmentId).second) {
            continue;
        }

        auto segmentNode = std::make_unique<Node>();
        segmentNode->kind = NodeKind::Segment;
        segmentNode->object = segment;
        segmentNode->parent = root_.get();
        Node* segmentRaw = segmentNode.get();
        uuidToNode_[segmentId] = segmentRaw;
        root_->children.push_back(std::move(segmentNode));

        for (const auto& point : segment->points()) {
            const std::string pointId = point->id().toString();
            if (!seenPipePoints.insert(pointId).second) {
                continue;
            }

            auto pointNode = std::make_unique<Node>();
            pointNode->kind = NodeKind::PipePoint;
            pointNode->object = point.get();
            pointNode->parent = segmentRaw;
            uuidToNode_[pointId] = pointNode.get();
            segmentRaw->children.push_back(std::move(pointNode));
        }
    }

    auto loosePoints = document_.allPipePoints();
    std::sort(loosePoints.begin(), loosePoints.end(), [](const model::PipePoint* lhs, const model::PipePoint* rhs) {
        return lhs->name() < rhs->name();
    });

    for (model::PipePoint* point : loosePoints) {
        const std::string pointId = point->id().toString();
        if (!seenPipePoints.insert(pointId).second) {
            continue;
        }

        auto pointNode = std::make_unique<Node>();
        pointNode->kind = NodeKind::PipePoint;
        pointNode->object = point;
        pointNode->parent = root_.get();
        uuidToNode_[pointId] = pointNode.get();
        root_->children.push_back(std::move(pointNode));
    }

    endResetModel();
}

const SegmentTreeModel::Node* SegmentTreeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }
    return static_cast<const Node*>(index.internalPointer());
}

SegmentTreeModel::Node* SegmentTreeModel::nodeFromIndex(const QModelIndex& index)
{
    if (!index.isValid()) {
        return nullptr;
    }
    return static_cast<Node*>(index.internalPointer());
}

QModelIndex SegmentTreeModel::indexForNode(const Node* node) const
{
    if (!node || !node->parent) {
        return {};
    }

    const Node* parentNode = node->parent;
    for (std::size_t i = 0; i < parentNode->children.size(); ++i) {
        if (parentNode->children[i].get() == node) {
            return createIndex(static_cast<int>(i), 0, const_cast<Node*>(node));
        }
    }
    return {};
}

bool SegmentTreeModel::isNodeSelected(const Node& node) const
{
    if (!node.object || selectionManager_.selected().empty()) {
        return false;
    }

    const foundation::UUID selectedId = selectionManager_.selected().front();
    return node.object->id() == selectedId;
}

} // namespace ui
