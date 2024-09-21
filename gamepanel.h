#ifndef GAMEPANEL_H
#define GAMEPANEL_H

#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include "gamecontrol.h"
#include "cardpanel.h"
#include <QTimer>
#include "animationwindow.h"
#include "countdown.h"
#include "bgmcontrol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GamePanel; }
QT_END_NAMESPACE

class GamePanel : public QMainWindow
{
    Q_OBJECT

public:
    GamePanel(QWidget *parent = nullptr);
    ~GamePanel();
public:
    enum AnimationType{ShunZi, LianDui, Plane, JokerBomb, Bomb, Bet};
    void gameControlInit();      //初始化游戏控制类
    void updatePlayerScore();   //更新分数面板的分数
    void initCardMap();         //切割并存储图片
    //裁剪图片,被裁剪的图片、裁剪的位置
    void cropImage(QPixmap &pixmap, int x, int y, Card &card);
    void initButtonsGroup();    //初始化按钮组,处理其中发射的信号
    void initPlayerContext();   //初始化玩家在界面的环境
    void initGameScene();       //初始化游戏场景,初始化完之后我们就开始发牌阶段
    //处理游戏状态
    void gameStatusProcess(GameControl::GameStatus status);
    void startDiapatchCard();   //处理发牌逻辑,开始发牌前的准备工作
    void cardMoveStep(Player *player, int currentPos); //扑克牌移动步长

    //处理分发得到的扑克牌,用于卡牌的显示,解决发牌阶段扑克牌不显示在窗口的问题
    //具备存储多张扑克牌的能力,形参是Cards类型
    void disposeCard(Player *player, const Cards &cards);
    void updatePlayerCards(Player *player);       //更新扑克牌在窗口中的显示
    void showAnimation(AnimationType type, int bet = 0);
    //处理玩家打出去的牌
    void onDisposePlayHand(Player *player, const Cards &cards);

    //隐藏玩家打出的牌
    void hidePlayerDropCards(Player *player);
    //加载玩家头像
    QPixmap loadRoleImage(Player::Sex sex, Player::Direction direction, Player::Role role);
    //显示玩家最终得分面板
    void showEndingScorePanel();
    //初始化闹钟倒计时类
    void initCountDown();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;   //鼠标移动,框选多张扑克牌对象
public slots:
    void onDispatchCard(); //定时器处理发牌动画
    //游戏控制类发信信号接收的槽函数
    void onPlayerStatusChanged(Player *player, GameControl::PlayerStatus status);
    void onGrabLordBet(Player *player, int point, bool flag);
    //处理玩家选牌
    void onCardSelected(Qt::MouseButton button);

    //处理用户玩家出牌函数
    void onUserPlayHand();

    //用户玩家不出牌
    void onUserPass();
private:
    Ui::GamePanel *ui;
private:
    enum CardAlignment{Horizontal, Vertical};
    //界面上显示的内容比较多,所以我们创建一个结构体数据块
    //存储这些数据
    struct PlayerContext
    {
        //玩家扑克牌显示区域
        QRect cardRect;
        //出牌区域
        QRect playHandRect;
        //扑克牌对齐方式,垂直显示,水平显示
        CardAlignment alignment;
        //扑克牌是正面还是背面
        bool isFrontSide;
        //游戏过程中的提示信息,比如说不出牌
        QLabel *info;  //可以显示图片也可以显示文本,所以选择QLabel
        //玩家头像信息
        QLabel *roleImg;
        //玩家刚打出的牌
        Cards lastCards;
    };

    QPixmap m_bkImage; //背景图片
    GameControl *m_gameCtrl;   //游戏控制类对象
    QVector<Player *> m_playerList;

    QMap<Card, CardPanel *> m_cardMap; //存储扑克牌点数花色信息和图片信息
    QSize m_cardSize;     //扑克牌的尺寸大小
    QPixmap m_cardBackImg;  //扑克牌的背景图片
    QMap<Player *, PlayerContext> m_contextMap;

    CardPanel *m_baseCard; //发牌时的底牌
    CardPanel *m_moveCard; //移动的扑克牌
    QVector<CardPanel *> m_lastThreeCard;//剩余的三张底牌

    QPoint m_baseCardPos; //游戏底牌的位置
    GameControl::GameStatus m_gameStatus; //游戏状态
    QTimer *m_timer; //用于发牌的动画效果
    AnimationWindow *m_animation;

    //准备出牌时选中的扑克牌
    CardPanel *m_currentSelectedPanel;  //保存当前选中的扑克牌

    QSet<CardPanel *> m_selectedCards;  //存储选中的多张扑克牌

    QRect m_cardsRect;   //非机器人玩家每张扑克牌显示区域
    QHash<CardPanel *, QRect> m_userCards;   //非机器人玩家

    CountDown *m_countDown;
    BGMControl *m_bgm;
};
#endif // GAMEPANEL_H
