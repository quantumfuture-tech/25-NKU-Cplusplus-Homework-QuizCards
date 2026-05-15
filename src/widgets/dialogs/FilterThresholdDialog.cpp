#include "FilterThresholdDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

FilterThresholdDialog::FilterThresholdDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("自定义筛选"));
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("最低正确率 (0-100):")));
    m_minAccuracySpin = new QDoubleSpinBox;
    m_minAccuracySpin->setRange(0, 100);
    m_minAccuracySpin->setSuffix("%");
    layout->addWidget(m_minAccuracySpin);

    layout->addWidget(new QLabel(tr("最高正确率 (0-100):")));
    m_maxAccuracySpin = new QDoubleSpinBox;
    m_maxAccuracySpin->setRange(0, 100);
    m_maxAccuracySpin->setValue(100);
    m_maxAccuracySpin->setSuffix("%");
    layout->addWidget(m_maxAccuracySpin);

    layout->addWidget(new QLabel(tr("最少学习次数:")));
    m_minAttemptsSpin = new QSpinBox;
    m_minAttemptsSpin->setRange(0, 9999);
    layout->addWidget(m_minAttemptsSpin);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

double FilterThresholdDialog::minAccuracy() const {
    return m_minAccuracySpin->value() / 100.0;
}
double FilterThresholdDialog::maxAccuracy() const {
    return m_maxAccuracySpin->value() / 100.0;
}
int FilterThresholdDialog::minAttempts() const {
    return m_minAttemptsSpin->value();
}

