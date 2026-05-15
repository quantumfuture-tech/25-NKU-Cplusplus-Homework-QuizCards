#ifndef FILTERTHRESHOLDDIALOG_H
#define FILTERTHRESHOLDDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>

class FilterThresholdDialog : public QDialog {
    Q_OBJECT
public:
    explicit FilterThresholdDialog(QWidget* parent = nullptr);
    double minAccuracy() const;
    double maxAccuracy() const;
    int minAttempts() const;

private:
    QDoubleSpinBox* m_minAccuracySpin;
    QDoubleSpinBox* m_maxAccuracySpin;
    QSpinBox* m_minAttemptsSpin;
};

#endif // FILTERTHRESHOLDDIALOG_H