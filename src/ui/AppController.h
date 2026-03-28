#pragma once

#include "app/Document.h"
#include "app/SelectionManager.h"
#include "app/TransactionManager.h"

#include <QObject>
#include <QString>
#include <QStringList>

namespace ui {

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString documentName READ documentName WRITE setDocumentName NOTIFY documentNameChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
    Q_PROPERTY(QStringList selectedUuids READ selectedUuids NOTIFY selectionChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY transactionStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY transactionStateChanged)

public:
    AppController(app::Document& document,
                  app::TransactionManager& transactionManager,
                  app::SelectionManager& selectionManager,
                  QObject* parent = nullptr);

    QString documentName() const;
    void setDocumentName(const QString& name);

    int selectedCount() const;
    QStringList selectedUuids() const;
    bool canUndo() const;
    bool canRedo() const;

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

    void wireCallbacks();
};

} // namespace ui
