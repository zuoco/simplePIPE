#include "visualization/SceneManager.h"

#include <algorithm>

namespace visualization {

SceneManager::SceneManager()
    : root_{vsg::Group::create()}
{
}

vsg::ref_ptr<vsg::Group> SceneManager::root() const {
    return root_;
}

void SceneManager::addNode(const std::string& uuid, vsg::ref_ptr<vsg::Node> node) {
    if (!node || nodeMap_.count(uuid)) {
        return; // 空节点或 UUID 已存在时忽略
    }
    nodeMap_[uuid] = node;
    root_->addChild(node);
}

bool SceneManager::removeNode(const std::string& uuid) {
    auto it = nodeMap_.find(uuid);
    if (it == nodeMap_.end()) {
        return false;
    }

    // 从 root_->children 中移除对应节点
    auto& children = root_->children;
    auto childIt = std::find(children.begin(), children.end(), it->second);
    if (childIt != children.end()) {
        children.erase(childIt);
    }

    nodeMap_.erase(it);
    return true;
}

bool SceneManager::updateNode(const std::string& uuid, vsg::ref_ptr<vsg::Node> newNode) {
    auto it = nodeMap_.find(uuid);
    if (it == nodeMap_.end() || !newNode) {
        return false;
    }

    // 在 root_->children 中找到旧节点并替换
    auto& children = root_->children;
    auto childIt = std::find(children.begin(), children.end(), it->second);
    if (childIt != children.end()) {
        *childIt = newNode;
    }

    it->second = newNode;
    return true;
}

void SceneManager::batchUpdate(
    const std::vector<std::pair<std::string, vsg::ref_ptr<vsg::Node>>>& updates)
{
    for (const auto& [uuid, newNode] : updates) {
        updateNode(uuid, newNode);
    }
}

vsg::ref_ptr<vsg::Node> SceneManager::findNode(const std::string& uuid) const {
    auto it = nodeMap_.find(uuid);
    return (it != nodeMap_.end()) ? it->second : vsg::ref_ptr<vsg::Node>{};
}

std::string SceneManager::findUuidByNode(const vsg::Node* node) const {
    if (!node) {
        return {};
    }

    for (const auto& [uuid, mappedNode] : nodeMap_) {
        if (mappedNode.get() == node) {
            return uuid;
        }
    }

    return {};
}

bool SceneManager::hasNode(const std::string& uuid) const {
    return nodeMap_.count(uuid) > 0;
}

size_t SceneManager::nodeCount() const {
    return nodeMap_.size();
}

} // namespace visualization
