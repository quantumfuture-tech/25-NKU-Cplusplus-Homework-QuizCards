#include "ReviewTableModel.h"
#include <QString>

ReviewTableModel::ReviewTableModel(CardManager* cardMgr, QObject* parent)
    : QAbstractTableModel(parent), m_cardMgr(cardMgr),
      m_filterType("all"), m_minRate(0.0), m_maxRate(1.0), m_minAttempts(0) {}

void ReviewTableModel::setDataset(const StatisticsDataset& dataset) {
    beginResetModel();
    m_dataset = dataset;
    rebuildRows();
    endResetModel();
}

void ReviewTableModel::clear() {
    beginResetModel();
    m_dataset = StatisticsDataset();
    m_rows.clear();
    endResetModel();
}

void ReviewTableModel::setFilter(const QString& filterType, double minRate, double maxRate, int minAttempts) {
    m_filterType = filterType;
    m_minRate = minRate;
    m_maxRate = maxRate;
    m_minAttempts = minAttempts;
    beginResetModel();
    rebuildRows();
    endResetModel();
}

void ReviewTableModel::rebuildRows() {
    m_rows.clear();
    for (int cid : m_dataset.cardIds) {
        const Card* card = m_cardMgr->getCardById(cid);
        if (!card) continue;
        auto it = m_dataset.stats.find(cid);
        int total = 0, correct = 0;
        bool last = false;
        if (it != m_dataset.stats.end()) {
            total = it->second.totalAttempts;
            correct = it->second.correctCount;
            last = it->second.lastResult;
        }
        double acc = (total > 0) ? (static_cast<double>(correct) / total) : 0.0;
        DisplayRow row;
        row.cardId = cid;
        row.question = QString::fromStdString(card->question);
        row.totalAttempts = total;
        row.correctCount = correct;
        row.accuracy = acc;
        row.lastResult = last;
        if (matchesFilter(row)) {
            m_rows.push_back(row);
        }
    }
}

bool ReviewTableModel::matchesFilter(const DisplayRow& row) const {
    if (m_filterType == "all") return true;
    if (m_filterType == "unlearned") return row.totalAttempts == 0;
    if (m_filterType == "attempts_lt_3") return row.totalAttempts < 3;
    if (m_filterType == "accuracy_lt_50") return row.accuracy < 0.5;
    if (m_filterType == "accuracy_50_80") return row.accuracy >= 0.5 && row.accuracy <= 0.8;
    if (m_filterType == "accuracy_gt_80") return row.accuracy > 0.8;
    if (m_filterType == "last_wrong") return !row.lastResult && row.totalAttempts > 0;
    if (m_filterType == "custom") {
        if (row.totalAttempts < m_minAttempts) return false;
        if (row.accuracy < m_minRate || row.accuracy > m_maxRate) return false;
        return true;
    }
    return true;
}

int ReviewTableModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(m_rows.size());
}

int ReviewTableModel::columnCount(const QModelIndex&) const {
    return 4; // 问题 | 正确/总次数 | 正确率 | 最近结果
}

QVariant ReviewTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_rows.size()))
        return QVariant();
    const auto& row = m_rows[index.row()];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return row.question;
        case 1: return QString("%1/%2").arg(row.correctCount).arg(row.totalAttempts);
        case 2: return QString::number(row.accuracy * 100, 'f', 1) + "%";
        case 3: return row.lastResult ? tr("正确") : (row.totalAttempts > 0 ? tr("错误") : tr("未学习"));
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() >= 1) return Qt::AlignCenter;
    }
    return QVariant();
}

QVariant ReviewTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("问题");
        case 1: return tr("正确/总次数");
        case 2: return tr("正确率");
        case 3: return tr("最近结果");
        }
    }
    return QVariant();
}

int ReviewTableModel::cardIdAtRow(int row) const {
    if (row >= 0 && row < static_cast<int>(m_rows.size()))
        return m_rows[row].cardId;
    return -1;
}