#include "strategy.h"
#include <QMap>
#include <functional>
//游戏策略类,主要是对于机器人玩家的出牌控制
Strategy::Strategy(Player *player,const Cards &cards)
    :m_player(player),m_cards(cards)
{
}

Cards Strategy::makeStrategy()
{
    //得到出牌玩家对象和打出的牌
    Player *pendPlayer=m_player->getPendPlayer();
    Cards pendCards=m_player->getPendCards();
    //判断上次出牌的玩家是不是我自己,如果是我自己,出牌没有限制
    //1.我出完牌,其他玩家没有出牌,又轮到我了
    //2.游戏刚开始,我第一个出牌
    if(pendPlayer==m_player || pendPlayer==nullptr)
    {
        //直接出牌,出牌没有限制
        return firstPlay();
    }
    else
    {
        PlayHand type(pendCards);
        Cards beatCards=getGreaterCards(type);
        bool shouldBeat=whetherToBeat(beatCards);
        if(shouldBeat)
        {
            return beatCards;
        }
        else
        {
            return Cards();
        }
    }
    return Cards();
}

Cards Strategy::firstPlay()
{
    //分析玩家手中的牌的牌型
    PlayHand hand(m_cards);
    //是单一牌型
    if(hand.getHandType()!=PlayHand::Hand_Unknown)
    {
        return m_cards;
    }
    //不是单一牌型,判断玩家手中是否有顺子
    QVector<Cards> optimalSeq=pickOptimalSeqSingles();
    if(!optimalSeq.isEmpty())
    {
        //得到单牌的数量
        int baseNumber=findCardsByCount(1).size();
        Cards save=m_cards;
        //把得到的顺子从玩家手中剔除
        save.remove(optimalSeq);
        int lastNumber=Strategy(m_player,save).findCardsByCount(1).size();
        if(baseNumber>lastNumber)
        {
            return optimalSeq[0];
        }
    }
    bool hasPlane=false;
    bool hasTriple=false;
    bool hasPair=false;

    Cards backup=m_cards;
    QVector<Cards> bombArray=findCardType(PlayHand(PlayHand::Hand_Bomb,Card::CardBegin,0),false);
    backup.remove(bombArray);

    //有没有飞机
    QVector<Cards> planeArray=Strategy(m_player,backup).findCardType(PlayHand(PlayHand::Hand_Plane,Card::CardBegin,0),false);
    if(!planeArray.isEmpty())
    {
        hasPlane=true;
        backup.remove(planeArray);
    }
    //有没有三张的牌
    QVector<Cards> seqTripleArray=Strategy(m_player,backup).findCardType(PlayHand(PlayHand::Hand_Triple,Card::CardBegin,0),false);
    if(!seqTripleArray.isEmpty())
    {
        hasTriple=true;
        backup.remove(seqTripleArray);
    }
    //有没有连对
    QVector<Cards> seqPairArray=Strategy(m_player,backup).findCardType(PlayHand(PlayHand::Hand_Seq_Pair,Card::CardBegin,0),false);
    if(!seqPairArray.isEmpty())
    {
        hasPair=true;
        backup.remove(seqPairArray);
        //删除连对后,只剩下一些散碎的牌,适合查找三带一的带的一这张牌
    }

    if(hasPair)
    {
        Cards maxPair;
        for(int i=0;i<seqPairArray.size();++i)
        {
            if(seqPairArray[i].cardCount() > maxPair.cardCount())
            {
                maxPair=seqPairArray[i];
            }
        }
        return maxPair;
    }
    if(hasPlane)
    {
        //飞机带两对
        bool planeTwoPair=false;
        QVector<Cards> pairArray;
        for(Card::CardPoint point=Card::Card3;point <= Card::Card10;point=static_cast<Card::CardPoint>(point+1))
        {
            Cards pair=Strategy(m_player,backup).findSamePointCards(point,2);
            if(!pair.isEmpty())
            {
                pairArray.push_back(pair);
                if(pairArray.size()==2)
                {
                    planeTwoPair=true;
                    break;
                }
            }
        }
        if(planeTwoPair)
        {
            Cards temp=planeArray[0];
            temp.add(pairArray);
            return temp;
        }
        else
        {
            //飞机带两对
            bool planeTwoSingle=false;
            QVector<Cards> singleArray;
            for(Card::CardPoint point=Card::Card3;point<Card::Card10;point=static_cast<Card::CardPoint>(point+1))
            {
                //尽量不去拆分对子
                if(backup.pointCount(point)==1)
                {
                    Cards single=Strategy(m_player,backup).findSamePointCards(point,1);
                    if(!single.isEmpty())
                    {
                        singleArray.push_back(single);
                        if(singleArray.size()==2)
                        {
                            planeTwoSingle=true;
                            break;
                        }
                    }
                }
            }
            if(planeTwoSingle)
            {
                Cards temp=planeArray[0];
                temp.add(singleArray);
                return temp;
            }
            else
            {
                return planeArray[0];
            }
        }
    }
    if(hasTriple)
    {
        if(PlayHand(seqTripleArray[0]).getCardPoint()<Card::CardA)
        {
            for(Card::CardPoint point=Card::Card3;point<Card::CardA; point=static_cast<Card::CardPoint>(point+1))
            {
                int pointCount=backup.pointCount(point);
                if(pointCount==1)
                {
                    Cards single=Strategy(m_player,backup).findSamePointCards(point,1);
                    Cards temp=seqTripleArray[0];
                    temp.add(single);
                    return temp;
                }
                else if(pointCount==2)
                {
                    Cards pair=Strategy(m_player,backup).findSamePointCards(point,2);
                    Cards temp=seqTripleArray[0];
                    temp.add(pair);
                    return temp;
                }
            }
        }
        return seqTripleArray[0];
    }

    //出单牌或者对牌
    Player *nextPlayer=m_player->getNextPlayer();
    if(nextPlayer->getCards().cardCount()==1 && nextPlayer->getRole()!=m_player->getRole())
    {
        for(Card::CardPoint point=static_cast<Card::CardPoint>(Card::CardEnd-1);point >= Card::Card3;point=static_cast<Card::CardPoint>(point-1))
        {
            int pointCount=backup.pointCount(point);
            if(pointCount==1)
            {
                Cards single=Strategy(m_player,backup).findSamePointCards(point,1);
                return single;
            }
            else if(pointCount==2)
            {
                Cards pair=Strategy(m_player,backup).findSamePointCards(point,2);
                return pair;
            }
        }
    }
    else
    {
        for(Card::CardPoint point=Card::Card3;point < Card::CardEnd;point=static_cast<Card::CardPoint>(point+1))
        {
            int pointCount=backup.pointCount(point);
            if(pointCount==1)
            {
                Cards single=Strategy(m_player,backup).findSamePointCards(point,1);
                return single;
            }
            else if(pointCount==2)
            {
                Cards pair=Strategy(m_player,backup).findSamePointCards(point,2);
                return pair;
            }
        }
    }
    return Cards();
}

