#include "EditCardDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

EditCardDialog::EditCardDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("编辑卡片"));
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("问题:")));
    m_questionEdit = new QLineEdit;
    layout->addWidget(m_questionEdit);
    layout->addWidget(new QLabel(tr("答案:")));
    m_answerEdit = new QLineEdit;
    layout->addWidget(m_answerEdit);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString EditCardDialog::question() const { return m_questionEdit->text(); }
QString EditCardDialog::answer() const { return m_answerEdit->text(); }

void EditCardDialog::setQuestion(const QString& q) {
    m_questionEdit->setText(q);
}
void EditCardDialog::setAnswer(const QString& a) {
    m_answerEdit->setText(a);
}