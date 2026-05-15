#include "StatsManagerWidget.h"
#include "AppEngine.h"
#include "ReviewWidget.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

StatsManagerWidget::StatsManagerWidget(QWidget* parent) : QWidget(parent) {
    m_model = new StatsTreeModel(this);
    m_model->setRootPath(AppEngine::instance().statsManager()->rootPath());
    m_treeView = new QTreeView;
    m_treeView->setModel(m_model);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QModelIndex idx = m_treeView->indexAt(pos);
        QMenu menu;
        if (idx.isValid()) {
            bool isFolder = m_model->isFolder(idx);
            if (isFolder) {
                menu.addAction(tr("新建文件夹"), this, [this, idx]() {
                    bool ok;
                    QString name = QInputDialog::getText(this, tr("新建文件夹"), tr("文件夹名:"), QLineEdit::Normal, "", &ok);
                    if (ok && !name.isEmpty()) {
                        m_model->createFolder(idx, name);
                    }
                });
                menu.addAction(tr("移动到..."), this, [this, idx]() {
                    QMessageBox::information(this, tr("移动"), tr("请选择目标文件夹（未实现完整选择器）"));
                });
            }
            menu.addAction(tr("重命名"), this, [this, idx]() {
                bool ok;
                QString newName = QInputDialog::getText(this, tr("重命名"), tr("新名称:"), QLineEdit::Normal, m_model->data(idx).toString(), &ok);
                if (ok && !newName.isEmpty()) {
                    m_model->renameNode(idx, newName);
                }
            });
            menu.addAction(tr("删除"), this, [this, idx]() {
                if (QMessageBox::question(this, tr("确认"), tr("确定删除吗？")) == QMessageBox::Yes) {
                    m_model->deleteNode(idx);
                }
            });
        } else {
            menu.addAction(tr("新建文件夹（根目录）"), this, [this]() {
                bool ok;
                QString name = QInputDialog::getText(this, tr("新建文件夹"), tr("文件夹名:"), QLineEdit::Normal, "", &ok);
                if (ok && !name.isEmpty()) {
                    m_model->createFolder(QModelIndex(), name);
                }
            });
        }
        menu.exec(m_treeView->viewport()->mapToGlobal(pos));
    });

    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex& idx) {
        if (!idx.isValid()) return;
        auto path = m_model->pathForIndex(idx);
        if (path.empty()) return;
        StatisticsDataset ds;
        if (m_model->isFolder(idx)) {
            ds = AppEngine::instance().statsManager()->aggregateFolder(path);
        } else {
            auto opt = AppEngine::instance().statsManager()->loadSession(path);
            if (!opt) {
                QMessageBox::warning(this, tr("错误"), tr("无法加载统计文件"));
                return;
            }
            ds = *opt;
        }
        QDialog* dialog = new QDialog(this);
        dialog->setWindowTitle(tr("统计详情 - ") + QString::fromStdString(path.filename().string()));
        dialog->resize(700, 500);
        auto* layout = new QVBoxLayout(dialog);
        ReviewWidget* review = new ReviewWidget(dialog);
        review->loadDataset(ds);
        layout->addWidget(review);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
}