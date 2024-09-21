#ifndef USERPLAYER_H
#define USERPLAYER_H

#include "player.h"
#include <QObject>

class UserPlayer : public Player
{
    Q_OBJECT
public:
    using Player::Player; //继承构造函数
    explicit UserPlayer(QObject *parent = nullptr);
public:
    void prepareCallLord() override;
    void preparePlayHand() override;
signals:
    void startCountDown();  //开始倒计时信号
};

#endif // USERPLAYER_H
