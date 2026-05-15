#ifndef BATCHIMPORTDIALOG_H
#define BATCHIMPORTDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QVector>
#include <QPair>

class BatchImportDialog : public QDialog {
    Q_OBJECT
public:
    explicit BatchImportDialog(QWidget* parent = nullptr);
    QVector<QPair<QString, QString>> importPairs() const;

private:
    QTextEdit* m_textEdit;
};

#endif // BATCHIMPORTDIALOG_H