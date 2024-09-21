#ifndef STRATEGY_H
#define STRATEGY_H

#include "player.h"
#include "playhand.h"

class Strategy
{
public:
    //策略类对应某个机器人玩家和牌
    Strategy(Player *player, const Cards &cards);
public:
    //1.制定出牌策略
    Cards makeStrategy();
    //2.第一个出牌firstPlay,获取出牌对象
    Cards firstPlay();
    //3.得到比指定牌型大的牌
    Cards getGreaterCards(PlayHand type);
    //4.能大过指定的牌时,判断是出牌还是放行,true出牌,false放行
    bool whetherToBeat(Cards &cards);
    //5.找出count张点数为point的牌
    Cards findSamePointCards(Card::CardPoint point, int count);
    //找出所有点数数量为count的牌,得到一个多张扑克牌数组
    //对于一组扑克牌,我们相同数目的牌不止一种,所以说要创建一个容器保存多个点数的牌
    QVector<Cards> findCardsByCount(int count);
    //根据点数范围找对象,给出一个最大值,一个最小值
    Cards getRangeCards(Card::CardPoint begin, Card::CardPoint end);
    //按牌型找牌,并且指定要找的牌是否大于指定的牌型,bool变量用来指明后者
    QVector<Cards> findCardType(PlayHand hand, bool beat);
    //从指定的cards对象中找出满足条件的顺子
    void pickSeqSingles(QVector<QVector<Cards>> &allSeqRecord, const QVector<Cards> &seqSingle, const Cards &cards);
    QVector<Cards> pickOptimalSeqSingles();
private:
    using function = Cards (Strategy:: *)(Card::CardPoint point);
    struct CardInfo
    {
        Card::CardPoint begin;
        Card::CardPoint end;    //连对和顺子的终止点数不一样
        int extra;      //顺子或连对的数量
        bool beat;
        int number;     //指定点数的牌的数量
        int base;   //基础的连对或顺子的数量
        function getSeq;
    };
    //point是从哪一个点数开始搜索,number是找多少张
    QVector<Cards> getCards(Card::CardPoint point, int number);
    //type是要带的副牌的类型,找三带一或三代二的牌
    QVector<Cards> getTripleSingleOrPair(Card::CardPoint begin, PlayHand::HandType type);
    QVector<Cards> getPlane(Card::CardPoint begin);
    QVector<Cards> getPlaneTwoSingleOrTwoPair(Card::CardPoint begin, PlayHand::HandType type);
    QVector<Cards> getSeqPairOrSeqSingle(CardInfo &info);
    Cards getBaseSeqPair(Card::CardPoint point);    //获取基本的连对
    Cards getBaseSeqSingle(Card::CardPoint point);  //获取基本的顺子
    QVector<Cards> getBomb(Card::CardPoint begin);
private:
    Player *m_player;
    Cards m_cards;
};

#endif // STRATEGY_H
