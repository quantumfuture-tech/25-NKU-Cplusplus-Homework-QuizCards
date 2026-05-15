#ifndef APPENGINE_H
#define APPENGINE_H

#include <QObject>
#include <memory>
#include "core/CardManager.h"
#include "core/StatisticsManager.h"

class AppEngine : public QObject {
    Q_OBJECT
public:
    static AppEngine& instance();
    bool initialize();

    CardManager* cardManager() { return m_cardManager.get(); }
    StatisticsManager* statsManager() { return m_statsManager.get(); }

private:
    AppEngine() = default;
    std::unique_ptr<CardManager> m_cardManager;
    std::unique_ptr<StatisticsManager> m_statsManager;
};

#endif // APPENGINE_H