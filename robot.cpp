#include "robot.h"
#include "strategy.h"
#include "robotgrablord.h"
#include "robotplayhand.h"
#include <QDebug>

Robot::Robot(QObject *parent)
    : Player{parent}
{
    m_type = Player::Robot;
}

void Robot::prepareCallLord()
{
    RobotGrabLord *subThread=new RobotGrabLord(this);
    connect(subThread,&RobotGrabLord::finished,this,[=]()
    {
        qDebug() << "RobotGrabLoard子线程对象被析构" <<"名字是: "<<this->getName();
        subThread->deleteLater();
    });
    subThread->start();
}

void Robot::preparePlayHand()
{
    RobotPlayHand *subThread=new RobotPlayHand(this);
    connect(subThread,&RobotPlayHand::finished,this,[=]()
    {
        qDebug() << "RobotPlayHand子线程对象被析构" <<"名字是: "<<this->getName();
        subThread->deleteLater();
    });
    subThread->start();
    qDebug() <<"机器人玩家出牌线程启动";
}

void Robot::thinkCallLord()
{
    //基于手中的牌的计算权重
    //1.大小王:6
    //2.顺子/炸弹:5
    //3.三张点数相同的牌:4
    //4.2的权重:3
    //5.对牌:1

    int weight=0;
    Strategy strategy(this,m_cards);
    //获取大小王
    weight+=strategy.getRangeCards(Card::CardSmallJoker,Card::CardBigJoker).cardCount()*6;

    //获取顺子,炸弹没有影响
    QVector<Cards> optimalSeq=strategy.pickOptimalSeqSingles();
    weight+=optimalSeq.size()*5;
    //获取炸弹
    QVector<Cards> bombs=strategy.findCardsByCount(4);
    weight+=bombs.size()*5;

    //得到2的牌占的权重
    weight+=m_cards.pointCount(Card::Card2)*3;

    //获取三张点数相同的牌
    Cards temp=m_cards;
    temp.remove(optimalSeq);
    temp.remove(bombs);

    Cards cardsTwo=strategy.getRangeCards(Card::Card2,Card::Card2);     //获取点数为2的牌
    temp.remove(cardsTwo);  //删除点数2的牌,避免冲突

    QVector<Cards> triples=Strategy(this,temp).findCardsByCount(3);
    weight+=triples.size()*4;

    //得到对牌,先删除获取的三张牌
    temp.remove(triples);
    QVector<Cards> pair=Strategy(this,temp).findCardsByCount(2);
    weight+=pair.size()*1;

    if(weight>=22)
    {
        grabLordBet(3);
    }
    else if(weight<22 && weight >=18)
    {
        grabLordBet(2);
    }
    else if(weight<18 && weight >=10)
    {
        grabLordBet(1);
    }
    else
    {
        grabLordBet(0);
    }
}

void Robot::thinkPlayHand()
{
    Strategy strategy(this,m_cards);
    Cards cards=strategy.makeStrategy();
    playHand(cards);
}
