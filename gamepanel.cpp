#include "endingpanel.h"
#include "gamepanel.h"
#include "playhand.h"
#include "ui_gamepanel.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QDebug>
GamePanel::GamePanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GamePanel)
{
    ui->setupUi(this);

    //设置背景图,随即加载一张图片
    int num = QRandomGenerator::global()->bounded(10);
    QString path = QString(":/resource/images/background-%1.png").arg(num+1);
    m_bkImage.load(path);

    // setWindowIcon(QIcon(":/resource/images/logo.ico"));
    setWindowTitle("欢乐斗地主");
    setFixedSize(1000, 650);

    gameControlInit();      //初始化游戏控制类对象
    updatePlayerScore();    //分数面板玩家得分更新
    initCardMap();          //切割并存储扑克牌数据
    initButtonsGroup();     //初始化游戏按钮组
    initPlayerContext();    //初始化玩家的上下文环境
    initGameScene();
    initCountDown();    //初始化闹钟倒计时窗口

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GamePanel::onDispatchCard);
    m_animation = new AnimationWindow(this);

    // showEndingScorePanel(); //测试游戏结束面板功能
    // QTimer::singleShot(5000,this,[=](){
    //     showAnimation(LianDui);
    // });
    m_bgm = new BGMControl(this);
}

GamePanel::~GamePanel()
{
    delete ui;
}

void GamePanel::gameControlInit()
{
    m_gameCtrl = new GameControl(this);
    m_gameCtrl->playerInit();
    //获取三个玩家对象
    Robot *leftRobot = m_gameCtrl->getLeftRobot();
    Robot *rightRobot = m_gameCtrl->getRightRobot();
    UserPlayer *user = m_gameCtrl->getUserPlayer();
    //存储顺序,左侧,右侧,用户玩家
    m_playerList << leftRobot << rightRobot << user;

    connect(m_gameCtrl, &GameControl::playerStatusChanged, this, &GamePanel::onPlayerStatusChanged);
    connect(m_gameCtrl, &GameControl::notifyGrabLordBet, this, &GamePanel::onGrabLordBet);
    connect(m_gameCtrl, &GameControl::gameStatusChanged, this, &GamePanel::gameStatusProcess);
    connect(m_gameCtrl, &GameControl::notifyPlayHand, this, &GamePanel::onDisposePlayHand);

    connect(leftRobot, &Robot::notifyPickCards, this, &GamePanel::disposeCard);
    connect(rightRobot, &Robot::notifyPickCards, this, &GamePanel::disposeCard);
    connect(user, &Robot::notifyPickCards, this, &GamePanel::disposeCard);
}

void GamePanel::updatePlayerScore()
{
    ui->scorePanel->setScores(m_playerList[0]->getScore(), m_playerList[1]->getScore(), m_playerList[2]->getScore());
}

void GamePanel::initCardMap()
{
    //加载存储所有扑克牌图片的大图
    QPixmap pixmap(":/resource/images/card.png");
    //计算每张图片的大小,每张图片的宽度和高度
    m_cardSize.setWidth(pixmap.width() / 13);
    m_cardSize.setHeight(pixmap.height() / 5);

    //加载扑克牌的背面图,从大图中抠出小图
    m_cardBackImg = pixmap.copy(m_cardSize.width() * 2, m_cardSize.height() * 4, m_cardSize.width(), m_cardSize.height());

    //正常花色,先遍历花色方片花色♦
    for(int i = 0,suit = Card::SuitBegin+1; suit < Card::SuitEnd; ++suit, ++i)
    {
        for(int j = 0, point = Card::CardBegin+1; point < Card::CardSmallJoker; ++point, ++j)
        {
            Card card(static_cast<Card::CardPoint>(point), static_cast<Card::CardSuit>(suit));
            cropImage(pixmap, j * m_cardSize.width(), i * m_cardSize.height(), card);
        }
    }
    //大小王
    Card cardJoker;
    cardJoker.setPoint(Card::CardSmallJoker);
    cardJoker.setSuit(Card::SuitBegin);
    cropImage(pixmap, 0, 4 * m_cardSize.height(), cardJoker);

    cardJoker.setPoint(Card::CardBigJoker);
    cropImage(pixmap, m_cardSize.width(), 4 * m_cardSize.height(), cardJoker);
}

void GamePanel::cropImage(QPixmap &pixmap, int x, int y,Card &card)
{
    QPixmap sub = pixmap.copy(x, y, m_cardSize.width(), m_cardSize.height());
    CardPanel *panel = new CardPanel(this);
    panel->setImage(sub, m_cardBackImg); //设置前景图和背景图
    //将扑克牌对象的信息设置给窗口
    panel->setCard(card);
    panel->hide();   //先隐藏窗口
    m_cardMap.insert(card, panel); //保存扑克牌的信息和窗口
    connect(panel, &CardPanel::cardSelected, this, &GamePanel::onCardSelected);
}

