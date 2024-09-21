#include "playhand.h"

PlayHand::PlayHand()
{

}

PlayHand::PlayHand(Cards &cards)
{
    //首先对cards的牌进行分类,统计相同点数的牌的个数
    classify(cards);
    judgeCardType();
}

PlayHand::PlayHand(HandType type, Card::CardPoint point, int extra)
{
    m_handType = type;
    m_point = point;
    m_extra = extra;
}

PlayHand::HandType PlayHand::getHandType()
{
    return m_handType;
}

Card::CardPoint PlayHand::getCardPoint()
{
    return m_point;
}

int PlayHand::getExtra()
{
    return m_extra;
}

bool PlayHand::canBeat(const PlayHand &other)
{
    //几种极端情况
    //我的牌是未知的
    if(m_handType==Hand_Unknown)
    {
        return false;
    }
    //对方放弃出牌,直接胜利
    if(other.m_handType==Hand_Pass)
    {
        return true;
    }
    //我的牌是王炸
    if(m_handType==Hand_Bomb_Jokers)
    {
        return true;
    }
    //我是炸弹,对方是小于炸弹的牌型
    if(m_handType==Hand_Bomb && other.m_handType>=Hand_Single && other.m_handType<=Hand_Seq_Single)
    {
        return true;
    }
    //我俩的牌型一致,比较点数
    if(m_handType==other.m_handType)
    {
        if(m_handType==Hand_Seq_Single || m_handType==Hand_Seq_Pair)
        {
            //最小点数大于对方的牌且数量一样
            return m_point>other.m_point && m_extra==other.m_extra;
        }
        else
        {
            return m_point > other.m_point;
        }
    }
    return false;
}

//给一组扑克牌,对扑克牌的各个点数有几张进行分类,
// 比如这么一组扑克牌 Q 5 7 6 8 10 K 10 5
// 我们先进行排序,降序排序
// K Q 10 10 8 7 6 5 5
//我们创建一个数组,以扑克牌的最大点数作为数组长度,16
// 遍历list容器,取出其中的扑克牌,
// 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
//       2 1 1 1   2    1  1

//计算每种点数牌的数量,每个点数的牌最多有四张
void PlayHand::classify(Cards &cards)
{
    CardList list = cards.toCardList();
    int cardRecord[Card::CardEnd];  //16的数组,0-15
    memset(cardRecord, 0 , sizeof(int) * Card::CardEnd); //数组初始化
    for(int i = 0; i < list.size(); ++i)
    {
        Card card = list.at(i);
        cardRecord[card.point()]++; //把点数在数组中对应的位置的值加1
    }
    m_oneCard.clear();
    m_twoCard.clear();
    m_threeCard.clear();
    m_fourCard.clear();

    for(int i = 0; i < Card::CardEnd; ++i)
    {
        if(cardRecord[i] == 1)
        {
            m_oneCard.push_back(static_cast<Card::CardPoint>(i));
        }
        else if(cardRecord[i] == 2)
        {
            m_twoCard.push_back(static_cast<Card::CardPoint>(i));
        }
        else if(cardRecord[i] == 3)
        {
            m_threeCard.push_back(static_cast<Card::CardPoint>(i));
        }
        else if(cardRecord[i] == 4)
        {
            m_fourCard.push_back(static_cast<Card::CardPoint>(i));
        }
    }
}