Cards Strategy::getGreaterCards(PlayHand type)
{
    Player *pendPlayer=m_player->getPendPlayer();
    //如果当前玩家和出牌玩家不是一伙的并且出牌玩家的牌的数量小于3
    if(pendPlayer != nullptr && pendPlayer!=nullptr && pendPlayer->getRole()!=m_player->getRole() && pendPlayer->getCards().cardCount()<=3)
    {
        QVector<Cards> bombs=findCardsByCount(4);
        for(int i=0;i<bombs.size();++i)
        {
            if(PlayHand(bombs[i]).canBeat(type))
            {
                return bombs[i];
            }
        }
        //没有找到炸弹,去找王炸
        Cards smallJoker=findSamePointCards(Card::CardSmallJoker,1);
        Cards bigJoker=findSamePointCards(Card::CardBigJoker,1);
        if(!smallJoker.isEmpty() && !bigJoker.isEmpty())
        {
            Cards jokers;
            jokers<<smallJoker<<bigJoker;
            return jokers;
        }
    }

    //如果当前玩家和下一个玩家不是一伙的
    Player *nextPlayer=m_player->getNextPlayer();
    Cards remain=m_cards;
    remain.remove(Strategy(m_player,remain).pickOptimalSeqSingles());

    auto beatCard=std::bind([=](const Cards &cards)
    {
        QVector<Cards> beatCardsArray=Strategy(m_player,cards).findCardType(type,true);
        if(!beatCardsArray.isEmpty())
        {
            if(m_player->getRole()!=nextPlayer->getRole() && nextPlayer->getCards().cardCount()<=2)
            {
                //找到最大的牌型压死对方
                return beatCardsArray.back();
            }
            else
            {
                return beatCardsArray.front();
            }
        }
        return Cards();
    },std::placeholders::_1);

    Cards cards;
    if(!(cards=beatCard(remain)).isEmpty())
    {
        return cards;
    }
    //没有找到合适的牌型,就要拆分顺子
    else
    {
       if(!(cards=beatCard(m_cards)).isEmpty())
       {
            return cards;
       }
    }
    return Cards();
}

