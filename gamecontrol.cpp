#include "gamecontrol.h"
#include <QRandomGenerator>
#include <QTimer>
#include "playhand.h"
#include <QDebug>
GameControl::GameControl(QObject *parent)
    : QObject{parent}
{

}

void GameControl::playerInit()
{
    m_robotLeft = new Robot("机器人A", this);
    m_robotRight = new Robot("机器人B", this);
    m_user = new UserPlayer("我自己", this);

    //头像显示方向
    m_robotLeft->setDirection(Player::Left);
    m_robotRight->setDirection(Player::Right);
    m_user->setDirection(Player::Right);

    //随机设置性别
    Player::Sex sex;
    sex = static_cast<Player::Sex>(QRandomGenerator::global()->bounded(0,2));
    m_robotLeft->setSex(sex);
    sex = static_cast<Player::Sex>(QRandomGenerator::global()->bounded(0,2));
    m_robotRight->setSex(sex);
    sex = static_cast<Player::Sex>(QRandomGenerator::global()->bounded(0,2));
    m_user->setSex(sex);

    //设置出牌顺序
    m_user->setPrevPlayer(m_robotLeft);
    m_user->setNextPlayer(m_robotRight);

    m_robotLeft->setPrevPlayer(m_robotRight);
    m_robotLeft->setNextPlayer(m_user);

    m_robotRight->setPrevPlayer(m_user);
    m_robotRight->setNextPlayer(m_robotLeft);

    //当前玩家指定为用户玩家,拥有优先叫地主的权力
    m_currentPlayer = m_user;

    //处理玩家发射出的信号
    connect(m_user, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabBet);
    connect(m_robotLeft, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabBet);
    connect(m_robotRight, &UserPlayer::notifyGrabLordBet, this, &GameControl::onGrabBet);

    //传递出牌玩家对象和打出的牌
    connect(this, &GameControl::pendingInfo, m_robotLeft, &Robot::storePendingInfo);
    connect(this, &GameControl::pendingInfo, m_robotRight, &Robot::storePendingInfo);
    connect(this, &GameControl::pendingInfo, m_user, &Robot::storePendingInfo);

    //处理玩家出牌
    connect(m_robotLeft, &Robot::notifyPlayHand,  this, &GameControl::onPlayHand);
    connect(m_robotRight, &Robot::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_user, &Robot::notifyPlayHand, this, &GameControl::onPlayHand);
}

Robot *GameControl::getLeftRobot()
{
    return m_robotLeft;
}

Robot *GameControl::getRightRobot()
{
    return m_robotRight;
}

UserPlayer *GameControl::getUserPlayer()
{
    return m_user;
}

void GameControl::setCurrentPlayer(Player *player)
{
    m_currentPlayer = player;
}

Player *GameControl::getCurrentPlayer()
{
    return m_currentPlayer;
}

Player *GameControl::getPendPlayer()
{
    return m_pendPlayer;
}

Cards GameControl::getPendCards()
{
    return m_pendCards;
}

void GameControl::initAllCards()
{
    m_allCards.clear();
    for(int p = Card::CardBegin+1; p < Card::CardSmallJoker; ++p)
    {
        for(int s = Card::SuitBegin+1; s < Card::SuitEnd; ++s)
        {
            Card card(static_cast<Card::CardPoint>(p), static_cast<Card::CardSuit>(s));
            //扑克牌添加到QSet容器中是无序的
            m_allCards.add(card);
        }
    }
    //大小王不分配点数
    m_allCards.add(Card(Card::CardSmallJoker, Card::SuitBegin));
    m_allCards.add(Card(Card::CardBigJoker, Card::SuitBegin));
}

Card GameControl::takeOneCard()
{
    //始化的时候在容器中存储了多张扑克牌
    //分发牌的时候随机抽取一张进行分发
    return m_allCards.takeRandomCard();
}

Cards GameControl::getSurplusCards()
{
    //发牌的时候一张一张发,循环完,剩下的三张牌没有分发留在m_allcards里面
    return m_allCards;
}

void GameControl::resetCardData()
{
    //洗牌,因为扑克牌添加到容器中是无序的,
    //所以重新创建对象添加到容器中即可完成初始化操作
    initAllCards();
    //清空玩家手中的牌,每个玩家也都有自己的扑克牌数据
    //存储到了自己的QSet容器中
    m_robotLeft->clearCards();
    m_robotRight->clearCards();
    m_user->clearCards();
    //初始化出牌玩家和牌
    m_pendPlayer = nullptr;
    m_pendCards.clear();
}

void GameControl::startLordCard()
{
    //当前玩家准备叫地主
    m_currentPlayer->prepareCallLord();
    //通知主窗口当前玩家的状态变换,信号接收者是主窗口
    emit playerStatusChanged(m_currentPlayer, ThinkingForCallLord);
}

