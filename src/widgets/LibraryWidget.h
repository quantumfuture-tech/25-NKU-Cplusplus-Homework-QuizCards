#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QWidget>
#include <QSplitter>
#include <QTreeView>
#include <QTableView>
#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include "models/TreeModel.h"
#include "models/CardTableModel.h"

class LibraryWidget : public QWidget {
    Q_OBJECT
public:
    explicit LibraryWidget(QWidget* parent = nullptr);
private slots:
    void onTreeSelectionChanged(const QModelIndex &index);
    //只负责弃选
    void onTreeItemClicked(const QModelIndex &index);
    void onAddCard();
    void onEditCard();
    void onDeleteCard();
    void onBatchImport();
    void onAddUnit();
    void onAddFolder();
    void onTreeContextMenu(const QPoint& pos);

private:
    int m_previousId;

    void saveExpandedState();
    void restoreExpandedState();
    QSet<int> m_expandedNodeIndices;
    QSet<QString> m_expandedPaths;  // 存储从根到节点的完整路径字符串
    
    QSplitter* m_splitter;
    QTreeView* m_treeView;
    QTableView* m_cardView;
    TreeModel* m_treeModel;
    CardTableModel* m_cardModel;
    
    QPushButton* m_newUnitBtn;
    QPushButton* m_newFolderBtn;
    QPushButton* m_addCardBtn;
    QPushButton* m_batchBtn;
    
    QWidget* m_rightPanel;
    QLabel* m_unitStatsLabel;
};

#endif // LIBRARYWIDGET_H