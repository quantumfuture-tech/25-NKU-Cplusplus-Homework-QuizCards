#include "ReviewWidget.h"
#include "AppEngine.h"
#include "widgets/dialogs/FilterThresholdDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QShowEvent>

ReviewWidget::ReviewWidget(QWidget* parent) : QWidget(parent) {
    m_model = new ReviewTableModel(AppEngine::instance().cardManager(), this);
    m_tableView = new QTableView;
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setAlternatingRowColors(true);

    /*
    // 加载默认数据：聚合所有统计会话，如果没有则显示空
    auto statsMgr = AppEngine::instance().statsManager();
    auto allStats = statsMgr->aggregateFolder(statsMgr->rootPath());
    if (!allStats.cardIds.empty()) {
        m_model->setDataset(allStats);
    } else {
        // 若没有统计记录，可以显示当前所有激活卡片的信息（仅学习情况为0）
        StatisticsDataset emptyDs;
        emptyDs.name = "暂无统计";
        emptyDs.cardIds = AppEngine::instance().cardManager()->getActiveCardIds();
        m_model->setDataset(emptyDs);
    }
    */
    // 筛选栏
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QWidget* filterBar = new QWidget;
    QHBoxLayout* filterLayout = new QHBoxLayout(filterBar);
    filterLayout->setContentsMargins(0,0,0,0);

    auto addFilterButton = [&](const QString& text, const QString& filterType, double minRate=0.0, double maxRate=1.0, int minAttempts=0) {
        QPushButton* btn = new QPushButton(text);
        connect(btn, &QPushButton::clicked, this, [this, filterType, minRate, maxRate, minAttempts]() {
            m_model->setFilter(filterType, minRate, maxRate, minAttempts);
        });
        filterLayout->addWidget(btn);
    };

    addFilterButton(tr("全部"), "all");
    addFilterButton(tr("学习次数＜3"), "attempts_lt_3");
    addFilterButton(tr("正确率＜50%"), "accuracy_lt_50");
    addFilterButton(tr("正确率50%～80%"), "accuracy_50_80");
    addFilterButton(tr("正确率＞80%"), "accuracy_gt_80");
    addFilterButton(tr("上次错误"), "last_wrong");
    addFilterButton(tr("未学习"), "unlearned");

    QPushButton* customBtn = new QPushButton(tr("自定义..."));
    connect(customBtn, &QPushButton::clicked, this, [this]() {
        FilterThresholdDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            m_model->setFilter("custom", dlg.minAccuracy(), dlg.maxAccuracy(), dlg.minAttempts());
        }
    });
    filterLayout->addWidget(customBtn);
    filterLayout->addStretch();

    scrollArea->setWidget(filterBar);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);
    layout->addWidget(m_tableView);

    connect(m_tableView, &QTableView::doubleClicked, this, [this](const QModelIndex& idx) {
        int row = idx.row();
        int cid = m_model->cardIdAtRow(row);
        if (cid != -1) {
            const Card* card = AppEngine::instance().cardManager()->getCardById(cid);
            if (card) {
                QString msg = tr("卡片ID: %1\n问题: %2\n答案: %3")
                              .arg(cid).arg(QString::fromStdString(card->question)).arg(QString::fromStdString(card->answer));
                QMessageBox::information(this, tr("卡片详情"), msg);
            }
        }
    });
}

void ReviewWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    // 每次显示时刷新数据
    auto statsMgr = AppEngine::instance().statsManager();
    auto allStats = statsMgr->aggregateFolder(statsMgr->rootPath());
    if (!allStats.cardIds.empty()) {
        m_model->setDataset(allStats);
    } else {
        // 若没有统计记录，可以显示当前所有激活卡片的信息（仅学习情况为0）
        StatisticsDataset emptyDs;
        emptyDs.name = "暂无统计";
        emptyDs.cardIds = AppEngine::instance().cardManager()->getActiveCardIds();
        m_model->setDataset(emptyDs);
    }
}

void ReviewWidget::loadDataset(const StatisticsDataset& ds) {
    m_model->setDataset(ds);
}

void ReviewWidget::setTempStats(const std::unordered_map<int, CardStats>& tempStats, const std::vector<int>& cardIds) {
    StatisticsDataset ds;
    ds.name = "临时抽查统计";
    ds.cardIds = cardIds;
    ds.stats = tempStats;
    m_model->setDataset(ds);
}

void ReviewWidget::clear() {
    m_model->clear();
}