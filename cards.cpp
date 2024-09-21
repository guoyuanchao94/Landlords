#include "cards.h"
#include <QRandomGenerator>
Cards::Cards()
{

}

Cards::Cards(const Card &card)
{
    add(card);
}

void Cards::add(const Card &card)
{
    m_cards.insert(card);
}

void Cards::add(const Cards &cards)
{
    //cards中也有一个QSet容器存储扑克牌
    //求并集,存储二者扑克牌
    m_cards.unite(cards.m_cards);
}

void Cards::add(const QVector<Cards> &cards)
{
    for(int i = 0; i < cards.count(); ++i)
    {
        add(cards.at(i));
    }
}

void Cards::remove(const Card &card)
{
    m_cards.remove(card);
}

void Cards::remove(const Cards &cards)
{
    //移除多张扑克牌,求差集
    m_cards.subtract(cards.m_cards);
}

void Cards::remove(const QVector<Cards> &cards)
{
    for(int i = 0; i < cards.size(); ++i)
    {
        remove(cards.at(i));
    }
}

Cards &Cards::operator <<(const Card &card)
{
    add(card);
    return *this;
}

Cards &Cards::operator <<(const Cards &cards)
{
    add(cards);
    return *this;
}

//扑克牌的数量
int Cards::cardCount()
{
    return m_cards.size();
}

bool Cards::isEmpty()
{
    return m_cards.isEmpty();
}

bool Cards::isEmpty() const
{
    return m_cards.isEmpty();
}

void Cards::clear()
{
    m_cards.clear();
}

//求存储的多张扑克牌中的最小点数
Card::CardPoint Cards::minPoint()
{
    Card::CardPoint min=Card::CardEnd; //首先置成最大值
    if(!m_cards.isEmpty())
    {
        for(auto it = m_cards.begin(); it != m_cards.end(); it++)
        {
            if(it->point()<min)
            {
                min=it->point();
            }
        }
    }
    return min;
}

Card::CardPoint Cards::maxPoint()
{
    Card::CardPoint max=Card::CardBegin;
    if(!m_cards.isEmpty())
    {
        for(auto it = m_cards.begin(); it != m_cards.end(); it++)
        {
            if(it->point() > max)
            {
                max = it->point();
            }
        }
    }
    return max;
}

int Cards::pointCount(Card::CardPoint point)
{
    int count = 0;
    if(!m_cards.isEmpty())
    {
        for(auto it = m_cards.begin(); it != m_cards.end(); it++)
        {
            if(it->point() == point)
            {
                count++;
            }
        }
    }
    return count;
}

bool Cards::contains(const Card &card)
{
    return m_cards.contains(card);
}

bool Cards::contains(const Cards &cards)
{
    return m_cards.contains(cards.m_cards);
}

Card Cards::takeRandomCard()
{
    int num = QRandomGenerator::global()->bounded(m_cards.size());
    QSet<Card>::const_iterator it = m_cards.constBegin();
    for(int i = 0; i < num; ++i, ++it); //遍历m_cards容器,找到第num个元素
    Card card =*it;
    m_cards.erase(it);
    return card;
}

CardList Cards::toCardList(SortType type)
{
    CardList list; //QVector<Card>
    //遍历QSet容器取出元素放入list
    for(auto it = m_cards.begin(); it !=m_cards.end(); ++it)
    {
        list << *it;
    }
    if(type == Asc)
    {
        // qsort已经被废弃了,从小到大升序排序
        std::sort(list.begin(), list.end(), lessSort);
    }
    else if(type == Desc)
    {
        std::sort(list.begin(), list.end(), greaterSort);
    }
    return list;
}

