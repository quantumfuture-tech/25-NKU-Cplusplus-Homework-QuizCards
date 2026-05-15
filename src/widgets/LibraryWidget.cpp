#include "LibraryWidget.h"
#include "AppEngine.h"
#include "widgets/dialogs/EditCardDialog.h"
#include "widgets/dialogs/BatchImportDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QGroupBox>
#include <functional>
#include <QSet>

LibraryWidget::LibraryWidget(QWidget* parent) : QWidget(parent) {
    auto* engine = &AppEngine::instance();
    m_treeModel = new TreeModel(engine->cardManager(), this);
    m_cardModel = new CardTableModel(engine->cardManager(), this);
    m_previousId = 0;
    // 监听数据变化，刷新树模型
    //connect(engine->cardManager(), &CardManager::dataChanged, m_treeModel, &TreeModel::refresh);

    connect(engine->cardManager(), &CardManager::dataChanged, this, [this]() {
        saveExpandedState();        // 保存当前展开状态
        m_treeModel->refresh();     // 刷新模型（会触发视图重置）
        restoreExpandedState();     // 恢复展开状态
    });
    // 左侧树视图
    m_treeView = new QTreeView;
    m_treeView->setModel(m_treeModel);
    m_treeView->setHeaderHidden(true);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->expandAll();

    // 右侧面板
    m_rightPanel = new QWidget;
    auto* rightLayout = new QVBoxLayout(m_rightPanel);
    
    m_unitStatsLabel = new QLabel(tr("请选择一个单元"));
    m_unitStatsLabel->setStyleSheet("background: #3b3b3b; padding: 8px; border-radius: 4px;");
    rightLayout->addWidget(m_unitStatsLabel);

    m_cardView = new QTableView;
    m_cardView->setModel(m_cardModel);
    m_cardView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cardView->horizontalHeader()->setStretchLastSection(true);
    m_cardView->setAlternatingRowColors(true);
    rightLayout->addWidget(m_cardView);

    // 按钮布局：第一行是新建单元/文件夹，第二行是卡片操作
    auto* topBtnLayout = new QHBoxLayout;
    m_newUnitBtn = new QPushButton(tr("📁 新建单元"));
    m_newFolderBtn = new QPushButton(tr("📂 新建文件夹"));
    topBtnLayout->addWidget(m_newUnitBtn);
    topBtnLayout->addWidget(m_newFolderBtn);
    topBtnLayout->addStretch();
    rightLayout->addLayout(topBtnLayout);
    
    auto* cardBtnLayout = new QHBoxLayout;
    m_addCardBtn = new QPushButton(tr("+ 添加卡片"));
    auto* editBtn = new QPushButton(tr("✏️ 编辑卡片"));
    auto* deleteBtn = new QPushButton(tr("🗑️ 删除卡片"));
    m_batchBtn = new QPushButton(tr("📋 批量导入"));
    cardBtnLayout->addWidget(m_addCardBtn);
    cardBtnLayout->addWidget(editBtn);
    cardBtnLayout->addWidget(deleteBtn);
    cardBtnLayout->addWidget(m_batchBtn);
    cardBtnLayout->addStretch();
    rightLayout->addLayout(cardBtnLayout);

    // 分割器
    m_splitter = new QSplitter(Qt::Horizontal);
    m_splitter->addWidget(m_treeView);
    m_splitter->addWidget(m_rightPanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_splitter);

    // 信号连接
    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &LibraryWidget::onTreeSelectionChanged);
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &LibraryWidget::onTreeContextMenu);
    connect(m_newUnitBtn, &QPushButton::clicked, this, &LibraryWidget::onAddUnit);
    connect(m_newFolderBtn, &QPushButton::clicked, this, &LibraryWidget::onAddFolder);
    connect(m_addCardBtn, &QPushButton::clicked, this, &LibraryWidget::onAddCard);
    connect(m_batchBtn, &QPushButton::clicked, this, &LibraryWidget::onBatchImport);
    connect(editBtn, &QPushButton::clicked, this, &LibraryWidget::onEditCard);
    connect(deleteBtn, &QPushButton::clicked, this, &LibraryWidget::onDeleteCard);

    connect(m_treeView, &QTreeView::clicked, this, &LibraryWidget::onTreeItemClicked);
}

void LibraryWidget::saveExpandedState() {
    m_expandedPaths.clear();
    
    std::function<void(const QModelIndex&, QString)> traverse = [&](const QModelIndex& parent, QString currentPath) {
        int rows = m_treeModel->rowCount(parent);
        for (int i = 0; i < rows; ++i) {
            QModelIndex idx = m_treeModel->index(i, 0, parent);
            if (!idx.isValid()) continue;
            
            // 获取节点显示文本（例如 "[文件夹] 根目录"）
            QString nodeText = m_treeModel->data(idx, Qt::DisplayRole).toString();
            QString nodePath = currentPath.isEmpty() ? nodeText : currentPath + "/" + nodeText;
            
            if (m_treeView->isExpanded(idx)) {
                m_expandedPaths.insert(nodePath);
                traverse(idx, nodePath);
            }
        }
    };
    
    traverse(m_treeView->rootIndex(), "");
}