void GamePanel::initButtonsGroup()
{
    ui->btnGroup->initButtons();
    ui->btnGroup->selectPanel(ButtonGroup::Start);
    //初始化完游戏场景后我们就开始发牌
    connect(ui->btnGroup, &ButtonGroup::startGame, this, [=]()
    {
        //开始游戏按钮组消失,切换到空页
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        //清空所有玩家得分
        m_gameCtrl->clearPlayerScore();
        updatePlayerScore();
        //修改游戏的状态,游戏处于发牌状态
        gameStatusProcess(GameControl::DispatchCard);
        //启动背景音乐的播放
        m_bgm->startBGM(80);
    });
    connect(ui->btnGroup, &ButtonGroup::playHand, this, &GamePanel::onUserPlayHand);
    //跳过出牌
    connect(ui->btnGroup, &ButtonGroup::pass, this, &GamePanel::onUserPass);
    //玩家点击主界面抢地主的那四个按钮,产生不同的分数
    connect(ui->btnGroup, &ButtonGroup::betPoint, this, [=](int point)
    {
        //切换完按钮组之后就开始下注了
        //机器人玩家在下注的时候不需要使用界面上的按钮,通过程序控制
        m_gameCtrl->getUserPlayer()->grabLordBet(point);
        //叫完地主隐藏按钮组窗口
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
    });
}

void GamePanel::initPlayerContext()
{
    //放置玩家扑克牌的区域
    const QRect cardsRect[]=
    {
        QRect(90, 130, 100, height() - 200), //左侧机器人
        QRect(rect().right() - 190, 130, 100, height() - 200), //右侧机器人
        QRect(250, rect().bottom() - 120, width() - 500, 100) //当前玩家
    };
    //玩家出牌区域
    const QRect playHandRect[]=
    {
        QRect(260, 150, 100, 100),
        QRect(rect().right() - 360, 150, 100, 100),
        QRect(150, rect().bottom() - 290, width() - 300, 105)
    };
    //玩家头像显示区域,只需要计算出一个坐标点就好了
    const QPoint roleImagePos[]=
    {
        QPoint(cardsRect[0].left() - 80, cardsRect[0].height() / 2 + 20),
        QPoint(cardsRect[1].right() + 10, cardsRect[1].height() / 2 + 20),
        QPoint(cardsRect[2].right() - 10, cardsRect[2].top() - 10)
    };

    //通过容器获取非机器人玩家对应的索引,找不到返回-1
    int index = m_playerList.indexOf(m_gameCtrl->getUserPlayer());
    //循环显示玩家位置信息,左机器人,右机器人,玩家,我们在实例化游戏控制类的时候
    //按照以上顺序插入了三个玩家对象
    for(int i = 0; i < m_playerList.size(); ++i)
    {
        PlayerContext contetx;
        //如果当前索引等于非机器人玩家索引,扑克牌水平显示,否则垂直显示
        contetx.alignment = i == index ? Horizontal : Vertical;
        contetx.isFrontSide = i == index ? true : false;
        contetx.cardRect = cardsRect[i];
        contetx.playHandRect = playHandRect[i];

        //提示信息,指针首先初始化
        contetx.info = new QLabel(this);
        contetx.info->resize(160, 98);  //小心125的放缩
        contetx.info->hide();

        //提示信息显示到出牌区域的中心位置
        QRect rect=playHandRect[i];     //获取出牌区域

        QPoint point(rect.left() + (rect.width() - contetx.info->width()) / 2,
                     rect.top() + (rect.height() - contetx.info->height()) / 2);
        contetx.info->move(point);

        //玩家头像
        contetx.roleImg = new QLabel(this);
        contetx.roleImg->resize(84, 120); //小心125的放缩
        contetx.roleImg->hide();
        contetx.roleImg->move(roleImagePos[i]);
        m_contextMap.insert(m_playerList.at(i), contetx);
    }
}

void GamePanel::initGameScene()
{
    //发牌区的扑克牌,不需要显示正面图片
    m_baseCard = new CardPanel(this);
    m_baseCard->setImage(m_cardBackImg, m_cardBackImg);
    //移动的扑克牌,也不需要显示正面图片
    m_moveCard = new CardPanel(this);
    m_moveCard->setImage(m_cardBackImg, m_cardBackImg);
    for(int i = 0; i < 3; ++i)
    {
        //构造三张底牌,也不需要正面显示图片
        CardPanel *panel = new CardPanel(this);
        panel->setImage(m_cardBackImg,m_cardBackImg);
        m_lastThreeCard.push_back(panel);
        panel->hide();
    }
    m_baseCardPos = QPoint((width() - m_cardSize.width()) / 2, height() / 2 - 100);

    //动画效果的扑克牌和被分发的牌的起始坐标都是一样的,都移动到相同的位置
    m_baseCard->move(m_baseCardPos);
    m_moveCard->move(m_baseCardPos);

    //移动三张底牌
    //三张扑克牌看成一个整体,计算x坐标
    int base=(width() - 3 * m_cardSize.width() - 2 * 10) / 2;

    for(int i = 0; i < 3; ++i)
    {
        m_lastThreeCard[i]->move(base + (m_cardSize.width() + 10) * i, 20);
    }
}

