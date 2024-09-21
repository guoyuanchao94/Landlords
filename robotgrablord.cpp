#include "robotgrablord.h"
#include <QDebug>
RobotGrabLord::RobotGrabLord(Player *player,QObject *parent):QThread(parent)
{
    m_player=player;
}

void RobotGrabLord::run()
{
    msleep(2000); //避免叫地主动作完成过快,模拟思考环节
    m_player->thinkCallLord();
    qDebug()<<"机器人玩家叫地主线程启动";
}
