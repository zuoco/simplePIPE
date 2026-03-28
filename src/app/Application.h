#pragma once

#include "app/Document.h"
#include "app/DependencyGraph.h"
#include "app/TransactionManager.h"
#include "app/SelectionManager.h"
#include "app/WorkbenchManager.h"

#include <memory>
#include <mutex>
#include <stdexcept>

namespace app {

/// 中央单例：持有所有管理器实例，提供全局统一访问入口。
///
/// 使用 Meyers' Singleton (C++11 static local) 保证线程安全的延迟初始化。
/// 必须先调用 init() 再调用 instance()。
///
/// 典型用法:
///   // main.cpp 中
///   Application::init();
///   auto& app = Application::instance();
///   app.document().setName("MyProject");
///   app.workbenchManager().switchWorkbench("Design");
///
///   // 任意位置
///   auto& doc = Application::instance().document();
class Application {
public:
    /// 初始化单例（只能调用一次，在 main() 最前面调用）
    static void init();

    /// 获取单例引用（init 未调用时抛异常）
    static Application& instance();

    /// 销毁单例（可选，程序退出时调用，便于测试重置）
    static void destroy();

    // ---- 管理器访问 ----

    Document&           document()           { return *document_; }
    DependencyGraph&    dependencyGraph()     { return *dependencyGraph_; }
    TransactionManager& transactionManager()  { return *transactionManager_; }
    SelectionManager&   selectionManager()    { return *selectionManager_; }
    WorkbenchManager&   workbenchManager()    { return *workbenchManager_; }

    const Document&           document()           const { return *document_; }
    const DependencyGraph&    dependencyGraph()     const { return *dependencyGraph_; }
    const TransactionManager& transactionManager()  const { return *transactionManager_; }
    const SelectionManager&   selectionManager()    const { return *selectionManager_; }
    const WorkbenchManager&   workbenchManager()    const { return *workbenchManager_; }

    // 禁止拷贝和移动
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

private:
    Application();
    ~Application() = default;

    std::unique_ptr<Document>           document_;
    std::unique_ptr<DependencyGraph>    dependencyGraph_;
    std::unique_ptr<TransactionManager> transactionManager_;
    std::unique_ptr<SelectionManager>   selectionManager_;
    std::unique_ptr<WorkbenchManager>   workbenchManager_;

    static Application* instance_;
    static std::once_flag initFlag_;
};

} // namespace app