void GamePanel::gameStatusProcess(GameControl::GameStatus status)
{
    //记录游戏状态
    m_gameStatus = status;
    //处理游戏状态
    switch(status)
    {
    case GameControl::DispatchCard:
        startDiapatchCard();    //开始发牌,发牌前的准备工作
        break;
    case GameControl::CallingLord:
    {
        // 取出底牌
        CardList list = m_gameCtrl->getSurplusCards().toCardList();
        //给底牌窗口设置图片
        for(int i = 0; i < list.size(); ++i)
        {
            QPixmap front = m_cardMap[list.at(i)]->getImage();
            m_lastThreeCard[i]->setImage(front, m_cardBackImg);
            m_lastThreeCard[i]->hide(); //叫地主完成之后显示三张底牌
        }
        //开始叫地主
        m_gameCtrl->startLordCard();
        break;
    }
    case GameControl::PlayingHand:
        //隐藏发牌区的底牌和移动的牌
        m_baseCard->hide();
        m_moveCard->hide();
        //显示留给地主的三张牌
        for(int i = 0;  i< m_lastThreeCard.size(); ++i)
        {
            m_lastThreeCard.at(i)->show();
        }
        //隐藏各个玩家抢地主过程中的提示信息
        for(int i = 0; i < m_playerList.size(); ++i)
        {
            PlayerContext &context=m_contextMap[m_playerList.at(i)];
            context.info->hide();
            //显示各个玩家的头像
            Player *player = m_playerList.at(i);
            QPixmap pixmap = loadRoleImage(player->getSex(), player->getDirection(), player->getRole());
            context.roleImg->setPixmap(pixmap);
            context.roleImg->show();
        }
        break;
    default:
        break;
    }
}

void GamePanel::startDiapatchCard()
{
    //每一局新的游戏开始前都需要对上一句游戏的数据清空
    //重置每张扑克牌的属性
    for(auto it = m_cardMap.begin(); it != m_cardMap.end(); ++it)
    {
        // qDebug()<<static_cast<Card::CardPoint>(it.key().point())<<" "<<static_cast<Card::CardSuit>(it.key().suit());
        //设置窗口为非选中状态
        it.value()->setSelected(false);
        it.value()->setFrontSide(true);   //显示窗口的正面图片
        it.value()->hide();
    }
    //隐藏三张扑克牌,三张底牌在游戏过程中一直显示
    for(int i  =0; i < m_lastThreeCard.size(); ++i)
    {
        m_lastThreeCard.at(i)->hide();
    }
    //重置玩家的上下文信息
    int index=m_playerList.indexOf(m_gameCtrl->getUserPlayer());

    //m_playerList在初始化gameControl类的时候就存储了三个玩家的信息
    for(int i = 0; i < m_playerList.size(); ++i)
    {
        m_contextMap[m_playerList.at(i)].lastCards.clear(); //清空玩家打出的牌
        m_contextMap[m_playerList.at(i)].info->hide();      //隐藏提示
        m_contextMap[m_playerList.at(i)].roleImg->hide();  //隐藏头像
        m_contextMap[m_playerList.at(i)].isFrontSide = i == index ? true : false;
    }
    //重置所有玩家的扑克牌数据
    m_gameCtrl->resetCardData();
    //显示底牌,也就是待分发的牌
    m_baseCard->show();
    //隐藏按钮面板
    ui->btnGroup->selectPanel(ButtonGroup::Empty);
    //开始发牌之后我们启动定时器实现发牌的动画效果
    m_timer->start(10);
    //播放背景音乐
    m_bgm->playAssistMusic(BGMControl::Dispatch);
}

