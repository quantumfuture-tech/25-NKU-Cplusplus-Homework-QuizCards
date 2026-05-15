#include "QuizWidget.h"
#include "AppEngine.h"
#include "core/Debug.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QProgressBar>
#include <QDebug>

QuizWidget::QuizWidget(QWidget* parent) : QWidget(parent) {
    //栈视图组件
    m_stacked = new QStackedWidget(this);
    
    // ========== 配置页面 ==========
    m_configPage = new QWidget;
    auto* configLayout = new QVBoxLayout(m_configPage);
    
    // 题目来源分组
    QGroupBox* sourceGroup = new QGroupBox(tr("题目来源"));
    QVBoxLayout* sourceLayout = new QVBoxLayout(sourceGroup);
    m_sourceCombo = new QComboBox;
    m_sourceCombo->addItems({tr("当前激活题库"), tr("抽查筐"), tr("自定义标签")});
    sourceLayout->addWidget(m_sourceCombo);
    configLayout->addWidget(sourceGroup);
    connect(AppEngine::instance().cardManager(), &CardManager::dataChanged, this,
        [this]() {
            m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
            int max = 1;
            int min = 1;
            switch (m_sourceCombo->currentIndex()) {
                case 0:
                m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
                break;
                default:
                m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
                break;
            }
            max = m_config.sourceCardIds.size();
            if (m_config.dividemode == DivideMode::Bypercentage) {
                max = 100;
                min = std::ceil(100 / m_config.sourceCardIds.size());
            }
            m_groupValueSpin->setRange(min, max);
            m_groupValueSpin->setValue(min);
            m_config.requiredValue = min;
            
        }
    );
    connect(m_sourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int idx) {
            m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
            int max = 1;
            int min = 1;
            switch (idx) {
                case 0:
                m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
                break;
                default:
                m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
                break;
            }
            max = m_config.sourceCardIds.size();
            if (m_config.dividemode == DivideMode::Bypercentage) {
                max = 100;
                min = std::ceil(100 / m_config.sourceCardIds.size());
            }
            m_groupValueSpin->setRange(min, max);
            m_groupValueSpin->setValue(min);
            m_config.requiredValue = min;
            
        }
    );
    
    //设置抽查设置、卡片集合
    m_config.sourceCardIds = AppEngine::instance().cardManager()->getActiveCardIds();
    m_config.requiredValue = 1;
    m_config.dividemode = DivideMode::Bycards;
    // 抽查模式分组
    QGroupBox* modeGroup = new QGroupBox(tr("抽查模式"));
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
    m_modeCombo = new QComboBox;
    m_modeCombo->addItems({tr("随机分组"), tr("达标递归")});
    modeLayout->addWidget(m_modeCombo);
    
    // 动态参数区域（模式一参数）
    m_mode1Params = new QWidget(this);
    QHBoxLayout* param1Layout = new QHBoxLayout(m_mode1Params);
    QLabel* groupMethodLabel = new QLabel(tr("分组方式:"));
    m_groupMethodCombo = new QComboBox;
    m_groupMethodCombo->addItems({tr("每组卡片数"), tr("总轮数"), tr("百分比")});
    m_groupValueSpin = new QSpinBox;
    
    //初始化spin
    m_groupValueSpin->setRange(1, m_config.sourceCardIds.size());
    m_groupValueSpin->setValue(1);



    param1Layout->addWidget(groupMethodLabel);
    param1Layout->addWidget(m_groupMethodCombo);
    param1Layout->addWidget(m_groupValueSpin);
    param1Layout->addStretch();
    modeLayout->addWidget(m_mode1Params);
    connect(m_groupValueSpin, &QSpinBox::valueChanged, this, 
        [this](int val){
            m_config.requiredValue = val;
        }
    );

    connect(m_groupMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int idx) {
            int max = m_config.sourceCardIds.size();
            int min = 1;
            if (idx == 2) {
                max = 100;
                min = std::ceil(100 / m_config.sourceCardIds.size());
            }
            m_groupValueSpin->setRange(min, max);
            m_groupValueSpin->setValue(min);
            m_config.requiredValue = min;
            switch (idx){
                case 0:
                m_config.dividemode = DivideMode::Bycards;
                break;
                case 1:
                m_config.dividemode = DivideMode::Bygroups;
                break;
                case 2:
                m_config.dividemode = DivideMode::Bypercentage;
                break;
            }
        }
    );
    // 模式二参数
    m_mode2Params = new QWidget(this);
    QHBoxLayout* param2Layout = new QHBoxLayout(m_mode2Params);
    QLabel* masteryLabel = new QLabel(tr("达标所需连续正确次数:"));
    m_masterySpin = new QSpinBox;
    m_masterySpin->setRange(1, 10);
    m_masterySpin->setValue(1);
    param2Layout->addWidget(masteryLabel);
    param2Layout->addWidget(m_masterySpin);
    param2Layout->addStretch();
    modeLayout->addWidget(m_mode2Params);
    
    connect(m_masterySpin, &QSpinBox::valueChanged, this,
        [this](int val) {
            m_config.requiredValue = val;
        });
    // 根据模式切换显示参数区域
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int idx) {
            m_mode1Params->setVisible(idx == 0);
            m_mode2Params->setVisible(idx == 1);
            m_config.quizmode = idx ? QuizMode::MasteryRecursive : QuizMode::RandomGroup;
        });
    m_mode1Params->setVisible(true);//初始显示抽查1设置组件
    m_mode2Params->setVisible(false);
    
    modeLayout->addStretch();
    configLayout->addWidget(modeGroup);
    
    // 开始按钮
    QPushButton* startBtn = new QPushButton(tr("开始抽查"));
    configLayout->addWidget(startBtn);
    configLayout->addStretch();
    
    connect(startBtn, &QPushButton::clicked, this, &QuizWidget::onStartQuiz);
    
    // ========== 抽查页面 ==========
    m_quizPage = new QWidget;
    auto* quizLayout = new QVBoxLayout(m_quizPage);
    m_progressBar = new QProgressBar;
    m_questionLabel = new QLabel(tr("点击卡片显示答案"),m_quizPage);
    m_questionLabel->setAlignment(Qt::AlignCenter);
    m_questionLabel->setStyleSheet("background: lightblue; padding: 20px; font-size: 18px; border-radius: 10px;");
    m_questionLabel->setMinimumHeight(200);
    QPushButton* correctBtn = new QPushButton(tr("正确"));
    QPushButton* wrongBtn = new QPushButton(tr("错误"));
    QPushButton* finishBtn = new QPushButton(tr("提前结束"));
    quizLayout->addWidget(m_progressBar);
    quizLayout->addWidget(m_questionLabel);
    quizLayout->addWidget(correctBtn);
    quizLayout->addWidget(wrongBtn);
    quizLayout->addWidget(finishBtn);
    
    connect(correctBtn, &QPushButton::clicked, this, &QuizWidget::onAnswerCorrect);
    connect(wrongBtn, &QPushButton::clicked, this, &QuizWidget::onAnswerWrong);
    connect(finishBtn, &QPushButton::clicked, this, &QuizWidget::onForceFinish);
    
    m_stacked->addWidget(m_configPage);
    m_stacked->addWidget(m_quizPage);
    m_stacked->setCurrentWidget(m_configPage);
    
    auto* mainLayout = new QVBoxLayout(this);//本界面布局
    mainLayout->addWidget(m_stacked);
}

