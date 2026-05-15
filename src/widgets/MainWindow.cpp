#include "MainWindow.h"
#include "LibraryWidget.h"
#include "QuizWidget.h"
#include "ReviewWidget.h"
#include "StatsManagerWidget.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_tabWidget = new QTabWidget;
    m_tabWidget->addTab(new LibraryWidget, tr("题库"));
    m_tabWidget->addTab(new QuizWidget, tr("抽查"));
    m_tabWidget->addTab(new ReviewWidget, tr("查阅"));
    m_tabWidget->addTab(new StatsManagerWidget, tr("统计管理"));
    setCentralWidget(m_tabWidget);
    resize(800, 600);
}

MainWindow::~MainWindow() {}