void GamePanel::cardMoveStep(Player *player,int currentPos)
{
    //得到每个玩家扑克牌的展示区域
    QRect cardRect =m_contextMap[player].cardRect;
    //计算每一次移动的步长
    //把扑克牌移动的距离拆分成一百份,每一份的长度
    const int unit[]=
    {
        (m_baseCardPos.x() - cardRect.right()) / 100,
        (cardRect.left() - m_baseCardPos.x()) / 100,
        (cardRect.top() - m_baseCardPos.y()) / 100
    };
    //计算扑克牌的实时坐标
    const QPoint pos[]=
    {
        QPoint(m_baseCardPos.x() - currentPos * unit[0], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x() + currentPos * unit[1], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x(), m_baseCardPos.y() + currentPos * unit[2])
    };
    int index=m_playerList.indexOf(player);
    // m_moveCard是在初始化游戏场景的时候new出来的
    m_moveCard->move(pos[index]);

    //临界状态处理
    if(currentPos == 0)
    {
        m_moveCard->show();
    }
    if(currentPos == 100)
    {
        m_moveCard->hide();
    }
}

void GamePanel::disposeCard(Player *player, const Cards &cards)
{
    //有序排列扑克牌
    Cards &myCards = const_cast<Cards &>(cards);
    CardList list = myCards.toCardList();
    for(int i = 0; i < list.size(); ++i)
    {
        CardPanel *panel=m_cardMap[list.at(i)];
        panel->setOwner(player); //设置扑克牌对象的所有者
    }
    //更新扑克牌在窗口中的显示,定义函数处理
    updatePlayerCards(player);
}

void GamePanel::updatePlayerCards(Player *player)
{
    //把玩家分到的牌显示到该显示的位置上
    Cards cards = player->getCards();
    CardList list = cards.toCardList();   //把QSet容器中的元素排序
    // qDebug()<<list.size();

    m_cardsRect = QRect();
    //每次进入函数,之前的旧数据已经无效了
    m_userCards.clear();
    //取出展示扑克牌的区域,扑克牌要叠加显示
    int cardSpace = 20;
    QRect cardRect = m_contextMap[player].cardRect;
    for(int i = 0;i < list.size(); ++i)
    {
        //m_cardMap存储了所有扑克牌的信息和窗口,在裁剪图片的时候获取到的
        CardPanel *panel = m_cardMap[list.at(i)];
        panel->show();
        panel->raise();   //当前窗口在父窗口处于最上层
        //机器人玩家只展示背面图片
        panel->setFrontSide(m_contextMap[player].isFrontSide);

        //水平显示扑克牌或垂直显示扑克牌,
        // 初始化玩家上下文的时候指定了水平显示还是垂直显示
        if(m_contextMap[player].alignment == Horizontal)
        {
            int leftX = cardRect.left() + (cardRect.width() - (list.size() - 1) * cardSpace-panel->width()) / 2;
            int topY = cardRect.top() + (cardRect.height() - m_cardSize.height()) / 2;
            //鼠标点击事件,如果扑克牌被选中了
            if(panel->isSelected())
            {
                topY -= 10;
            }
            panel->move(leftX + cardSpace * i, topY);

            //计算总的扑克牌的矩形区域
            m_cardsRect = QRect(leftX, topY, cardSpace * i + m_cardSize.width(), m_cardSize.height());

            int currentWidth = 0;
            if(i == list.size()-1)
            {
                currentWidth = m_cardSize.width();
            }
            else
            {
                currentWidth = cardSpace;
            }
            //计算每张扑克牌的矩形区域
            QRect cardPanelRect(leftX + cardSpace * i, topY, currentWidth, m_cardSize.height());
            m_userCards.insert(panel, cardPanelRect);
        }
        else
        {
            int leftX = cardRect.left() + (cardRect.width() - m_cardSize.width()) / 2;
            int topY = cardRect.top() + (cardRect.height() - (list.size()-1) * cardSpace-panel->height()) / 2;
            panel->move(leftX, topY + cardSpace * i);
        }
    }

    //显示玩家打出的牌
    //得到玩家的出牌区域以及本轮打出的牌
    QRect playCardRect = m_contextMap[player].playHandRect;
    Cards lastCards = m_contextMap[player].lastCards;

    //本轮打出的牌不为空
    if(!lastCards.isEmpty())
    {
        int playSpacing = 24; //定义出牌区域的扑克牌堆叠的显示大小
        CardList lastCardList = lastCards.toCardList();
        CardList::ConstIterator itPlayed = lastCardList.constBegin();
        for(int i = 0; itPlayed != lastCardList.constEnd(); ++itPlayed, ++i)
        {
            CardPanel *panel = m_cardMap[*itPlayed];
            //显示正面
            panel->setFrontSide(true);
            panel->raise();
            //将打出的牌移动到出牌区域
            if(m_contextMap[player].alignment == Horizontal)
            {
                int leftBase = playCardRect.left() + (playCardRect.width() - (lastCardList.size() - 1) * playSpacing - panel->width()) / 2;
                int top = playCardRect.top() + (playCardRect.height() - panel->height()) / 2;
                panel->move(leftBase +  i * playSpacing, top);
            }
            else
            {
                int left = playCardRect.left() + (playCardRect.width() - panel->width()) / 2;
                int top=playCardRect.top();
                panel->move(left, top + i * playSpacing);
            }
            panel->show();
        }
    }
}