QuizWidget::~QuizWidget() {
    // 确保会话被正确释放
    qDebug() << "QuizWidget析构";
}

void QuizWidget::onStartQuiz() {
    // 抽查的开始，从配置构造 QuizSession
    qDebug()<<"抽查按钮触发";
    if(configBptMaker()){
        qDebug()<<"断点设置成功！";
    }
    //创建m_session单例
    qDebug()<<"创建m_session单例";
    m_session = std::make_unique<QuizSession>(m_config, AppEngine::instance().cardManager(), this);
    if (m_session->start()) {//开始抽查
        showQuizPage();//切换显示栈
        //更新card文字
        updateCardDisplay();
        updateProgress();
    } else {
        QMessageBox::warning(this, tr("错误"), tr("无法开始抽查，请检查卡片数量"));
    }
}

bool QuizWidget::configBptMaker() {
    qDebug()<<"试图设置断点";
    if (m_config.quizmode != QuizMode::RandomGroup) {return false;}
    m_config.breakPoint.clear();
    int totalSize = static_cast<int>(m_config.sourceCardIds.size());
    qDebug()<<"获取总卡片数："<<totalSize;
    int groupSize;
    int bpt = 0;
    switch (m_config.dividemode) {
        //按每轮卡片数
        case DivideMode::Bycards:
        groupSize = m_config.requiredValue;
        qDebug()<<"按卡片数："<<groupSize;
        bpt = groupSize;
        while(bpt <= totalSize) {
            m_config.breakPoint.push_back(bpt);
            qDebug()<<"设置断点："<<bpt;
            bpt += groupSize;
        }
        break;
        //按轮数
        case DivideMode::Bygroups:
        groupSize = std::ceil(double(totalSize / m_config.requiredValue));
        qDebug()<<"按轮数："<<m_config.requiredValue<<"每轮卡片："<<groupSize;
        bpt = groupSize;
        while (bpt <= totalSize) {
            m_config.breakPoint.push_back(bpt);
            qDebug()<<"设置断点："<<bpt;
            bpt += groupSize;
        }
        break;
        //按百分数
        case DivideMode::Bypercentage:
        for (int i = 1; m_config.requiredValue * i <= 100; i++) {
            bpt = std::ceil(double(totalSize * m_config.requiredValue * i) / 100);
            qDebug()<<totalSize<<"*"<< m_config.requiredValue<<"*"<< i<<"/"<< 100<<"="<<bpt;
            m_config.breakPoint.push_back(bpt);
            qDebug()<<"设置断点："<<bpt;
        }
        break;
    }
    if (m_config.breakPoint.back() < totalSize){
        m_config.breakPoint.push_back(totalSize);
        qDebug()<<"设置断点："<<totalSize;
    }
    return true;
}