void LibraryWidget::restoreExpandedState() {
    std::function<void(const QModelIndex&, QString)> traverse = [&](const QModelIndex& parent, QString currentPath) {
        int rows = m_treeModel->rowCount(parent);
        for (int i = 0; i < rows; ++i) {
            QModelIndex idx = m_treeModel->index(i, 0, parent);
            if (!idx.isValid()) continue;
            
            QString nodeText = m_treeModel->data(idx, Qt::DisplayRole).toString();
            QString nodePath = currentPath.isEmpty() ? nodeText : currentPath + "/" + nodeText;
            
            if (m_expandedPaths.contains(nodePath)) {
                m_treeView->expand(idx);
                traverse(idx, nodePath);
            }
        }
    };
    
    traverse(m_treeView->rootIndex(), "");
}

void LibraryWidget::onTreeSelectionChanged(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    int nodeIdx = index.internalId();
    const auto& node = AppEngine::instance().cardManager()->getNode(nodeIdx);
    
    if (node.type == TreeNodeType::Unit) {
        m_rightPanel->setVisible(true);
        m_cardModel->setUnitIndex(nodeIdx);
        int cardCount = node.cardIds.size();
        m_unitStatsLabel->setText(tr("单元: %1 | 卡片数: %2")
            .arg(QString::fromStdString(node.name))
            .arg(cardCount));
    } else {
        // 文件夹节点
        m_rightPanel->setVisible(true);
        m_cardModel->setUnitIndex(-1);
        m_unitStatsLabel->setText(tr("文件夹: %1\n右键或使用上方按钮创建子项")
            .arg(QString::fromStdString(node.name)));
    }
}

void LibraryWidget::onTreeItemClicked(const QModelIndex &index) {
    // 如果点击的索引就是当前选中的索引，则取消选中
    if (m_previousId == index.internalId()) {
        m_previousId = 0;
        m_treeView->clearSelection();
        m_treeView->setCurrentIndex(QModelIndex());
        m_unitStatsLabel->setText(tr("请选择一个单元"));
        m_cardModel->setUnitIndex(-1);
    }
    else{
        m_previousId = index.internalId();
    }
    return;
}

void LibraryWidget::onAddUnit() {
    QModelIndex idx = m_treeView->currentIndex();
    int parentIdx = idx.isValid() ? idx.internalId() : 0;
    const auto& parentNode = AppEngine::instance().cardManager()->getNode(parentIdx);
    
    if (parentNode.type != TreeNodeType::Folder) {
        QMessageBox::warning(this, tr("提示"), tr("只能在文件夹下创建单元"));
        return;
    }
    
    bool ok;
    QString name = QInputDialog::getText(this, tr("新建单元"), 
        tr("单元名称:"), QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        int newIdx = AppEngine::instance().cardManager()->createUnit(parentIdx, name.toStdString());
        if (newIdx != -1) {
            saveExpandedState();        // 保存当前展开状态
            m_treeModel->refresh();     // 刷新模型（会触发视图重置）
            restoreExpandedState();
        } else {
            QMessageBox::warning(this, tr("错误"), tr("创建失败，可能名称重复或路径错误"));
        }
    }
}

void LibraryWidget::onAddFolder() {
    QModelIndex idx = m_treeView->currentIndex();
    int parentIdx = idx.isValid() ? idx.internalId() : 0;
    const auto& parentNode = AppEngine::instance().cardManager()->getNode(parentIdx);
    
    if (parentNode.type != TreeNodeType::Folder) {
        QMessageBox::warning(this, tr("提示"), tr("只能在文件夹下创建子文件夹"));
        return;
    }
    
    bool ok;
    QString name = QInputDialog::getText(this, tr("新建文件夹"), 
        tr("文件夹名称:"), QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        int newIdx = AppEngine::instance().cardManager()->createFolder(parentIdx, name.toStdString());
        if (newIdx != -1) {
            saveExpandedState();        // 保存当前展开状态
            m_treeModel->refresh();     // 刷新模型（会触发视图重置）
            restoreExpandedState();
        } else {
            QMessageBox::warning(this, tr("错误"), tr("创建失败，可能名称重复"));
        }
    }
}