void GamePanel::showAnimation(AnimationType type, int bet)
{
    switch(type)
    {
    case AnimationType::ShunZi:
    case AnimationType::LianDui:
        m_animation->setFixedSize(250, 150); //顺子和连对的图片大小
        m_animation->move((width()-m_animation->width()) / 2, 200);
        m_animation->showSequence(static_cast<AnimationWindow::Type>(type));
        break;
    case AnimationType::Plane:
        m_animation->setFixedSize(800, 75); //设置成显示的图片的大小
        m_animation->move((width()-m_animation->width()) / 2, 200);
        m_animation->showPlane();
        break;
    case AnimationType::JokerBomb:
        m_animation->setFixedSize(250, 200); //设置成显示的图片的大小
        m_animation->move((width() - m_animation->width()) / 2, (height()-m_animation->height()) / 2 - 70);
        m_animation->showJokerBomb();
        break;
    case AnimationType::Bomb:
        m_animation->setFixedSize(180, 200); //设置成显示的图片的大小
        m_animation->move((width() - m_animation->width()) / 2, (height()-m_animation->height()) / 2 - 70);
        m_animation->showBomb();
        break;
    case AnimationType::Bet:
        m_animation->setFixedSize(160, 98); //设置成显示的图片的大小
        m_animation->move((width() - m_animation->width()) / 2, (height() - m_animation->height()) / 2 - 140);
        m_animation->showBetScore(bet);
        break;
    }
    //用于其他窗口的显示,我们显示完这个分数窗口后还要显示其他的牌型窗口
    m_animation->show();
}

void GamePanel::onDisposePlayHand(Player *player, const Cards &cards)
{
    //存储玩家这一轮打出的牌
    auto it = m_contextMap.find(player);
    (*it).lastCards = cards;
    //2.根据牌型,播放动画效果
    Cards &myCards = const_cast<Cards &>(cards);
    PlayHand hand(myCards);
    PlayHand::HandType type = hand.getHandType();
    if(type == PlayHand::Hand_Plane || type == PlayHand::Hand_Plane_Two_Single || type == PlayHand::Hand_Plane_Two_Pair)
    {
        showAnimation(Plane);
    }
    else if(type == PlayHand::Hand_Seq_Pair)
    {
        showAnimation(LianDui);
    }
    else if(type == PlayHand::Hand_Seq_Single)
    {
        showAnimation(ShunZi);
    }
    else if(type == PlayHand::Hand_Bomb)
    {
        showAnimation(Bomb);
    }
    else if(type == PlayHand::Hand_Bomb_Jokers)
    {
        showAnimation(JokerBomb);
    }
    //如果玩家打出的是空牌
    if(myCards.isEmpty())
    {
        (*it).info->setPixmap(QPixmap(":/resource/images/pass.png"));
        (*it).info->show();
        m_bgm->playPassMusic(static_cast<BGMControl::RoleSex>(player->getSex()));
    }
    else
    {
        if(m_gameCtrl->getPendPlayer() == player || m_gameCtrl->getPendPlayer() == nullptr)
        {
            m_bgm->playCardMusic(cards, true, static_cast<BGMControl::RoleSex>(player->getSex()));
        }
        else
        {
            m_bgm->playCardMusic(cards, false, static_cast<BGMControl::RoleSex>(player->getSex()));
        }
    }
    //3.更新玩家手中的牌的显示
    updatePlayerCards(player);
    //4.播放提示音乐
    //判断玩家牌的剩余的数量
    if(player->getCards().cardCount() == 2)
    {
        m_bgm->playLastMusic(BGMControl::Last2, static_cast<BGMControl::RoleSex>(player->getSex()));
    }
    else if(player->getCards().cardCount() == 1)
    {
        m_bgm->playLastMusic(BGMControl::Last1, static_cast<BGMControl::RoleSex>(player->getSex()));
    }
}

void GamePanel::hidePlayerDropCards(Player *player)
{

    qDebug()<<player->getName();
    auto it=m_contextMap.find(player);
    //判断是否有效
    if(it!=m_contextMap.end())
    {
        //玩家上次打出的牌是空的,会显示一个不要的提示信息
        //我们把这个提示信息隐藏掉
        if(it->lastCards.isEmpty())
        {
            (*it).info->hide();
        }
        else
        {
            CardList list=it->lastCards.toCardList();
            for(auto last=list.begin();last!=list.end();last++)
            {
                m_cardMap[*last]->hide();
            }
        }
        //清空上一次打出的牌
        (*it).lastCards.clear();
    }
}

