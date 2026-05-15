#include "BatchImportDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

BatchImportDialog::BatchImportDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("批量导入卡片"));
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("每行格式: 问题|答案")));
    m_textEdit = new QTextEdit;
    layout->addWidget(m_textEdit);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QVector<QPair<QString, QString>> BatchImportDialog::importPairs() const {
    QVector<QPair<QString, QString>> result;
    QString text = m_textEdit->toPlainText();
    for (const QString& line : text.split('\n', Qt::SkipEmptyParts)) {
        auto parts = line.split('|');
        if (parts.size() >= 2) {
            result.append({parts[0].trimmed(), parts[1].trimmed()});
        }
    }
    return result;
}