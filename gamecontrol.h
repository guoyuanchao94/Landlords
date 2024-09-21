#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include "userplayer.h"
#include "robot.h"
#include "cards.h"

//记录玩家下注的信息
struct BetRecord
{
    BetRecord()
    {
        reset();
    }
    //可以直接清空数据,也可以初始化
    void reset()
    {
        player = nullptr;
        point = 0;
        times = 0;
    }
    Player *player;    //记录当前下注的玩家
    int point;         //记录玩家下注的点数
    int times;         //记录是第几个叫地主的玩家
};

class GameControl : public QObject
{
    Q_OBJECT
public:
    //游戏状态
    enum GameStatus
    {
        DispatchCard, //发牌阶段
        CallingLord,  //叫地主阶段
        PlayingHand   //出牌阶段
    };
    //玩家状态
    enum PlayerStatus
    {
        ThinkingForCallLord, //思考叫地主
        ThinkingForPlayHand, //思考出牌
        Winning              //玩家胜利
    };
public:
    explicit GameControl(QObject *parent = nullptr);
public:
    //初始化玩家
    void playerInit();
    //得到玩家的实例对象
    Robot *getLeftRobot();
    Robot *getRightRobot();
    UserPlayer *getUserPlayer();

    //设置当前玩家,获取当前玩家,当前玩家是实时变化的
    void setCurrentPlayer(Player *player);
    Player *getCurrentPlayer();

    //得到出牌玩家和打出的牌
    Player *getPendPlayer();
    Cards getPendCards();

    //初始化扑克牌
    void initAllCards();
    //每次发一张扑克牌
    Card takeOneCard();
    //得到最后的三张底牌
    Cards getSurplusCards();
    //当前游戏结束,准备开始下一局游戏,重置扑克牌数据
    void resetCardData();
    //准备叫地主
    void startLordCard();
    //成为地主,记录成为地主的玩家和下注的分数
    void becomeLord(Player *player, int bet);
    //清空玩家分数
    void clearPlayerScore();

    //得到玩家最高的下注分数
    int getPlayerMaxBet();

private:
    Robot *m_robotLeft = nullptr;
    Robot *m_robotRight = nullptr;
    UserPlayer *m_user = nullptr;
    Player *m_currentPlayer = nullptr;

    //记录待处理的玩家和出的牌
    Player *m_pendPlayer = nullptr;
    Cards m_pendCards;

    //记录所有扑克牌,游戏初始化得到的所有的扑克牌
    Cards m_allCards;
    BetRecord m_betRecord;
    int m_currentBet = 0; //成为地主的下注分数
public slots:
    //处理叫地主
    void onGrabBet(Player *player, int point);
    //处理出牌
    void onPlayHand(Player *player, const Cards &cards);
signals:
    void playerStatusChanged(Player *player, PlayerStatus status);
    //通知玩家抢地主
    void notifyGrabLordBet(Player *player, int point, bool flag);
    void gameStatusChanged(GameStatus status);
    //通知玩家出牌
    void notifyPlayHand(Player *player, const Cards &cards);
    //给玩家传递出牌数据
    void pendingInfo(Player *player, const Cards &cards);
};

#endif // GAMECONTROL_H
