#include "CardTableModel.h"
#include <algorithm>

CardTableModel::CardTableModel(CardManager* mgr, QObject* parent)
    : QAbstractTableModel(parent), m_mgr(mgr) {}

void CardTableModel::setUnitIndex(int unitIdx) {
    m_unitIdx = unitIdx;
    refresh();
}

void CardTableModel::refresh() {
    beginResetModel();
    if (m_unitIdx >= 0 && m_unitIdx < static_cast<int>(m_mgr->getNodes().size()) &&
        m_mgr->getNodes()[m_unitIdx].type == TreeNodeType::Unit) {
        m_cardIds = m_mgr->getNodes()[m_unitIdx].cardIds;
        updateSort();
    } else {
        m_cardIds.clear();
    }
    endResetModel();
}

void CardTableModel::updateSort() {
    // 简化排序：按正确率（从统计中获取）降序，未学习（无统计）排最前
    // 实际正确率需要从 StatisticsManager 获取，这里先不做完整聚合，留作扩展
    // 简单按ID排序
    std::sort(m_cardIds.begin(), m_cardIds.end());
}

int CardTableModel::rowCount(const QModelIndex&) const {
    return static_cast<int>(m_cardIds.size());
}

int CardTableModel::columnCount(const QModelIndex&) const { return 3; } // 问题，激活，正确率

QVariant CardTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_cardIds.size())) return QVariant();
    int cid = m_cardIds[index.row()];
    const Card* card = m_mgr->getCardById(cid);
    if (!card) return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) return QString::fromStdString(card->question);
        else if (index.column() == 2) return tr("N/A"); // 正确率占位
    } else if (role == Qt::CheckStateRole && index.column() == 1) {
        return card->active ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}

bool CardTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::CheckStateRole && index.column() == 1) {
        int cid = m_cardIds[index.row()];
        Card* card = const_cast<Card*>(m_mgr->getCardById(cid));
        if (card) {
            card->active = (value.toInt() == Qt::Checked);
            m_mgr->updateCard(cid, card->question, card->answer, card->active);
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags CardTableModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == 1) flags |= Qt::ItemIsUserCheckable;
    return flags;
}

int CardTableModel::cardIdAtRow(int row) const {
    if (row >= 0 && row < static_cast<int>(m_cardIds.size())) return m_cardIds[row];
    return -1;
}