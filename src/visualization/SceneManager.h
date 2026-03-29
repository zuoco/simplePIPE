#pragma once

#include <vsg/nodes/Group.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace visualization {

/// @brief 管理文档对象 UUID 到 VSG 场景节点的映射
///
/// SceneManager 持有一个根 vsg::Group 节点，作为整个可视化场景的顶层容器。
/// 文档对象（管点、管件等）通过 UUID 字符串注册节点；
/// 增量更新（addNode / removeNode / updateNode / batchUpdate）保持场景与模型同步。
///
/// 典型使用流程：
///   SceneManager mgr;
///   mgr.addNode(uuid1, createPipePointNode(...));
///   mgr.addNode(uuid2, createComponentNode(...));
///   mgr.removeNode(uuid1);
///   mgr.updateNode(uuid2, createComponentNode(...)); // 重新网格化后替换
///   viewer->setSceneData(mgr.root());
class SceneManager {
public:
    SceneManager();

    /// 场景根节点（将此节点设置为 Viewer 的场景数据）
    vsg::ref_ptr<vsg::Group> root() const;

    /// 向场景中注册一个对象节点
    /// @param uuid  文档对象 UUID 字符串
    /// @param node  要加入场景的 VSG 节点（应为 createPipePointNode/createComponentNode 的返回值）
    /// @note  若 uuid 已存在则不执行操作（需先调用 removeNode 再 addNode 或直接用 updateNode）
    void addNode(const std::string& uuid, vsg::ref_ptr<vsg::Node> node);

    /// 从场景中移除对象节点
    /// @param uuid  文档对象 UUID 字符串
    /// @return  true 若找到并移除，false 若 uuid 不存在
    bool removeNode(const std::string& uuid);

    /// 替换场景中已有对象节点（原子更新，保持 UUID 映射）
    /// @param uuid     文档对象 UUID 字符串
    /// @param newNode  新节点（重新网格化后的几何节点）
    /// @return  true 若找到并替换，false 若 uuid 不存在
    bool updateNode(const std::string& uuid, vsg::ref_ptr<vsg::Node> newNode);

    /// 批量更新多个对象节点（recompute 后批量调用）
    /// @param updates  (uuid, newNode) 对的列表
    void batchUpdate(
        const std::vector<std::pair<std::string, vsg::ref_ptr<vsg::Node>>>& updates);

    /// 按 UUID 查找节点（不存在时返回空 ref_ptr）
    vsg::ref_ptr<vsg::Node> findNode(const std::string& uuid) const;

    /// 按节点指针反查 UUID（不存在时返回空字符串）
    std::string findUuidByNode(const vsg::Node* node) const;

    /// 是否存在指定 UUID 的节点
    bool hasNode(const std::string& uuid) const;

    /// 当前场景中管理的节点数量
    size_t nodeCount() const;

    /// 返回所有已注册的 UUID 列表
    std::vector<std::string> allUuids() const;

private:
    vsg::ref_ptr<vsg::Group> root_;
    std::unordered_map<std::string, vsg::ref_ptr<vsg::Node>> nodeMap_;
};

} // namespace visualization
