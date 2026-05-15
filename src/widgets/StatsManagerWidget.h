#ifndef STATSMANAGERWIDGET_H
#define STATSMANAGERWIDGET_H

#include <QWidget>
#include <QTreeView>
#include "models/StatsTreeModel.h"

class StatsManagerWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatsManagerWidget(QWidget* parent = nullptr);

private:
    QTreeView* m_treeView;
    StatsTreeModel* m_model;
};

#endif // STATSMANAGERWIDGET_H