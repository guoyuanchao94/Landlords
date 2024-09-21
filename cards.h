#ifndef CARDS_H
#define CARDS_H
#include "card.h"
#include <QSet>

class Cards
{
public:
    enum SortType{Asc, Desc, NoSort};
public:
    Cards();
    //可以通过一张扑克牌构造多张扑克牌对象
    explicit Cards(const Card &card);
public:
    //Cards对象中维护一个QSet容器存储多张扑克牌
    void add(const Card &card); //添加一张扑克牌
    void add(const Cards &cards); //添加多张扑克牌
    void add(const QVector<Cards> &cards);

    void remove(const Card &card); //删除一张扑克牌
    void remove(const Cards &cards);
    void remove(const QVector<Cards> &cards);

    //重载流插入运算符
    Cards &operator <<(const Card &card);
    Cards &operator <<(const Cards &cards);

    int cardCount();    //扑克牌的数量,也就是容器的大小
    bool isEmpty();     //是否为空
    bool isEmpty() const;
    void clear();       //清空扑克牌,把容器清空掉
    Card::CardPoint minPoint(); //最小点数
    Card::CardPoint maxPoint(); //最大点数
    int pointCount(Card::CardPoint point);  //某个点数的牌的数量
    bool contains(const Card &card);  //扑克牌组中是否包含某个点数的扑克牌
    bool contains(const Cards &cards);  //扑克牌组中是否包含一组扑克
    //随机取出一张牌
    Card takeRandomCard();

    //QSet to QVector,QSet容器不能有序存储扑克牌信息
    CardList toCardList(SortType type = Desc); //有序排列扑克牌,默认降序排序
private:
    QSet<Card> m_cards; //存储扑克牌
};

#endif // CARDS_H
