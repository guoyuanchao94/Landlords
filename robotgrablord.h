#ifndef ROBOTGRABLORD_H
#define ROBOTGRABLORD_H

#include <QThread>
#include <QObject>
#include "player.h"

//机器人玩家抢地主线程
class RobotGrabLord : public QThread
{
    Q_OBJECT
public:
    RobotGrabLord(Player *player, QObject *parent=nullptr);
protected:
    void run() override;
private:
    Player *m_player;
};

#endif // ROBOTGRABLORD_H