bool Strategy::whetherToBeat(Cards &cards)
{
    //如果没有能压住对方的牌
    if(cards.isEmpty())
    {
        return false;
    }
    //得到出牌玩家对象,判断是否是一伙的
    Player *pendPlayer=m_player->getPendPlayer();
    if(pendPlayer->getRole()==m_player->getRole())
    {
        Cards left=m_cards;
        left.remove(cards);
        //如果手中的牌所剩无几并且是一个完整的牌型.
        //那么给一次出牌的机会就可以将牌全部打出并且胜利
        if(PlayHand(left).getHandType()!=PlayHand::Hand_Unknown)
        {
            return true;
        }
        Card::CardPoint basePoint=PlayHand(cards).getCardPoint();
        //不能拿高点牌去压自己伙伴
        if(basePoint==Card::Card2 || basePoint==Card::CardSmallJoker || basePoint==Card::CardBigJoker)
        {
            return false;
        }
    }
    else
    {
        PlayHand myHand(cards);
        //如果是三个2带1或者带一对
        if((myHand.getHandType()==PlayHand::Hand_Triple_Single || myHand.getHandType()==PlayHand::Hand_Triple_Pair)
                && myHand.getCardPoint() == Card::Card2)
        {
            return false;
        }
        //如果cards是对2,并且出牌玩家手中的牌的数量>=10,自己手中的牌的数量>=5
        if(myHand.getHandType()==PlayHand::Hand_Pair && myHand.getCardPoint()==Card::Card2
            &&pendPlayer->getCards().cardCount()>=10 && m_player->getCards().cardCount()>=5)
        {
            return false;
        }
    }
    return true;
}

//找点数为point的数量为count的牌
Cards Strategy::findSamePointCards(Card::CardPoint point, int count)
{
    //这里的count是要找的目标牌的数目,一个点数的牌最多有四张
    //我们既然要找牌,肯定不希望找0张牌,最少也得找一张牌
    if(count > 4 || count < 1)
    {
        return Cards();
    }
    if(point == Card::CardSmallJoker || point ==Card::CardBigJoker)
    {
        //大小王的牌只有一张
        if(count>1)
        {
            return Cards();
        }
        //传进来的点数是大小王且count==1
        Card card;
        card.setPoint(point);
        card.setSuit(Card::SuitBegin);
        //判断构造的card对象是否存在
        if(m_cards.contains(card))
        {
            Cards cards;
            cards.add(card);
            return cards;
        }
        //不包含大小王
        return Cards();
    }
    //如果搜索的点数不是大小王,遍历所有的花色
    //除了大小王之外,每一个点数都对应四种花色
    int findCount=0;
    Cards findCards;
    for(int suit=Card::SuitBegin+1;suit<Card::SuitEnd;++suit)
    {
        //基于点数和花色构造card对象
        Card card;
        card.setPoint(point);
        card.setSuit(static_cast<Card::CardSuit>(suit));
        if(m_cards.contains(card))
        {
            findCount++;
            //找到对应的牌保存起来
            findCards.add(card);
            //找到了对应数量的牌
            if(findCount==count)
            {
                return findCards;
            }
        }
    }
    return Cards();
}

QVector<Cards> Strategy::findCardsByCount(int count)
{
    if(count > 4 || count < 1)
    {
        return QVector<Cards>();
    }
    QVector<Cards> cardArray;
    for(Card::CardPoint point=Card::Card3;point<Card::CardEnd;point=static_cast<Card::CardPoint>(point+1))
    {
        if(m_cards.pointCount(point)==count)
        {
            Cards cards;
            cards<<findSamePointCards(point,count);
            cardArray<<cards;
        }
    }
    return cardArray;
}

Cards Strategy::getRangeCards(Card::CardPoint begin, Card::CardPoint end)
{
    Cards rangeCards;
    for(Card::CardPoint point = begin ; point < end ; point = static_cast<Card::CardPoint>(point+1))
    {
        int count=m_cards.pointCount(point);
        //findSamePointCards会返回找到的目标扑克牌
        Cards cards=findSamePointCards(point,count);
        rangeCards<<cards;
    }
    return rangeCards;
}