QPixmap GamePanel::loadRoleImage(Player::Sex sex, Player::Direction direction, Player::Role role)
{
    QVector<QString> lordMan;
    QVector<QString> lordWoman;
    QVector<QString> farmerMan;
    QVector<QString> farmerWoman;
    lordMan << ":/resource/images/lord_man_1.png" << ":/resource/images/lord_man_2.png";
    lordWoman << ":/resource/images/lord_woman_1.png" << ":/resource/images/lord_woman_2.png";
    farmerMan << ":/resource/images/farmer_man_1.png" << ":/resource/images/farmer_man_2.png";
    farmerWoman << ":/resource/images/farmer_woman_1.png" << ":/resource/images/farmer_woman_2.png";

    //加载图片 QPixmap QImage
    QImage image;
    int random=QRandomGenerator::global()->bounded(2);
    if(sex == Player::Man && role == Player::Lord)
    {
        image.load(lordMan.at(random));
    }
    else if(sex == Player::Man && role == Player::Farmer)
    {
        image.load(farmerMan.at(random));
    }
    else if(sex == Player::Woman && role == Player::Lord)
    {
        image.load(lordWoman.at(random));
    }
    else if(sex == Player::Woman && role == Player::Farmer)
    {
        image.load(farmerWoman.at(random));
    }
    QPixmap pixmap;
    if(direction == Player::Left)
    {
        pixmap = QPixmap::fromImage(image);
    }
    else
    {
        pixmap = QPixmap::fromImage(image.mirrored(true, false));
    }
    return pixmap;
}

void GamePanel::showEndingScorePanel()
{
    bool isLord=m_gameCtrl->getUserPlayer()->getRole() == Player::Lord ? true : false;
    bool isWin=m_gameCtrl->getUserPlayer()->isWin();
    EndingPanel *panel = new EndingPanel(isLord,isWin,this);
    panel->show();
    panel->move((width()-panel->width())/2,-panel->height());
    panel->setPlayerScore(m_gameCtrl->getLeftRobot()->getScore(),
                          m_gameCtrl->getRightRobot()->getScore(),
                          m_gameCtrl->getUserPlayer()->getScore());
    if(isWin)
    {
        m_bgm->playEndingMusic(true);
    }
    else
    {
        m_bgm->playEndingMusic(false);
    }
    QPropertyAnimation *animation = new QPropertyAnimation(panel,"geometry",this);
    //设置动画时长
    animation->setDuration(1500);
    //设置窗口的起始位置和终止位置
    animation->setStartValue(QRect(panel->x(),panel->y(),panel->width(),panel->height()));
    animation->setEndValue(QRect((width()-panel->width())/2,(height()-panel->height())/2,panel->width(),panel->height()));
    //设置窗口的运动曲线
    animation->setEasingCurve(QEasingCurve::OutBounce);
    //开始播放动画
    animation->start();
    //处理继续游戏按钮发射出的信号
    connect(panel,&EndingPanel::continueGame,this,[=]()
    {
        panel->close();
        panel->deleteLater();
        animation->deleteLater();
        //不显示按钮组窗口
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        //更改游戏状态到发牌阶段
        gameStatusProcess(GameControl::DispatchCard);
        m_bgm->startBGM(80);
    });
}

void GamePanel::initCountDown()
{
    m_countDown = new CountDown(this);
    m_countDown->move((width() - m_countDown->width()) / 2 ,(height()-m_countDown->height()) / 2 + 30);
    connect(m_countDown,&CountDown::notMuchTime,this,[=]()
    {
        //播放提示音乐
        m_bgm->playAssistMusic(BGMControl::Alert);
    });
    //当前玩家跳过出牌
    connect(m_countDown,&CountDown::clockTimeout,this,&GamePanel::onUserPass);
    UserPlayer *userPlayer = m_gameCtrl->getUserPlayer();
    connect(userPlayer,&UserPlayer::startCountDown,this,[=]()
    {
        //上一次的出牌玩家不是用户玩家并且用户玩家不是第一个出牌
        if(m_gameCtrl->getPendPlayer() != userPlayer && m_gameCtrl->getPendPlayer() != nullptr)
        {
            //显示闹钟倒计时窗口
            m_countDown->showCountDown();
        }
    });
}

void GamePanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawPixmap(rect(), m_bkImage);
}

void GamePanel::mouseMoveEvent(QMouseEvent *event)
{
    //判断鼠标移动时,按中的是否是鼠标左键
    if(event->buttons() & Qt::LeftButton)
    {
        //得到鼠标的位置
        QPoint position = event->pos();
        //判断鼠标位置是否在玩家待出牌区域
        //鼠标没有在待出牌区域
        if(!m_cardsRect.contains(position))
        {
            //没有选中扑克牌
            m_currentSelectedPanel = nullptr;
        }
        //鼠标在待出牌区域
        else
        {
            QList<CardPanel *> list = m_userCards.keys();     //获取所有的键
            for(int i=0;i<list.size();++i)
            {
                CardPanel *panel=list.at(i);
                if(m_userCards[panel].contains(position) && m_currentSelectedPanel != panel)
                {
                    panel->clicked();
                    m_currentSelectedPanel = panel;
                }
            }
        }
    }
}