void LibraryWidget::onAddCard() {
    QModelIndex idx = m_treeView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个单元"));
        return;
    }
    int nodeIdx = idx.internalId();
    const auto& node = AppEngine::instance().cardManager()->getNode(nodeIdx);
    if (node.type != TreeNodeType::Unit) {
        QMessageBox::warning(this, tr("提示"), tr("请选择一个单元"));
        return;
    }

    EditCardDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString question = dlg.question();
        QString answer = dlg.answer();
        if (question.isEmpty()) {
            QMessageBox::warning(this, tr("提示"), tr("问题不能为空"));
            return;
        }
        int newId = AppEngine::instance().cardManager()->addCardToUnit(
            nodeIdx, question.toStdString(), answer.toStdString(), true);
        if (newId != -1) {
            saveExpandedState();        // 保存当前展开状态
            m_treeModel->refresh();     // 刷新模型（会触发视图重置）
            restoreExpandedState();
            QMessageBox::information(this, tr("成功"), tr("卡片添加成功，ID: %1").arg(newId));
        }
    }
}

void LibraryWidget::onEditCard() {
    QModelIndex idx = m_cardView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一张卡片"));
        return;
    }
    int row = idx.row();
    int cid = m_cardModel->cardIdAtRow(row);
    if (cid == -1) return;
    
    const Card* card = AppEngine::instance().cardManager()->getCardById(cid);
    if (!card) return;
    
    EditCardDialog dlg(this);
    dlg.setQuestion(QString::fromStdString(card->question));
    dlg.setAnswer(QString::fromStdString(card->answer));
    if (dlg.exec() == QDialog::Accepted) {
        AppEngine::instance().cardManager()->updateCard(
            cid,
            dlg.question().toStdString(),
            dlg.answer().toStdString(),
            card->active
        );
        saveExpandedState();        // 保存当前展开状态
        m_treeModel->refresh();     // 刷新模型（会触发视图重置）
        restoreExpandedState();
    }
}

void LibraryWidget::onDeleteCard() {
    QModelIndex idx = m_cardView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一张卡片"));
        return;
    }
    int row = idx.row();
    int cid = m_cardModel->cardIdAtRow(row);
    if (cid == -1) return;
    
    if (QMessageBox::question(this, tr("确认删除"), 
        tr("确定要删除这张卡片吗？")) == QMessageBox::Yes) {
        QModelIndex treeIdx = m_treeView->currentIndex();
        if (!treeIdx.isValid()) return;
        int nodeIdx = treeIdx.internalId();
        AppEngine::instance().cardManager()->removeCardFromUnit(nodeIdx, cid);
        saveExpandedState();        // 保存当前展开状态
        m_treeModel->refresh();     // 刷新模型（会触发视图重置）
        restoreExpandedState();
    }
}

void LibraryWidget::onBatchImport() {
    QModelIndex idx = m_treeView->currentIndex();
    if (!idx.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个单元"));
        return;
    }
    int nodeIdx = idx.internalId();
    const auto& node = AppEngine::instance().cardManager()->getNode(nodeIdx);
    if (node.type != TreeNodeType::Unit) {
        QMessageBox::warning(this, tr("提示"), tr("请选择一个单元"));
        return;
    }

    BatchImportDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        auto pairs = dlg.importPairs();
        if (pairs.isEmpty()) {
            QMessageBox::warning(this, tr("提示"), tr("没有有效的卡片数据"));
            return;
        }
        int count = 0;
        for (const auto& p : pairs) {
            int newId = AppEngine::instance().cardManager()->addCardToUnit(
                nodeIdx, p.first.toStdString(), p.second.toStdString(), true);
            if (newId != -1) count++;
        }
        saveExpandedState();        // 保存当前展开状态
        m_treeModel->refresh();     // 刷新模型（会触发视图重置）
        restoreExpandedState();
        QMessageBox::information(this, tr("成功"), tr("成功导入 %1 张卡片").arg(count));
    }
}

void LibraryWidget::onTreeContextMenu(const QPoint& pos) {
    QModelIndex idx = m_treeView->indexAt(pos);
    QMenu menu(this);
    
    if (idx.isValid()) {
        int nodeIdx = idx.internalId();
        const auto& node = AppEngine::instance().cardManager()->getNode(nodeIdx);
        if (node.type == TreeNodeType::Folder) {
            menu.addAction(tr("新建单元"), this, &LibraryWidget::onAddUnit);
            menu.addAction(tr("新建文件夹"), this, &LibraryWidget::onAddFolder);
            menu.addSeparator();
            // 可添加重命名/删除（需实现）
        } else if (node.type == TreeNodeType::Unit) {
            menu.addAction(tr("重命名"), [this, nodeIdx]() {
                // 实现重命名
            });
            menu.addAction(tr("删除单元"), [this, nodeIdx]() {
                // 实现删除
            });
        }
    } else {
        // 空白区域右键 -> 根目录
        menu.addAction(tr("新建单元"), this, &LibraryWidget::onAddUnit);
        menu.addAction(tr("新建文件夹"), this, &LibraryWidget::onAddFolder);
    }
    menu.exec(m_treeView->viewport()->mapToGlobal(pos));
}