QVector<Cards> Strategy::findCardType(PlayHand hand, bool beat)
{
    //这个hand对象应该是其他玩家的牌的牌型
    PlayHand::HandType type=hand.getHandType();
    Card::CardPoint point=hand.getCardPoint();
    int extra=hand.getExtra();

    //确定起始点数,如果beat=false,就不关心点数是多少,说明不需要关心之前玩家的出牌
    Card::CardPoint beginPoint=beat ? static_cast<Card::CardPoint>(point+1) : Card::Card3;
    switch(type)
    {
    case PlayHand::Hand_Single:
        return getCards(beginPoint,1);
    case PlayHand::Hand_Pair:
        return getCards(beginPoint,2);
    case PlayHand::Hand_Triple:
        return getCards(beginPoint,3);
    case PlayHand::Hand_Triple_Single:
        return getTripleSingleOrPair(beginPoint,PlayHand::Hand_Single);
    case PlayHand::Hand_Triple_Pair:
        return getTripleSingleOrPair(beginPoint,PlayHand::Hand_Pair);
    case PlayHand::Hand_Plane:
        return getPlane(beginPoint);
    case PlayHand::Hand_Plane_Two_Single:
        return getPlaneTwoSingleOrTwoPair(beginPoint,PlayHand::Hand_Single);
    case PlayHand::Hand_Plane_Two_Pair:
        return getPlaneTwoSingleOrTwoPair(beginPoint,PlayHand::Hand_Pair);
    case PlayHand::Hand_Seq_Pair:
    {
        CardInfo info;
        info.begin=beginPoint;
        info.end=Card::CardQ;
        info.number=2;
        info.base=3;
        info.extra=extra;
        info.beat=beat;
        info.getSeq=&Strategy::getBaseSeqPair;
        return getSeqPairOrSeqSingle(info);
    }
    case PlayHand::Hand_Seq_Single:
    {
        CardInfo info;
        info.begin=beginPoint;
        info.end=Card::Card10;
        info.number=1;
        info.base=5;
        info.extra=extra;
        info.beat=beat;
        info.getSeq=&Strategy::getBaseSeqSingle;
        return getSeqPairOrSeqSingle(info);
    }
    case PlayHand::Hand_Bomb:
        return getBomb(beginPoint);
    default:
        return QVector<Cards>();
    }
}

//allSeqRecord保存每一次for循环的数据
//seqSingle保存每一次递归的顺子
void Strategy::pickSeqSingles(QVector<QVector<Cards>> &allSeqRecord,const QVector<Cards> &seqSingle,const Cards &cards)
{
    //从cards对象中找出真正的能够单独出牌的顺子
    //这个allSeq中保存的顺子不能单独出牌,比如说一组是34567,另一组是3456789这种,他们是相互依赖的
    QVector<Cards> allSeq=Strategy(m_player,cards).findCardType(PlayHand(PlayHand::Hand_Seq_Single,Card::CardBegin,0),false);
    if(allSeq.isEmpty())
    {
        //递归结束,将满足条件的顺子传递给调用者
        //找出一组顺子之后,删除掉这组牌,再从剩余的牌中找顺子
        allSeqRecord<<seqSingle;
    }
    else
    {
        Cards saveCards=cards;
        for(int i=0;i<allSeq.size();++i)
        {
            //将顺子取出
            Cards aScheme=allSeq.at(i);
            Cards temp=saveCards;
            //将这组牌中的顺子删除掉,继续看剩下的牌中有无独立的顺子
            temp.remove(aScheme);
            //每次递归出现的顺子都存储在这里,seq中保存了每一轮递归的顺子数据,传递给seqArray保存
            QVector<Cards> seqArray=seqSingle;
            seqArray<<aScheme;
            //检测还有没有其他的顺子
            // seqArray存储一轮for循环多轮递归得到的所有可用的顺子
            //allSeqRecord存储多轮for循环中多轮递归得到的顺子
            pickSeqSingles(allSeqRecord,seqArray,temp);
        }
    }

}

