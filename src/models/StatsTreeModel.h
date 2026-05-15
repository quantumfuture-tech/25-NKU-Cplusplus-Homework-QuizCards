#ifndef STATSTREEMODEL_H
#define STATSTREEMODEL_H

#include <QAbstractItemModel>
#include <filesystem>
#include <vector>

class StatsTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    struct Node {
        std::string name;
        std::filesystem::path fullPath;
        bool isFolder;
        int parentIdx;
        std::vector<int> children;
    };

    explicit StatsTreeModel(QObject* parent = nullptr);
    void setRootPath(const std::filesystem::path& rootPath);
    void refresh();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    std::filesystem::path pathForIndex(const QModelIndex &index) const;
    bool isFolder(const QModelIndex &index) const;
    bool createFolder(const QModelIndex &parentIdx, const QString& folderName);
    bool renameNode(const QModelIndex &index, const QString& newName);
    bool deleteNode(const QModelIndex &index);
    bool moveNode(const QModelIndex &from, const QModelIndex &toFolder);

private:
    std::filesystem::path m_rootPath;
    std::vector<Node> m_nodes; // index 0 is root

    void rebuildTree();
    int findNodeByPath(const std::filesystem::path& path) const;
    void addDirectory(const std::filesystem::path& dirPath, int parentIdx);
    void addFile(const std::filesystem::path& filePath, int parentIdx);
};

#endif // STATSTREEMODEL_H