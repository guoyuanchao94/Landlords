#include "endingpanel.h"
#include <QPixmap>
#include <QPainter>

EndingPanel::EndingPanel(bool isLord, bool isWin,QWidget *parent)
    : QWidget{parent}
{
    m_pixmap.load(":/resource/images/gameover.png");
    this->setFixedSize(m_pixmap.size());

    //显示用户玩家的角色和状态
    m_title = new QLabel(this);
    if(isLord && isWin)
    {
        m_title->setPixmap(QPixmap(":/resource/images/lord_win.png"));
    }
    else if(isLord && !isWin)
    {
        m_title->setPixmap(QPixmap(":/resource/images/lord_fail.png"));
    }
    else if(!isLord && isWin)
    {
        m_title->setPixmap(QPixmap(":/resource/images/farmer_win.png"));
    }
    else if(!isLord && !isWin)
    {
        m_title->setPixmap(QPixmap(":/resource/images/farmer_fail.png"));
    }

    //将标签移动到结束面板的合适的位置
    m_title->move(125, 125);
    //显示分数面板,移动到结束面板中心位置
    m_score=new ScorePanel(this);
    m_score->move(75, 230);
    m_score->setFixedSize(260, 160);
    m_score->setMyFontColor(ScorePanel::Red);
    m_score->setMyFontSize(18);

    //继续游戏按钮
    m_continue = new QPushButton(this);
    m_continue->move(84, 429);
    //按钮设置样式
    QString style = R"(
        QPushButton{border-image:url(:/resource/images/button_normal.png)}
        QPushButton:hover{border-image:url(:/resource/images/button_hover.png)}
        QPushButton:pressed{border-image:url(:/resource/images/button_pressed.png)}
    )";
    m_continue->setStyleSheet(style);
    m_continue->setFixedSize(231, 48);
    connect(m_continue, &QPushButton::clicked, this, &EndingPanel::continueGame);
}

void EndingPanel::setPlayerScore(int left, int right, int user)
{
    m_score->setScores(left, right, user);
}

void EndingPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawPixmap(rect(), m_pixmap);
}