QVector<Cards> Strategy::pickOptimalSeqSingles()
{
    QVector<QVector<Cards>> seqRecord;
    QVector<Cards> seqSingles;
    Cards save=m_cards;
    save.remove(findCardsByCount(4));   //保护飞机牌型
    save.remove(findCardsByCount(3));   //保护三带一等牌型
    pickSeqSingles(seqRecord,seqSingles,save);
    if(seqRecord.isEmpty())
    {
        return QVector<Cards>();
    }
    QMap<int,int> seqMarks;
    for(int i=0;i<seqRecord.size();++i)
    {
        Cards backupCards=m_cards;
        QVector<Cards> seqArray=seqRecord[i];   //取出一组顺子集合
        backupCards.remove(seqArray);   //删除顺子
        //判断剩下的单牌的数量,数量越少,组合越合理
        //single存储了多种数量为一张的扑克牌对象
        //找出剩下的单牌
        QVector<Cards> singleArray=Strategy(m_player,backupCards).findCardsByCount(1);
        CardList cardList;
        for(int j=0;j<singleArray.size();++j)
        {
            cardList<<singleArray[j].toCardList();
        }
        int mark=0;
        for(int z=0;z<cardList.size();++z)
        {
            mark+=cardList[z].point()+15;
        }
        seqMarks.insert(i,mark);
    }
    int value=0;
    int commonMark=1000;
    for(auto it=seqMarks.constBegin();it!=seqMarks.constEnd();++it)
    {
        if(it.value()<commonMark)
        {
            commonMark=it.value();
            value=it.key();
        }
    }
    return seqRecord[value];
}

//point是从哪一个点数开始搜索,number是找的牌的数量
QVector<Cards> Strategy::getCards(Card::CardPoint point, int number)
{
    QVector<Cards> findCardsArray;
    for(Card::CardPoint pt=point;pt<Card::CardEnd;pt=static_cast<Card::CardPoint>(pt+1))
    {
        // 点数为point的牌的数量是否有number张
        //尽量不拆分别的牌型
        //对应点数的牌是否有number张,如果有的话就取出来
        if(m_cards.pointCount(pt)==number)
        {
            Cards cards=findSamePointCards(pt,number);
            findCardsArray<<cards;
        }
    }
    return findCardsArray;
}

//type是副牌的类型,比如三带一,三带二,这个type就是单牌或者双牌
QVector<Cards> Strategy::getTripleSingleOrPair(Card::CardPoint begin, PlayHand::HandType type)
{
    //找到点数相同的三张牌
    QVector<Cards> findCardArray=getCards(begin,3);
    //搜索单牌或者成对的牌
    if(!findCardArray.isEmpty())
    {
        Cards remainCards=m_cards;
        //将三张牌从用户手中删除
        remainCards.remove(findCardArray);
        Strategy strategy(m_player,remainCards);
        QVector<Cards> cardsArray= strategy.findCardType(PlayHand(type,Card::CardBegin,0),false);
        if(!cardsArray.isEmpty())
        {
            //将找到的牌和三张点数的牌进行组合
            for(int i=0;i<findCardArray.size();++i)
            {
                findCardArray[i].add(cardsArray.at(i));
            }
        }
        else
        {
            findCardArray.clear();
        }
    }
    //如果没有找到,我们直接返回一个空对象
    return findCardArray;
}

QVector<Cards> Strategy::getPlane(Card::CardPoint begin)
{
    QVector<Cards> findCardArray;
    for(Card::CardPoint point=begin;point<=Card::CardK;point=static_cast<Card::CardPoint>(point+1))
    {
        //最大的飞机KKKAAA
        Cards preveCards=findSamePointCards(point,3);
        Cards nextCards=findSamePointCards(static_cast<Card::CardPoint>(point+1),3);
        if(!preveCards.isEmpty() && !nextCards.isEmpty())
        {
            Cards temp;
            temp<<preveCards<<nextCards;
            findCardArray<<temp;
        }
    }
    return findCardArray;
}

QVector<Cards> Strategy::getPlaneTwoSingleOrTwoPair(Card::CardPoint begin, PlayHand::HandType type)
{
    QVector<Cards> findCardArray=getPlane(begin);
    if(!findCardArray.isEmpty())
    {
        Cards remainCards=m_cards;
        remainCards.remove(findCardArray);
        Strategy strategy(m_player,remainCards);
        QVector<Cards> cardsArray= strategy.findCardType(PlayHand(type,Card::CardBegin,0),false);
        //飞机带两张单牌或两个对
        if(cardsArray.size()>=2)
        {
            //将找到的牌和三张点数的牌进行组合
            for(int i=0;i<findCardArray.size();++i)
            {
                Cards temp;
                temp<<cardsArray[0]<<cardsArray[1];
                findCardArray[i].add(temp);
            }
        }
        else
        {
            findCardArray.clear();
        }
    }
    //如果没有找到,我们直接返回一个空对象
    return findCardArray;
}