//定时器溢出槽函数
void GamePanel::onDispatchCard()
{
    //记录扑克牌的位置,定时器溢出一次,该函数就被调用一次
    //所以说我们需要一个静态变量来记录当前扑克牌的位置
    static int currentMovePos = 0;
    //获取当前玩家,给当前玩家发牌,这个当前玩家就是用户玩家,所以说先给用户玩家发牌
    Player *currentPlayer = m_gameCtrl->getCurrentPlayer();
    //如果扑克牌达到指定区域
    if(currentMovePos >= 100)
    {
        //给玩家发一张牌
        Card card = m_gameCtrl->takeOneCard();
        //当前玩家把分到的牌存储到自己的QSet容器中
        currentPlayer->storeDispatchCard(card);

        Cards cards(card);
        // disposeCard(currentPlayer,cards);  //处理分发得到的扑克牌,让扑克牌在主界面显示
        //切换玩家后,currentPos置0,重复动作
        m_gameCtrl->setCurrentPlayer(currentPlayer->getNextPlayer());
        currentMovePos = 0;
        //继续发牌动画
        cardMoveStep(currentPlayer, currentMovePos);
        if(m_gameCtrl->getSurplusCards().cardCount() == 3)
        {
            //终止定时器
            m_timer->stop();
            //发完,切换游戏状态
            gameStatusProcess(GameControl::CallingLord);
            //终止发牌音乐的播放
            m_bgm->stopAssistMusic();
            return;
        }
    }
    //先去实现这个扑克牌的移动效果
    cardMoveStep(currentPlayer, currentMovePos);
    currentMovePos += 15; //currentMovePos最大值是100
}

void GamePanel::onPlayerStatusChanged(Player *player, GameControl::PlayerStatus status)
{
    switch(status)
    {
        //玩家考虑抢地主
    case GameControl::ThinkingForCallLord:        
        if(player == m_gameCtrl->getUserPlayer())
        {
            // 切换按钮组,当前玩家是用户玩家才显示按钮组
            //切换完按钮组之后就开始下注,传递下注的最大分数,用于相关按钮的显示或不显示
            ui->btnGroup->selectPanel(ButtonGroup::CallLord, m_gameCtrl->getPlayerMaxBet());
        }
        break;
    case GameControl::ThinkingForPlayHand:
        //1.玩家出牌前,隐藏上一轮打出去的牌
        hidePlayerDropCards(player);
        if(player == m_gameCtrl->getUserPlayer())
        {
            Player *pendPlayer = m_gameCtrl->getPendPlayer();
            //如果说出牌玩家是用户玩家,或者出牌玩家为空
            if(pendPlayer == m_gameCtrl->getUserPlayer() || pendPlayer == nullptr)
            {
                ui->btnGroup->selectPanel(ButtonGroup::PlayCard);
            }
            else
            {
                ui->btnGroup->selectPanel(ButtonGroup::PassOrPlay);
            }
        }
        //机器人玩家不显示按钮组窗口
        else
        {
            ui->btnGroup->selectPanel(ButtonGroup::Empty);
        }
        break;
    case GameControl::Winning:
        //先终止背景音乐的播放
        m_bgm->stopBGM();
        //玩家胜利
        //展示其他玩家的花色和点数
        m_contextMap[m_gameCtrl->getLeftRobot()].isFrontSide = true;
        m_contextMap[m_gameCtrl->getRightRobot()].isFrontSide = true;
        //左右侧机器人玩家对象更新扑克牌显示
        updatePlayerCards(m_gameCtrl->getLeftRobot());
        updatePlayerCards(m_gameCtrl->getRightRobot());
        //游戏胜利,更新玩家得分
        updatePlayerScore();
        //赢得胜利的玩家下一局优先抢地主
        m_gameCtrl->setCurrentPlayer(player);
        showEndingScorePanel();

        break;
    }
}

void GamePanel::onGrabLordBet(Player *player, int point, bool flag)
{
    // 显示抢地主的信息提示
    //根据键取出key
    PlayerContext context=m_contextMap[player];
    if(point == 0)
    {
        //玩家直接放弃抢地主
        QPixmap pixmap(":/resource/images/buqinag.png");
        context.info->setPixmap(pixmap);
    }
    else
    {
        //玩家第一次抢地主
        if(flag)
        {
            QPixmap pixmap(":/resource/images/jiaodizhu.png");
            context.info->setPixmap(pixmap);
        }
        else
        {
            //不是第一次抢地主
            QPixmap pixmap(":/resource/images/qiangdizhu.png");
            context.info->setPixmap(pixmap);
        }
        //显示叫地主的分数,玩家抢地主下注的分数是0,不显示抢地主的分数
        showAnimation(Bet, point);
    }
    context.info->show();

    //播放相应的音乐
    m_bgm->playerRobLordMusic(point, static_cast<BGMControl::RoleSex>(player->getSex()), flag);
}

