#ifndef EDITCARDDIALOG_H
#define EDITCARDDIALOG_H

#include <QDialog>
#include <QLineEdit>

class EditCardDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditCardDialog(QWidget* parent = nullptr);
    QString question() const;
    QString answer() const;
    void setQuestion(const QString& q);
    void setAnswer(const QString& a);

private:
    QLineEdit* m_questionEdit;
    QLineEdit* m_answerEdit;
};

#endif // EDITCARDDIALOG_H