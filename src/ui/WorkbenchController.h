#pragma once

#include "app/WorkbenchManager.h"
#include "visualization/ViewManager.h"

#include <QObject>
#include <QString>
#include <QStringList>

namespace ui {

class WorkbenchController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeWorkbench READ activeWorkbench NOTIFY activeWorkbenchChanged)
    Q_PROPERTY(QStringList activePanels READ activePanels NOTIFY activeWorkbenchChanged)
    Q_PROPERTY(QStringList toolbarActions READ toolbarActions NOTIFY activeWorkbenchChanged)
    Q_PROPERTY(QStringList workbenchNames READ workbenchNames CONSTANT)

public:
    explicit WorkbenchController(app::WorkbenchManager& manager, visualization::ViewManager* viewManager = nullptr, QObject* parent = nullptr);

    QString activeWorkbench() const;
    QStringList activePanels() const;
    QStringList toolbarActions() const;
    QStringList workbenchNames() const;

    Q_INVOKABLE bool switchWorkbench(const QString& name);
    Q_INVOKABLE void notifyViewportLoaded(QObject* viewportObj);


signals:
    void viewportLoaded(QObject* viewportObj);

    void activeWorkbenchChanged();

private:
    app::WorkbenchManager& manager_;
    visualization::ViewManager* viewManager_ = nullptr;
};

} // namespace ui
