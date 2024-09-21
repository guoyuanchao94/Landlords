#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "cards.h"

//玩家类定义
class Player : public QObject
{
    Q_OBJECT
public:
    enum Role{Lord , Farmer}; //角色
    enum Sex{Man, Woman};      //性别,用于判断音效的播放
    enum Direction{Left, Right};  //头像显示方向,左和右
    enum Type{Robot, User, UnKnown}; //玩家类型,机器人玩家和用户玩家的处理不同
public:
    explicit Player(QObject *parent = nullptr);
    explicit Player(QString name, QObject *parent = nullptr);
public:
    void setName(QString name);
    QString getName();
    //角色
    void setRole(Role role);
    Role getRole();
    //性别
    void setSex(Sex sex);
    Sex getSex();

    //显示方向
    void setDirection(Direction direction);
    Direction getDirection();

    //玩家类型
    void setType(Type type);
    Type getType();

    //玩家分数
    void setScore(int score);
    int getScore();

    //玩家是否胜利
    void setWin(bool flag);
    bool isWin();

    //玩家需要知道上一个出牌的玩家和下一个需要出牌的玩家,控制出牌顺序
    void setPrevPlayer(Player *player);
    void setNextPlayer(Player *player);
    Player *getPrevPlayer();
    Player *getNextPlayer();

    void grabLordBet(int point);    //抢地主/叫地主函数

    //存储玩家手中的扑克牌,发牌时获得
    void storeDispatchCard(const Card &card);   //发牌时每次发一张牌存储
    void storeDispatchCard(const Cards &cards); //存储抢到地主获得的牌

    Cards getCards();   //获得玩家手中的牌便于展示

    void clearCards();  //清空玩家手中的牌
    void playHand(const Cards &cards); //玩家出牌
    //出牌的玩家待处理的扑克牌,也就是出牌的玩家待处理的卡牌
    Player *getPendPlayer();
    Cards getPendCards();

    //存储出牌玩家对象和打出的牌,槽函数
    void storePendingInfo(Player *player, const Cards &cards);

    //虚函数,交由子类去处理
    virtual void prepareCallLord();
    virtual void preparePlayHand();
    virtual void thinkCallLord();   //考虑叫地主
    virtual void thinkPlayHand();   //考虑出牌
protected:
    QString m_name;
    Role m_role;
    Sex m_sex;
    Direction m_direction;  //头像显示方向
    Type m_type;            //玩家类型
    int m_score = 0;
    bool m_isWin = false;

    Player *m_prev = nullptr;
    Player *m_next = nullptr;
    Cards m_cards;          //存储的玩家被分发到的扑克牌

    Player *m_pendPlayer = nullptr;   //当前出牌的玩家
    Cards m_pendCards;      //待处理的扑克牌
signals:
    //哪一个玩家下注了几分
    void notifyGrabLordBet(Player *player, int point);
    void notifyPlayHand(Player *player, const Cards &cards);
    //通知已经发牌了,解决抢完地主后,剩余的三张牌没有给地主的问题
    void notifyPickCards(Player *player, const Cards &cards);
};

#endif // PLAYER_H
