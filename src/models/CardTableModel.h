#ifndef CARDTABLEMODEL_H
#define CARDTABLEMODEL_H

#include <QAbstractTableModel>
#include "core/CardManager.h"

class CardTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CardTableModel(CardManager* mgr, QObject* parent = nullptr);
    void setUnitIndex(int unitIdx);
    void refresh();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int cardIdAtRow(int row) const;

private:
    CardManager* m_mgr;
    int m_unitIdx = -1;
    std::vector<int> m_cardIds; // 按正确率排序后的卡片ID
    void updateSort();
};

#endif // CARDTABLEMODEL_H