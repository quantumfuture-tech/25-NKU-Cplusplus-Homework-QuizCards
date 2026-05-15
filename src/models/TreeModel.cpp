#include "TreeModel.h"

TreeModel::TreeModel(CardManager* mgr, QObject* parent)
    : QAbstractItemModel(parent), m_mgr(mgr) {}

void TreeModel::refresh() {
    beginResetModel();
    endResetModel();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();
    
    int parentIdx = 0;  // 默认根节点索引为0
    if (parent.isValid()) {
        parentIdx = parent.internalId();
    }
    
    const auto& nodes = m_mgr->getNodes();
    if (parentIdx >= static_cast<int>(nodes.size())) return QModelIndex();
    
    const auto& parentNode = nodes[parentIdx];
    if (row < 0 || row >= static_cast<int>(parentNode.childrenIndices.size())) return QModelIndex();
    
    int childIdx = parentNode.childrenIndices[row];
    return createIndex(row, column, childIdx);
}

QModelIndex TreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) return QModelIndex();
    int childNodeIdx = child.internalId();
    const auto& nodes = m_mgr->getNodes();
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (int c : nodes[i].childrenIndices) {
            if (c == childNodeIdx) {
                return createIndex(static_cast<int>(i), 0, static_cast<int>(i));
            }
        }
    }
    return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex &parent) const {
    int nodeIdx = parent.isValid() ? parent.internalId() : 0;
    const auto& nodes = m_mgr->getNodes();
    if (nodeIdx >= static_cast<int>(nodes.size())) return 0;
    return static_cast<int>(nodes[nodeIdx].childrenIndices.size());
}

int TreeModel::columnCount(const QModelIndex &) const { return 1; }

QVariant TreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    int nodeIdx = index.internalId();
    const auto& nodes = m_mgr->getNodes();
    if (nodeIdx >= static_cast<int>(nodes.size())) return QVariant();
    const auto& node = nodes[nodeIdx];
    if (role == Qt::DisplayRole) {
        QString name = QString::fromStdString(node.name);
        if (node.type == TreeNodeType::Folder) return tr("[文件夹] %1").arg(name);
        else return tr("[单元] %1").arg(name);
    } else if (role == Qt::CheckStateRole) {
        return node.active ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::CheckStateRole) {
        int nodeIdx = index.internalId();
        bool newActive = (value.toInt() == Qt::Checked);
        // 递归设置该节点及其所有后代
        m_mgr->setNodeActiveRecursively(nodeIdx, newActive);
        // 通知视图数据变化（整个树需要刷新，因为许多节点的复选框可能改变）
        emit dataChanged(index, this->index(0, 0, QModelIndex())); // 简化：刷新全树
        return true;
    }
    return false;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}