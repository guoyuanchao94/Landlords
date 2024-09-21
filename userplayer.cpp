#include "userplayer.h"

UserPlayer::UserPlayer(QObject *parent)
    : Player{parent}
{
    m_type=Player::User;
}

void UserPlayer::prepareCallLord()
{

}

void UserPlayer::preparePlayHand()
{
    emit startCountDown();  //通知当前玩家准备出牌,闹钟开始倒计时
}