//对拿到的牌进行类型区分
void PlayHand::judgeCardType()
{
    m_handType = Hand_Unknown; //初始化牌的类型
    m_point = Card::CardBegin; //初始化点数
    m_extra = 0;  //初始化额外变量,用来记录连对有多少对,顺子有多少张牌

    if(isPass())
    {
        m_handType = Hand_Pass;
    }
    if(isSingle())
    {
        m_handType = Hand_Single;
        m_point = m_oneCard[0];
    }
    else if(isPair())
    {
        m_handType = Hand_Pair;
        m_point = m_twoCard[0];
    }
    //是不是三张点数相同的牌
    else if(isTriple())
    {
        m_handType = Hand_Triple;
        m_point = m_threeCard[0];
    }
    else if(isTripleSingle())
    {
        m_handType = Hand_Triple_Single;
        m_point = m_threeCard[0]; //不关心三带一带的那张牌的点数
    }
    else if(isTriplePair())
    {
        m_handType = Hand_Triple_Pair;
        m_point = m_threeCard[0];     //不关心带的牌的对的点数
    }
    else if(isPlane())
    {
        m_handType = Hand_Plane;
        //记录点数最小的牌
        m_point = m_threeCard[0];
    }
    else if(isPlaneTwoSingle())
    {
        //不去关心带的两张单牌的点数
        m_handType = Hand_Plane_Two_Single;
        m_point = m_threeCard[0];
    }
    else if(isPlaneTwoPair())
    {
        m_handType = Hand_Plane_Two_Pair;
        m_point = m_threeCard[0];
    }
    else if(isSeqPair())
    {
        m_handType = Hand_Seq_Pair;
        m_point = m_twoCard[0];
        //记录连对的个数
        m_extra = m_twoCard.size();
    }
    else if(isSeqSingle())
    {
        m_handType = Hand_Seq_Single;
        m_point = m_oneCard[0];
        m_extra = m_oneCard.size();
    }
    else if(isBomb())
    {
        m_handType = Hand_Bomb;
        m_point = m_fourCard[0];
    }
    else if(isBombSingle())
    {
        m_handType = Hand_Bomb_Single;
        m_point = m_fourCard[0];
    }
    else if(isBombPair())
    {
        m_handType = Hand_Bomb_Pair;
        m_point = m_fourCard[0];
    }
    else if(isBombTwoSingle())
    {
        m_handType = Hand_Bomb_Two_Single;
        m_point = m_fourCard[0];
    }
    else if(isBombJokers())
    {
        //王炸只有一个
        m_handType = Hand_Bomb_Jokers;
    }
    else if(isBombJokersSingle())
    {
        m_handType = Hand_Bomb_Jokers_Single;
    }
    else if(isBombJokersPair())
    {
        m_handType = Hand_Bomb_Jokers_Pair;
    }
    else if(isBombJokersTwoSingle())
    {
        m_handType = Hand_Bomb_Jokers_Two_Single;
    }
}

bool PlayHand::isPass()
{
    if(m_oneCard.isEmpty() && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        return true;
    }
    return false;
}

bool PlayHand::isSingle()
{
    //打出的牌是单牌,在进行牌的分类的时候,被加进了oneCard容器
    if(m_oneCard.size() == 1 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        return true;
    }
    return false;
}

bool PlayHand::isPair()
{
    if(m_oneCard.isEmpty() && m_twoCard.size() == 1 && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        return true;
    }
    return false;
}

bool PlayHand::isTriple()
{
    if(m_oneCard.isEmpty() && m_twoCard.isEmpty() && m_threeCard.size() == 1 && m_fourCard.isEmpty())
    {
        return true;
    }
    return false;
}

//三带一
bool PlayHand::isTripleSingle()
{
    if(m_oneCard.size() == 1 && m_twoCard.isEmpty() && m_threeCard.size() == 1 && m_fourCard.isEmpty())
    {
        return true;
    }
    return false;
}

bool PlayHand::isTriplePair()
{
    if(m_oneCard.isEmpty() && m_twoCard.size() == 1 && m_threeCard.size()==1 && m_fourCard.isEmpty())
    {
        return true;
    }
    return false;
}

//两个点数连续的三张牌333444
bool PlayHand::isPlane()
{
    if(m_oneCard.isEmpty() && m_twoCard.isEmpty() && m_threeCard.size() == 2 && m_fourCard.isEmpty())
    {
        //判断点数是否相邻并且较大的点数的牌是否小于2
        //默认是升序排序
        std::sort(m_threeCard.begin(), m_threeCard.end());
        if(m_threeCard[1] - m_threeCard[0] == 1 && m_threeCard[1] < Card::Card2 )
        {
            return true;
        }
    }
    return false;
}

bool PlayHand::isPlaneTwoSingle()
{
    //飞机带两张不同的牌,不能带大小王
    if(m_oneCard.size() == 2 && m_twoCard.isEmpty() && m_threeCard.size() == 2 && m_fourCard.isEmpty())
    {
        //
        std::sort(m_threeCard.begin(), m_threeCard.end());
        std::sort(m_oneCard.begin(), m_oneCard.end());
        //飞机带两张单牌,并且带的两张单牌不是大小王
        if(m_threeCard[1] - m_threeCard[0] == 1 && m_threeCard[1] < Card::Card2
        && m_oneCard[0] != Card::CardSmallJoker && m_oneCard[1] != Card::CardBigJoker)
        {
            return true;
        }
    }
    return false;
}

