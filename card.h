#ifndef CARD_H
#define CARD_H

#include <QVector>
class Card
{    
public:
    //花色枚举值
    enum CardSuit
    {
        SuitBegin, //可以处理大小王的花色
        Diamond, //方块
        Club,    //梅花
        Heart,   //红桃
        Spade,    //黑桃
        SuitEnd
    };
    //点数枚举值
    enum CardPoint
    {
        CardBegin, //处理大小王的点数
        Card3,
        Card4,
        Card5,
        Card6,
        Card7,
        Card8,
        Card9,
        Card10,
        CardJ,
        CardQ,
        CardK,
        CardA,
        Card2,
        CardSmallJoker,
        CardBigJoker,
        CardEnd
    };
public:
    Card();
    Card(CardPoint point, CardSuit suit); //点数和花色构造函数
public:
    //写入何读取函数
    void setPoint(CardPoint point);
    void setSuit(CardSuit suit);
    CardPoint point() const;
    CardSuit suit() const;
private:
    CardPoint m_point;
    CardSuit m_suit;
};

//比较两个Card对象,用于扑克牌窗口的大小顺序排列
bool lessSort(const Card &card1, const Card &card2);
bool greaterSort(const Card &card1, const Card &card2);
bool operator <(const Card &card1, const Card &card2); //QMap键排序

// 重载==号,QSet容器存储不重复的元素,判断两个元素是否相等,而且无序,不需要重载<
bool operator ==(const Card &card1, const Card &card2);

//重写qHash函数,计算存储的数据的哈希值
uint qHash(const Card &card);

using CardList = QVector<Card>; //QVector元素有序,可以顺序排列扑克牌

#endif // CARD_H