void QuizWidget::showConfigPage() {
    m_stacked->setCurrentWidget(m_configPage);
}

void QuizWidget::showQuizPage() {
    m_stacked->setCurrentWidget(m_quizPage);
}
//更新卡片文字，需保证m_session存在
void QuizWidget::updateCardDisplay() {
    qDebug()<<"更新卡片文字";
    auto opt = m_session->currentCard();
    if (opt) {
        qDebug()<<"成功获取卡片";
        m_questionLabel->setText(QString::fromStdString(opt->question));
    }
    else{
        myDebug()<<"已经无卡片";
    }
}
//更新进度条，需保证m_session存在
void QuizWidget::updateProgress() {
    qDebug()<<"更新进度条";
    auto [round, totalRounds, answered, roundTotal] = m_session->getProgress();
    if (roundTotal == 0){
        qDebug() << "该轮无卡片！";
        return ;
    }
    m_progressBar->setMaximum(roundTotal);
    m_progressBar->setValue(answered);
    m_progressBar->setFormat(tr("第 %1 / %2 轮 - 进度 %3/%4").arg(round).arg(totalRounds).arg(answered).arg(roundTotal));
    qDebug() << "更新成功";
}

void QuizWidget::onAnswerCorrect() {
    if(m_session->recordResult(true)){//回答正确，更新进度，卡片
        updateProgress(),updateCardDisplay();
    }
    else{//结束时仅更新进度
        updateProgress();
        QuizFinished();
    }
    
}

void QuizWidget::onAnswerWrong() {
    if(m_session->recordResult(false)){//回答正确，更新进度，卡片
        updateProgress(),updateCardDisplay();
    }
    else{//结束时仅更新进度
        updateProgress();
        QuizFinished();
    }
}

void QuizWidget::onForceFinish() {
    if (QMessageBox::question(this, tr("确认"), tr("提前结束抽查？")) == QMessageBox::Yes) {
        QuizFinished();
    }
}

void QuizWidget::QuizFinished() {
    qDebug() << "！试图保存统计结果！";
    auto dataset = m_session->finalize();
    AppEngine::instance().statsManager()->saveSession(dataset);
    QMessageBox::information(this, tr("抽查结束"), tr("统计已保存"));
    showConfigPage();
    m_session.reset();
}