#include "ui/AppController.h"

#include "ui/PipePointTableModel.h"
#include "ui/PipeSpecModel.h"
#include "ui/PropertyModel.h"
#include "ui/SegmentTreeModel.h"

namespace ui {

AppController::AppController(app::Document& document,
                             app::TransactionManager& transactionManager,
                             app::SelectionManager& selectionManager,
                             QObject* parent)
    : QObject(parent)
    , document_(document)
    , transactionManager_(transactionManager)
    , selectionManager_(selectionManager)
    , pipePointTableModel_(std::make_unique<PipePointTableModel>(document_, transactionManager_, selectionManager_))
    , segmentTreeModel_(std::make_unique<SegmentTreeModel>(document_, selectionManager_))
    , propertyModel_(std::make_unique<PropertyModel>(document_, selectionManager_))
    , pipeSpecModel_(std::make_unique<PipeSpecModel>(document_, transactionManager_))
{
    wireCallbacks();
}

AppController::~AppController() = default;

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

QObject* AppController::pipePointTableModel() const
{
    return pipePointTableModel_.get();
}

QObject* AppController::segmentTreeModel() const
{
    return segmentTreeModel_.get();
}

QObject* AppController::propertyModel() const
{
    return propertyModel_.get();
}

QObject* AppController::pipeSpecModel() const
{
    return pipeSpecModel_.get();
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
    selectionManager_.addSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        emit selectionChanged();
    });
}

} // namespace ui
