#include "ui/AppController.h"

namespace ui {

AppController::AppController(app::Document& document,
                             app::TransactionManager& transactionManager,
                             app::SelectionManager& selectionManager,
                             QObject* parent)
    : QObject(parent)
    , document_(document)
    , transactionManager_(transactionManager)
    , selectionManager_(selectionManager)
{
    wireCallbacks();
}

QString AppController::documentName() const
{
    return QString::fromStdString(document_.name());
}

void AppController::setDocumentName(const QString& name)
{
    const std::string next = name.toStdString();
    if (document_.name() == next) {
        return;
    }

    document_.setName(next);
    emit documentNameChanged();
}

int AppController::selectedCount() const
{
    return static_cast<int>(selectionManager_.size());
}

QStringList AppController::selectedUuids() const
{
    QStringList ids;
    ids.reserve(static_cast<qsizetype>(selectionManager_.selected().size()));

    for (const auto& id : selectionManager_.selected()) {
        ids.push_back(QString::fromStdString(id.toString()));
    }
    return ids;
}

bool AppController::canUndo() const
{
    return transactionManager_.canUndo();
}

bool AppController::canRedo() const
{
    return transactionManager_.canRedo();
}

void AppController::clearSelection()
{
    selectionManager_.clear();
}

void AppController::undo()
{
    if (!transactionManager_.canUndo()) {
        return;
    }
    transactionManager_.undo();
    emit transactionStateChanged();
}

void AppController::redo()
{
    if (!transactionManager_.canRedo()) {
        return;
    }
    transactionManager_.redo();
    emit transactionStateChanged();
}

void AppController::wireCallbacks()
{
    selectionManager_.setSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        emit selectionChanged();
    });
}

} // namespace ui
