#pragma once

#include "app/WorkbenchManager.h"

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
    explicit WorkbenchController(app::WorkbenchManager& manager, QObject* parent = nullptr);

    QString activeWorkbench() const;
    QStringList activePanels() const;
    QStringList toolbarActions() const;
    QStringList workbenchNames() const;

    Q_INVOKABLE bool switchWorkbench(const QString& name);

signals:
    void activeWorkbenchChanged();

private:
    app::WorkbenchManager& manager_;
};

} // namespace ui
