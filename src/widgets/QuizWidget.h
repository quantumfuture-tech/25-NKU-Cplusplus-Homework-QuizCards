#ifndef QUIZWIDGET_H
#define QUIZWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "core/QuizController.h"
#include <QComboBox>
#include <QSpinBox>
#include<QProgressBar>
#include<QLabel>

class QuizWidget : public QWidget {
    Q_OBJECT
public:
    explicit QuizWidget(QWidget* parent = nullptr);
    ~QuizWidget();

private slots:
    void onStartQuiz();
    void onAnswerCorrect();
    void onAnswerWrong();
    void onForceFinish();
    
private:

    bool configBptMaker();
    void showConfigPage();
    void showQuizPage();
    void updateCardDisplay();
    void updateProgress();
    void QuizFinished();

    QuizConfig m_config;
    QStackedWidget* m_stacked;
    QWidget* m_configPage;
    QWidget* m_quizPage;
    QComboBox* m_sourceCombo;
    QComboBox* m_modeCombo;
    //抽查方式1设置组件
    QWidget* m_mode1Params;
    //抽查方式2设置组件
    QWidget* m_mode2Params;
    QComboBox* m_groupMethodCombo;
    QSpinBox* m_groupValueSpin;
    QSpinBox* m_masterySpin;
    QProgressBar* m_progressBar;
    QLabel* m_questionLabel;
    std::unique_ptr<QuizSession> m_session;
};

#endif // QUIZWIDGET_H