void GamePanel::onCardSelected(Qt::MouseButton button)
{
    //判断是不是出牌状态
    if(m_gameStatus == GameControl::DispatchCard || m_gameStatus == GameControl::CallingLord)
    {
        return;
    }
    //判断发出信号的牌的所有者是否是用户玩家
    CardPanel *panel = static_cast<CardPanel *>(sender());
    if(panel->getOwner() != m_gameCtrl->getUserPlayer())
    {
        return;
    }
    //保存选中的牌的窗口对象
    m_currentSelectedPanel = panel;

    //判断参数是鼠标的左键还是右键
    if(button == Qt::LeftButton)
    {
        // 设置扑克牌的选中状态
        panel->setSelected(!panel->isSelected());
        //更新扑克牌在窗口中的显示
        updatePlayerCards(panel->getOwner());
        QSet<CardPanel *>::const_iterator it = m_selectedCards.find(panel); //查看当前扑克牌是否被存储到QSet容器中
        //在容器中没有找到当前扑克牌
        if(it == m_selectedCards.constEnd())
        {
            m_selectedCards.insert(panel);
            qDebug()<<"尺寸: " <<m_selectedCards.size();
        }
        else    //找到了,删除掉
        {
            m_selectedCards.erase(it);
        }
        // qDebug() <<"待出牌的数目: " <<m_selectedCards.size();
        m_bgm->playAssistMusic(BGMControl::SelectCard);
    }
    else if(button == Qt::RightButton)
    {
        //调用出牌按钮的槽函数
        onUserPlayHand();
    }
}

void GamePanel::onUserPlayHand()
{
    //判断游戏状态
    if(m_gameStatus != GameControl::PlayingHand)
    {
        return;
    }
    //判断当前玩家是否是用户玩家
    if(m_gameCtrl->getCurrentPlayer() != m_gameCtrl->getUserPlayer())
    {
        return;
    }
    //判断要出的牌是否是空牌
    if(m_selectedCards.isEmpty())
    {
        return;
    }
    //得到要打出的牌的牌型
    Cards cards;
    for(auto it = m_selectedCards.begin(); it != m_selectedCards.end(); ++it)
    {
        Card card = (*it)->getCard();
        cards.add(card);
    }

    CardList list = cards.toCardList();
    qDebug()<<list.size();

    for(int i = 0;i < list.size(); ++i)
    {
        qDebug()<<"选中的牌的花色"<<list.at(i).suit()<<"点数"<<list.at(i).point();
    }
    //构造牌型
    PlayHand hand(cards);
    PlayHand::HandType type = hand.getHandType();
    if( type == PlayHand::Hand_Unknown)
    {
        return;
    }
    //当前玩家没有优先出牌权,判断当前玩家的牌能否压住上一家的牌
    if(m_gameCtrl->getPendPlayer() != m_gameCtrl->getUserPlayer())
    {
        Cards cards = m_gameCtrl->getPendCards();
        //当前玩家的牌不能压过出牌玩家的牌
        if(!hand.canBeat(PlayHand(cards)))
        {
            return;
        }
    }

    m_countDown->stopCountDown();   //停止倒计时
    //通过玩家对象出牌
    m_gameCtrl->getUserPlayer()->playHand(cards);
    //清空选中的牌
    m_selectedCards.clear();
}

void GamePanel::onUserPass()
{
    m_countDown->stopCountDown();
    //判断是不是用户玩家
    Player *currentPlayer = m_gameCtrl->getCurrentPlayer();
    Player *userPlayer = m_gameCtrl->getUserPlayer();
    if(currentPlayer != userPlayer)
    {
        return;
    }
    //判断当前玩家是不是上一次出牌的玩家
    //如果是,也不能跳过本次出牌
    Player *pendPlayer = m_gameCtrl->getPendPlayer();
    if(pendPlayer == userPlayer || pendPlayer == nullptr)
    {
        return;
    }
    //打出一个空的扑克牌
    Cards empty;
    userPlayer->playHand(empty);
    //清空用户选择的牌,用户可能选择了一些牌,但是没有打出去
    for(auto it = m_selectedCards.begin(); it != m_selectedCards.end(); it++)
    {
        (*it)->setSelected(false);
    }
    m_selectedCards.clear();
    //更新玩家待出牌区域的牌
    updatePlayerCards(userPlayer);
}


