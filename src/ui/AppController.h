#pragma once

#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/TransactionManager.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

namespace ui {

class PipePointTableModel;
class SegmentTreeModel;
class PropertyModel;
class PipeSpecModel;

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString documentName READ documentName WRITE setDocumentName NOTIFY documentNameChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
    Q_PROPERTY(QStringList selectedUuids READ selectedUuids NOTIFY selectionChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY transactionStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY transactionStateChanged)
    Q_PROPERTY(QObject* pipePointTableModel READ pipePointTableModel CONSTANT)
    Q_PROPERTY(QObject* segmentTreeModel READ segmentTreeModel CONSTANT)
    Q_PROPERTY(QObject* propertyModel READ propertyModel CONSTANT)
    Q_PROPERTY(QObject* pipeSpecModel READ pipeSpecModel CONSTANT)

public:
    AppController(app::Document& document,
                  app::TransactionManager& transactionManager,
                  app::SelectionManager& selectionManager,
                  QObject* parent = nullptr);
    ~AppController() override;

    QString documentName() const;
    void setDocumentName(const QString& name);

    int selectedCount() const;
    QStringList selectedUuids() const;
    bool canUndo() const;
    bool canRedo() const;

    QObject* pipePointTableModel() const;
    QObject* segmentTreeModel() const;
    QObject* propertyModel() const;
    QObject* pipeSpecModel() const;

    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

signals:
    void documentNameChanged();
    void selectionChanged();
    void transactionStateChanged();

private:
    app::Document& document_;
    app::TransactionManager& transactionManager_;
    app::SelectionManager& selectionManager_;

    std::unique_ptr<PipePointTableModel> pipePointTableModel_;
    std::unique_ptr<SegmentTreeModel> segmentTreeModel_;
    std::unique_ptr<PropertyModel> propertyModel_;
    std::unique_ptr<PipeSpecModel> pipeSpecModel_;

    void wireCallbacks();
};

} // namespace ui
