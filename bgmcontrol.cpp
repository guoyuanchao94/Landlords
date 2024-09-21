#include "bgmcontrol.h"

#include "playhand.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QTimer>

BGMControl::BGMControl(QObject *parent)
    : QObject{parent}
{
    for(int i = 0; i < 5; ++i)
    {
        QMediaPlayer *player = new QMediaPlayer(this);
        QMediaPlaylist *list = new QMediaPlaylist(this);
        //设置男女声播放和游戏结束提示音的播放模式为单次播放
        if(i < 2 || i == 4)
        {
            list->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
        }
        if(i == 2)
        {
            //bgm循环播放
            list->setPlaybackMode(QMediaPlaylist::Loop);
        }
        player->setPlaylist(list);
        player->setVolume(100);
        m_players.push_back(player);
        m_list.push_back(list);
    }
    initPlayList();
}

//初始化播放列表
void BGMControl::initPlayList()
{
    QStringList list;
    list << "Man" << "Woman" << "BGM" <<  "Other" << "Ending";
    //读json配置文件
    //解析json文件
    QFile file(":/resource/conf/playList.json");
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "文件打开失败";
        return;
    }
    QByteArray json = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonObject object = document.object();
    for(int i = 0; i < list.size(); ++i)
    {
        QString prefix = list.at(i);
        QJsonArray array = object.value(prefix).toArray();
        for(int j = 0; j < array.size(); ++j)
        {
            m_list[i]->addMedia(QMediaContent(QUrl(array.at(j).toString())));
        }
    }

}

void BGMControl::startBGM(int volume)
{
    m_list[2]->setCurrentIndex(0);
    m_players[2]->setVolume(volume);
    m_players[2]->play();
}

void BGMControl::stopBGM()
{
    m_players[2]->stop();
}

//玩家是否下注
//玩家的性别
//在什么情况下播放什么样的音乐

void BGMControl::playerRobLordMusic(int point, RoleSex sex, bool isFirst)
{
    int index = sex == Man ? 0 : 1;
    //第一个抢地主,播放叫地主的音乐
    if(isFirst && point > 0)
    {
        m_list[index]->setCurrentIndex(Order);
    }
    //放弃抢地主
    else if(point == 0)
    {
        //第一个抢地主的玩家
        if(isFirst)
        {
            //不叫
            m_list[index]->setCurrentIndex(NoOrder);
        }
        else
        {
            //不抢
            m_list[index]->setCurrentIndex(NoRob);
        }
    }
    else if(point == 2)
    {
        m_list[index]->setCurrentIndex(Rob1);
    }
    else if(point == 3)
    {
        m_list[index]->setCurrentIndex(Rob2);
    }
    m_players[index]->play();
}

void BGMControl::playCardMusic(Cards cards, bool isFirst, RoleSex sex)
{
    int index = sex == Man ? 0 : 1;
    QMediaPlaylist *list = m_list[index];
    Card::CardPoint point = Card::CardBegin;
    //取出牌型,进行判断
    PlayHand hand(cards);
    PlayHand::HandType type = hand.getHandType();
    if(type == PlayHand::Hand_Single || type == PlayHand::Hand_Pair || type == PlayHand::Hand_Triple)
    {
        //随机取出一张牌的点数就行,毕竟点数都一样
        point = cards.takeRandomCard().point();
    }
    int number  = 0;
    switch(type)
    {
    //单牌
    case PlayHand::Hand_Single:
        number = point - 1; //对齐
        break;
    case PlayHand::Hand_Pair:
        number = point - 1 + 15;
        break;
    case PlayHand::Hand_Triple:
        number = point - 1 + 15 + 13;
        break;
    case PlayHand::Hand_Triple_Single:
        number = ThreeBindOne;
        break;
    case PlayHand::Hand_Triple_Pair:
        number = ThreeBindPair;
        break;
    case PlayHand::Hand_Plane:
    case PlayHand::Hand_Plane_Two_Single:
    case PlayHand::Hand_Plane_Two_Pair:
        number = Plane;
        break;
    case PlayHand::Hand_Seq_Pair:
        number = SequencePair;
        break;
    case PlayHand::Hand_Seq_Single:
        number = Sequence;
        break;
    case PlayHand::Hand_Bomb:
        number = Bomb;
        break;
    case PlayHand::Hand_Bomb_Jokers:
        number = JokerBomb;
        break;
    case PlayHand::Hand_Bomb_Pair:
    case PlayHand::Hand_Bomb_Two_Single:
    case PlayHand::Hand_Bomb_Jokers_Pair:
    case PlayHand::Hand_Bomb_Jokers_Two_Single:
        number = FourBindTwo;
    default:
        break;
    }
    //如果不是第一个出牌的玩家
    if(!isFirst && (number >= Plane && number <= FourBindTwo))
    {
        list->setCurrentIndex(MoreBiger1 + QRandomGenerator::global()->bounded(2));
    }
    else
    {
        list->setCurrentIndex(number);
    }
    m_players[index]->play();
    if(number == Bomb || number == JokerBomb)
    {
        playAssistMusic(BombVoice);
    }
    if(number == Plane)
    {
        playAssistMusic(PlaneVoice);
    }
}

void BGMControl::playLastMusic(CardType type, RoleSex sex)
{
    int index = sex == Man ? 0 : 1;
    //找到播放列表
    QMediaPlaylist *list = m_list[index];
    if(m_players[index]->state() == QMediaPlayer::StoppedState)
    {
        list->setCurrentIndex(type);
        m_players[index]->play();
    }
    else
    {
        QTimer::singleShot(1500, this, [=]()
        {
            list->setCurrentIndex(type);
            m_players[index]->play();
        });
    }
}

void BGMControl::playPassMusic(RoleSex sex)
{
    int index = sex == Man ? 0 : 1;
    //找到播放列表
    QMediaPlaylist *list = m_list[index];
    int random = QRandomGenerator::global()->bounded(4);
    list->setCurrentIndex(Pass1 + random);
    m_players[index]->play();
}

void BGMControl::playAssistMusic(AssistMusic type)
{
    QMediaPlaylist::PlaybackMode mode;
    if(type == Dispatch)
    {
        //循环播放
        mode = QMediaPlaylist::CurrentItemInLoop;
    }
    else
    {
        //单曲播放一次
        mode = QMediaPlaylist::CurrentItemOnce;
    }
    //找到播放列表
    QMediaPlaylist *list = m_list[3];
    list->setCurrentIndex(type);
    list->setPlaybackMode(mode);
    m_players[3]->play();
}

void BGMControl::stopAssistMusic()
{
    m_players[3]->stop();
}

void BGMControl::playEndingMusic(bool isWin)
{
    if(isWin)
    {
        m_list[4]->setCurrentIndex(0);
    }
    else
    {
        m_list[4]->setCurrentIndex(1);
    }
    m_players[4]->play();
}