QVector<Cards> Strategy::getSeqPairOrSeqSingle(CardInfo &info)
{
    QVector<Cards> findCardArray;
    if(info.beat)
    {
        //连对最少是三张,QKA,2不包括连对
        for(Card::CardPoint point=info.begin;point<=info.end;point=static_cast<Card::CardPoint>(point+1))
        {
            bool found=true;    //设置找到了
            Cards seqCards;
            for(int i=0;i<info.extra;++i)
            {
                //获取两张扑克牌
                Cards cards=findSamePointCards(static_cast<Card::CardPoint>(point+i),info.number);
                // extra最小为3或5,最大不知道,对应于自己出牌的话,只要是连对就行,可能会出三张以上
                if(cards.isEmpty() || (point+info.extra>=Card::Card2))
                {
                    found=false;
                    seqCards.clear();
                    break;
                }
                else
                {
                    seqCards<<cards;
                }
            }
            if(found)
            {
                findCardArray<<seqCards;
                return findCardArray;
            }
        }
    }
    else
    {
        for(Card::CardPoint point=info.begin;point<=info.end;point=static_cast<Card::CardPoint>(point+1))
        {
            Cards baseSeq=(this->*info.getSeq)(point);
            //没有找到基础顺子或连对,那就从下一个点数继续查找
            if(baseSeq.isEmpty())
            {
                continue;
            }
            findCardArray<<baseSeq;
            int followed=info.base;
            Cards alerdayFollowedCards;     //存储后续找到的连对
            //不知道后续存在多少连对,所以循环没有范围
            while(true)
            {
                //确定新的起点
                Card::CardPoint followedPoint=static_cast<Card::CardPoint>(point+followed);
                if(followedPoint>=Card::Card2)
                {
                    break;
                }
                Cards followedCards=findSamePointCards(followedPoint,info.number);
                //接下来没有找到连续的两张牌,说明连对截止了,我们就退出循环
                if(followedCards.isEmpty())
                {
                    break;
                }
                else
                {
                    alerdayFollowedCards<<followedCards;
                    Cards newSeq=baseSeq;
                    newSeq<<alerdayFollowedCards;
                    findCardArray<<newSeq;
                    followed++;
                }
            }
        }
    }
    return findCardArray;
}

Cards Strategy::getBaseSeqPair(Card::CardPoint point)
{
    Cards cards0=findSamePointCards(point,2);
    Cards cards1=findSamePointCards(static_cast<Card::CardPoint>(point+1),2);
    Cards cards2=findSamePointCards(static_cast<Card::CardPoint>(point+2),2);
    Cards baseCards;
    if(!cards0.isEmpty() && !cards1.isEmpty() && !cards2.isEmpty())
    {
        baseCards<<cards0<<cards1<<cards2;
    }
    return baseCards;
}

Cards Strategy::getBaseSeqSingle(Card::CardPoint point)
{
    Cards cards0=findSamePointCards(point,1);
    Cards cards1=findSamePointCards(static_cast<Card::CardPoint>(point+1),1);
    Cards cards2=findSamePointCards(static_cast<Card::CardPoint>(point+2),1);
    Cards cards3=findSamePointCards(static_cast<Card::CardPoint>(point+3),1);
    Cards cards4=findSamePointCards(static_cast<Card::CardPoint>(point+4),1);
    Cards baseCards;
    if(!cards0.isEmpty() && !cards1.isEmpty() && !cards2.isEmpty() && !cards3.isEmpty() && !cards4.isEmpty())
    {
        baseCards<<cards0<<cards1<<cards2<<cards3<<cards4;
    }
    return baseCards;
}

QVector<Cards> Strategy::getBomb(Card::CardPoint begin)
{
    QVector<Cards> findCardArray;
    for(Card::CardPoint point=begin;point<Card::CardEnd;point=static_cast<Card::CardPoint>(point+1))
    {
        Cards cards=findSamePointCards(point,4);
        if(!cards.isEmpty())
        {
            findCardArray<<cards;
        }
    }
    return findCardArray;
}