bool PlayHand::isPlaneTwoPair()
{
    //飞机带两对,且对带的点数没有要求
    if(m_oneCard.isEmpty() && m_twoCard.size() == 2 && m_threeCard.size() == 2 && m_fourCard.isEmpty())
    {
        std::sort(m_threeCard.begin(), m_threeCard.end());
        if(m_threeCard[1] - m_threeCard[0] == 1 && m_threeCard[1] < Card::Card2 )
        {
            return true;
        }
    }
    return false;
}

//用户打出去的牌是不是连对,最少是三种两张相同的牌
bool PlayHand::isSeqPair()
{
    if(m_oneCard.isEmpty() && m_twoCard.size() >= 3 && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        std::sort(m_twoCard.begin(), m_twoCard.end());
        if(m_twoCard.last() - m_twoCard.first() == (m_twoCard.size() - 1)
        && m_twoCard.first() >= Card::Card3 && m_twoCard.last() < Card::Card2)
        {
            return true;
        }
    }
    return false;
}

//顺子牌最少是五张
bool PlayHand::isSeqSingle()
{
    if(m_oneCard.size() >= 5 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        std::sort(m_oneCard.begin(), m_oneCard.end());
        if(m_oneCard.last() - m_oneCard.first() == (m_oneCard.size() - 1)
        && m_oneCard.first() >= Card::Card3 && m_oneCard.last() < Card::Card2)
        {
            return true;
        }
    }
    return false;
}

//四张点数相同的牌
bool PlayHand::isBomb()
{
    if(m_oneCard.isEmpty() && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.size() == 1)
    {
        return true;
    }
    return false;
}

bool PlayHand::isBombSingle()
{
    if(m_oneCard.size() == 1 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.size() == 1)
    {
        return true;
    }
    return false;
}

bool PlayHand::isBombPair()
{
    if(m_oneCard.isEmpty() && m_twoCard.size() == 1 && m_threeCard.isEmpty() && m_fourCard.size() == 1)
    {
        return true;
    }
    return false;
}

bool PlayHand::isBombTwoSingle()
{
    if(m_oneCard.size() == 2 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.size() == 1)
    {
        std::sort(m_oneCard.begin(), m_oneCard.end());
        //炸弹带的两张单牌不能是大小王
        if(m_oneCard.first() != Card::CardSmallJoker && m_oneCard.last() != Card::CardBigJoker)
        {
            return true;
        }
    }
    return false;
}

bool PlayHand::isBombJokers()
{

    if(m_oneCard.size() == 2 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        std::sort(m_oneCard.begin(), m_oneCard.end());
        //判断两张单牌是否是大小王
        if(m_oneCard.first() == Card::CardSmallJoker && (m_oneCard.last() == Card::CardBigJoker))
        {
            return true;
        }
    }
    return false;
}

bool PlayHand::isBombJokersSingle()
{
    //大小王带一张单牌,不考虑单盘的点数
    if(m_oneCard.size() == 3 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        std::sort(m_oneCard.begin(), m_oneCard.end());
        if(m_oneCard[1] == Card::CardSmallJoker && (m_oneCard[2] == Card::CardBigJoker))
        {
            return true;
        }
    }
    return false;
}

//大小王带一对
bool PlayHand::isBombJokersPair()
{
    if(m_oneCard.size() == 2 && m_twoCard.size()==1 && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        std::sort(m_oneCard.begin(), m_oneCard.end());
        if(m_oneCard[0] == Card::CardSmallJoker && (m_oneCard[1] == Card::CardBigJoker))
        {
            return true;
        }
    }
    return false;
}

//大小王带两张单牌
bool PlayHand::isBombJokersTwoSingle()
{
    if(m_oneCard.size() == 4 && m_twoCard.isEmpty() && m_threeCard.isEmpty() && m_fourCard.isEmpty())
    {
        std::sort(m_oneCard.begin(), m_oneCard.end());
        if(m_oneCard[2] == Card::CardSmallJoker && (m_oneCard[3] == Card::CardBigJoker))
        {
            return true;
        }
    }
    return false;
}
