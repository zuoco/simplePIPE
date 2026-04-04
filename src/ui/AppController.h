// Copyright 2024-2026 PipeCAD Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "app/Document.h"
#include "app/SelectionManager.h"
#include "command/CommandStack.h"

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

    // StatusBar properties
    Q_PROPERTY(QString selectionInfo READ selectionInfo NOTIFY selectionChanged)
    Q_PROPERTY(QString mouseCoord READ mouseCoord NOTIFY mouseCoordChanged)
    Q_PROPERTY(double zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)

public:
    AppController(app::Document& document,
                  command::CommandStack& commandStack,
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

    // StatusBar accessors
    QString selectionInfo() const;
    QString mouseCoord() const;
    double zoomLevel() const;
    void setZoomLevel(double level);
    bool hasSelection() const;

    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void updateMouseCoord(double x, double y, double z);
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void selectByUuid(const QString& uuid);
    Q_INVOKABLE void multiSelect(const QStringList& uuids, bool append);

    /// 触发元件插入流程（由 ComponentToolStrip 调用）
    Q_INVOKABLE void insertComponent(const QString& componentType);

signals:
    void documentNameChanged();
    void selectionChanged();
    void transactionStateChanged();
    void mouseCoordChanged();
    void zoomLevelChanged();
    void editModeRequested();
    void viewModeRequested();

private:
    app::Document& document_;
    command::CommandStack& commandStack_;
    app::SelectionManager& selectionManager_;

    std::unique_ptr<PipePointTableModel> pipePointTableModel_;
    std::unique_ptr<SegmentTreeModel> segmentTreeModel_;
    std::unique_ptr<PropertyModel> propertyModel_;
    std::unique_ptr<PipeSpecModel> pipeSpecModel_;

    double mouseX_ = 0.0;
    double mouseY_ = 0.0;
    double mouseZ_ = 0.0;
    double zoomLevel_ = 100.0;

    void wireCallbacks();
};

} // namespace ui
