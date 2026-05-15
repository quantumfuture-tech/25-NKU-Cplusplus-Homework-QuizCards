#ifndef REVIEWTABLEMODEL_H
#define REVIEWTABLEMODEL_H

#include <QAbstractTableModel>
#include "core/StatisticsManager.h"
#include "core/CardManager.h"

class ReviewTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit ReviewTableModel(CardManager* cardMgr, QObject* parent = nullptr);
    void setDataset(const StatisticsDataset& dataset);
    void clear();
    void setFilter(const QString& filterType, double minRate = 0.0, double maxRate = 1.0, int minAttempts = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int cardIdAtRow(int row) const;

private:
    struct DisplayRow {
        int cardId;
        QString question;
        int totalAttempts;
        int correctCount;
        double accuracy;
        bool lastResult;
    };
    CardManager* m_cardMgr;
    StatisticsDataset m_dataset;
    std::vector<DisplayRow> m_rows;
    QString m_filterType;
    double m_minRate, m_maxRate;
    int m_minAttempts;

    void rebuildRows();
    bool matchesFilter(const DisplayRow& row) const;
};

#endif // REVIEWTABLEMODEL_H