void GameControl::becomeLord(Player *player,int bet)
{
    m_currentBet = bet;   //记录成为地主的下注分数
    player->setRole(Player::Lord);
    //前后玩家都是农民
    player->getPrevPlayer()->setRole(Player::Farmer);
    player->getNextPlayer()->setRole(Player::Farmer);

    m_currentPlayer = player; //抢到地主的先出牌
    player->storeDispatchCard(m_allCards); //分完牌后剩下的三张牌给地主

    //开启定时器 1s ,防止机器人玩家抢到地主后立马出牌
    QTimer::singleShot(1000, this, [=]()
    {
        //发射玩家状态改变和游戏状态变化
        emit gameStatusChanged(PlayingHand);
        emit playerStatusChanged(player, ThinkingForPlayHand);
        //当前玩家准备出牌
        m_currentPlayer->preparePlayHand();
        qDebug()<<m_currentPlayer->getName()<<"玩家成为地主: ";
    });
}

void GameControl::clearPlayerScore()
{
    m_robotLeft->setScore(0);
    m_robotRight->setScore(0);
    m_user->setScore(0);
}

int GameControl::getPlayerMaxBet()
{
    return m_betRecord.point;
}

void GameControl::onGrabBet(Player *player, int point)
{
    //如果说下注的分数是0,说明玩家不抢地主
    //存储的下注的分数比当前玩家下注的分数大,那么玩家下注的分数是无效的,就相当于弃权
    if(point == 0 || m_betRecord.point >= point)
    {
        emit notifyGrabLordBet(player, 0, false);
    }
    // m_betRecord.point==0说明没有人叫地主或者放弃抢地主
    //我们根据这个信息播放合适的音乐,展示相应的信息
    else if(point > 0 && m_betRecord.point == 0)
    {
        //第一个抢地主的玩家
        emit notifyGrabLordBet(player, point, true);   //信号接收者是主窗口
    }
    else
    {
        //第2个或第三个抢地主的玩家
        emit notifyGrabLordBet(player, point, false);
    }
    //判断玩家下注是不是三分,是的话直接结束抢地主
    if(point == 3)
    {
        becomeLord(player, point);
        //玩家成为地主,抢地主流程结束,清空数据
        m_betRecord.reset();
        return;
    }
    //玩家下注不是三分,我们需要对下注的分数进行比较,如果当前存储的分数小于下一个玩家下注的分数
    //我们就更新这个分数,保证这个结构体中的下注分数是最大的
    if(m_betRecord.point < point)
    {
        m_betRecord.point = point;
        m_betRecord.player = player;
    }
    m_betRecord.times++;
    //如果每个玩家都抢了一次地主,抢地主结束
    if(m_betRecord.times == 3)
    {
        //三个玩家都不抢地主,point每次都是0
        if(m_betRecord.point == 0)
        {
            //修改游戏状态,重新发牌
            emit gameStatusChanged(DispatchCard);
        }
        else
        {
            becomeLord(m_betRecord.player, m_betRecord.point);
        }
        m_betRecord.reset();
        return;
    }
    //通知下一个玩家继续抢地主
    m_currentPlayer = player->getNextPlayer();
    //发送信号,告诉主界面当前是抢地主状态
    emit playerStatusChanged(m_currentPlayer, ThinkingForCallLord);
    m_currentPlayer->prepareCallLord();
}

void GameControl::onPlayHand(Player *player, const Cards &cards)
{
    qDebug()<<"玩家出牌";
    //1.将玩家出的牌转发给主界面
    emit notifyPlayHand(player, cards);
    Cards &myCards = const_cast<Cards &>(cards);
    //2.判断玩家出的牌是不是空牌,给其他玩家发射信号,保存出牌玩家和打出的牌
    if(!myCards.isEmpty())
    {
        m_pendCards = cards;
        m_pendPlayer = player;
        emit pendingInfo(player, cards);
    }
    //如果存在炸弹,底分翻倍
    PlayHand::HandType type=PlayHand(myCards).getHandType();
    if(type == PlayHand::Hand_Bomb || type == PlayHand::Hand_Bomb_Jokers)
    {
        m_currentBet *= 2;
    }
    // 3.如果玩家的牌出完了,计算本局得分
    if(player->getCards().isEmpty())
    {
        Player *prev = player->getPrevPlayer();
        Player *next = player->getNextPlayer();
        if(player->getRole() == Player::Lord)
        {
            player->setScore(player->getScore() + m_currentBet * 2);
            prev->setScore(prev->getScore() - m_currentBet);
            next->setScore(next->getScore() - m_currentBet);
            //记录玩家状态
            player->setWin(true);
            prev->setWin(false);
            next->setWin(false);
        }
        else
        {
            //当前玩家不是地主
            player->setWin(true);
            player->setScore(player->getScore() + m_currentBet);
            //前一个玩家是地主
            if(prev->getRole() == Player::Lord)
            {
                prev->setScore(prev->getScore() - 2 * m_currentBet);
                next->setScore(next->getScore() + m_currentBet);
                prev->setWin(false);
                next->setWin(true);
            }
            else
            {
                next->setScore(next->getScore() - 2 * m_currentBet);
                prev->setScore(prev->getScore() + m_currentBet);
                next->setWin(false);
                prev->setWin(true);
            }
        }
        //玩家状态改变
        emit playerStatusChanged(player, GameControl::Winning);
        return;
    }
    //4.牌没有出完,下一个玩家继续出牌
    m_currentPlayer = player->getNextPlayer();
    m_currentPlayer->preparePlayHand();
    emit playerStatusChanged(m_currentPlayer, GameControl::ThinkingForPlayHand);
}
