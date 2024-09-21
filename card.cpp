#include "card.h"

Card::Card()
{

}

Card::Card(CardPoint point, CardSuit suit)
{
    setPoint(point);
    setSuit(suit);
}

void Card::setPoint(CardPoint point)
{
    m_point = point;
}

void Card::setSuit(CardSuit suit)
{
    m_suit = suit;
}

Card::CardPoint Card::point() const
{
    return m_point;
}

Card::CardSuit Card::suit() const
{
    return m_suit;
}

bool lessSort(const Card &card1, const Card &card2)
{
    if(card1.point() == card2.point())
    {
        return card1.suit() < card2.suit();
    }
    else
    {
        return card1.point() < card2.point();
    }
}

bool greaterSort(const Card &card1, const Card &card2)
{
    if(card1.point() == card2.point())
    {
        return card1.suit() > card2.suit();
    }
    else
    {
        return card1.point() > card2.point();
    }
}

bool operator <(const Card& card1, const Card& card2)
{
    return lessSort(card1, card2);
}

bool operator ==(const Card &card1,const Card &card2)
{
    if(card1.point() == card2.point() && card1.suit() == card2.suit())
    {
        return true;
    }
    return false;
}

//计算卡牌的哈希值
uint qHash(const Card &card)
{
    //计算的哈希值唯一
    return card.point() * 100 + card.suit();
}
