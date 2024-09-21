#ifndef PLAYHAND_H
#define PLAYHAND_H

#include "cards.h"
#include "card.h"

//出牌类属于策略类
class PlayHand
{
public:
    enum HandType
    {
        Hand_Unknown,               // 未知
        Hand_Pass,                  // 过

        Hand_Single,                // 单
        Hand_Pair,                  // 对

        Hand_Triple,                // 三个
        Hand_Triple_Single,         // 三带一
        Hand_Triple_Pair,           // 三带二

        Hand_Plane,                 // 飞机，555_666
        Hand_Plane_Two_Single,      // 飞机带单，555_666_3_4
        Hand_Plane_Two_Pair,        // 飞机带双，555_666_33_44

        Hand_Seq_Pair,              // 连对，33_44_55(_66...)
        Hand_Seq_Single,            // 顺子，34567(8...)

        Hand_Bomb,                  // 炸弹
        Hand_Bomb_Single,           // 炸弹带一个
        Hand_Bomb_Pair,             // 炸弹带一对
        Hand_Bomb_Two_Single,       // 炸弹带两单

        Hand_Bomb_Jokers,           // 王炸
        Hand_Bomb_Jokers_Single,    // 王炸带一个
        Hand_Bomb_Jokers_Pair,      // 王炸带一对
        Hand_Bomb_Jokers_Two_Single	// 王炸带两单
    };
public:
    PlayHand();
    //帮助玩家分析牌的点数和组合,我们传递一个Cards对象进去
    //将一组牌传递给playhand对象,判断牌的点数,花色还有额外的信息,比如牌的数量
    explicit PlayHand(Cards &cards);
    //传递牌的类型和点数,额外信息,构造一个PlayHand对象,扩展信息记录对的个数,牌的个数
    PlayHand(HandType type, Card::CardPoint point, int extra);
public:
    HandType getHandType(); //得到传递的参数的扑克牌的类型
    Card::CardPoint getCardPoint();
    int getExtra(); //获取到牌型的数量
    //比较自己的牌型和其他人的牌型的大小
    bool canBeat(const PlayHand &other);
private:
    //对扑克牌进行分类,1张的、两张的、三张的、四张的有多少种
    void classify(Cards &cards);
    //对牌型进行分类
    void judgeCardType();

    bool isPass();  //判断玩家手中的牌是不是空的
    bool isSingle();
    bool isPair();
    bool isTriple();
    bool isTripleSingle();
    bool isTriplePair();
    bool isPlane();
    bool isPlaneTwoSingle();
    bool isPlaneTwoPair();
    bool isSeqPair();
    bool isSeqSingle();
    bool isBomb();
    bool isBombSingle();
    bool isBombPair();
    bool isBombTwoSingle();
    bool isBombJokers();
    bool isBombJokersSingle();
    bool isBombJokersPair();
    bool isBombJokersTwoSingle();
private:
    HandType m_handType;
    Card::CardPoint m_point;
    int m_extra;
    //定义四个容器存储不同点数的扑克牌
    QVector<Card::CardPoint> m_oneCard; //存储点数只有一张的扑克牌
    QVector<Card::CardPoint> m_twoCard; //存储点数两张的扑克牌,比如22、33
    QVector<Card::CardPoint> m_threeCard;   //存储点数三张的扑克牌,比如333、444
    QVector<Card::CardPoint> m_fourCard;    //存储点数四张的扑克牌,4444、5555,存储炸弹用的
};

#endif // PLAYHAND_H
