#ifndef REVIEWWIDGET_H
#define REVIEWWIDGET_H

#include <QWidget>
#include <QTableView>
#include "models/ReviewTableModel.h"

class ReviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ReviewWidget(QWidget* parent = nullptr);

    // 加载完整统计数据集（正常查阅）
    void loadDataset(const StatisticsDataset& ds);

    // 加载临时统计（抽查中或结算时使用）
    void setTempStats(const std::unordered_map<int, CardStats>& tempStats, const std::vector<int>& cardIds);

    // 清空当前显示
    void clear();
protected:
    void showEvent(QShowEvent *event) override;
private:
    ReviewTableModel* m_model;
    QTableView* m_tableView;
};

#endif // REVIEWWIDGET_H