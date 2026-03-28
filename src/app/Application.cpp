#include "app/Application.h"

namespace app {

Application* Application::instance_ = nullptr;
std::once_flag Application::initFlag_;

Application::Application()
    : document_(std::make_unique<Document>())
    , dependencyGraph_(std::make_unique<DependencyGraph>())
    , transactionManager_(std::make_unique<TransactionManager>(*document_, *dependencyGraph_))
    , selectionManager_(std::make_unique<SelectionManager>())
    , workbenchManager_(std::make_unique<WorkbenchManager>(*document_))
{
}

void Application::init() {
    std::call_once(initFlag_, []() {
        instance_ = new Application();
    });
}

Application& Application::instance() {
    if (!instance_) {
        throw std::runtime_error("Application::init() must be called before instance()");
    }
    return *instance_;
}

void Application::destroy() {
    delete instance_;
    instance_ = nullptr;
    // 重置 once_flag 需要重新构造（用于测试场景）
    new (&initFlag_) std::once_flag();
}

} // namespace app
