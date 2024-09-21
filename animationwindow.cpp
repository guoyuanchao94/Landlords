#include "animationwindow.h"

#include <QPainter>
#include <QTimer>

//动画效果窗口类
AnimationWindow::AnimationWindow(QWidget *parent)
    : QWidget{parent}
{
}

void AnimationWindow::showBetScore(int bet)
{
    m_x = 0;
    if(bet == 1)
    {
        m_image.load(":/resource/images/score1.png");
    }
    else if(bet == 2)
    {
        m_image.load(":/resource/images/score2.png");
    }
    else if(bet == 3)
    {
        m_image.load(":/resource/images/score3.png");
    }
    update();
    QTimer::singleShot(2000, this, &AnimationWindow::hide);
}

void AnimationWindow::showSequence(Type type)
{
    m_x = 0;
    QString name = "";
    if(type == Sequence)
    {
        name=":/resource/images/shunzi.png";
    }
    else if(type == Pair)
    {
        name = ":/resource/images/liandui.png";
    }
    m_image.load(name);
    update();
    QTimer::singleShot(2000, this, &AnimationWindow::hide);
}

void AnimationWindow::showJokerBomb()
{
    m_index = 0;
    m_x = 0;
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
    {
        m_index++;
        if(m_index > 8)
        {
            timer->stop();
            timer->deleteLater();
            m_index = 8;
            hide();
        }
        QString name = QString(":/resource/images/joker_bomb_%1.png").arg(m_index);
        m_image.load(name);
        update();
    });
    timer->start(60);
}

void AnimationWindow::showBomb()
{
    m_index = 0;
    m_x = 0;
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
    {
        m_index++;
        if(m_index > 12)
        {
            timer->stop();
            timer->deleteLater();
            m_index = 12;
            hide();
        }
        QString name = QString(":/resource/images/bomb_%1.png").arg(m_index);
        m_image.load(name);
        update();
    });
    timer->start(60);
}

void AnimationWindow::showPlane()
{
    m_x = width();  //设置飞机在窗口的最右边
    m_image.load(":/resource/images/plane_1.png");
    setFixedHeight(m_image.height());
    update();

    QTimer *timer = new QTimer(this);
    //把窗口平均分成五份,每一份对应飞机五张图片
    int step = width() / 5;
    connect(timer, &QTimer::timeout, this, [=]()
    {
        static int distance = 0;    //飞机移动的距离
        static int times = 0;      //飞机的编号
        distance += 5;
        if(distance >= step)
        {
            distance = 0;
            times++;
            //循环显示
            QString name = QString(":/resource/images/plane_%1.png").arg(times % 5 + 1);
            m_image.load(name);
        }
        if(m_x <= -110)
        {
            timer->stop();
            timer->deleteLater();
            distance = 0;
            times = 0 ;
            hide();
        }
        m_x -= 5;
        update();
    });
    timer->start(15);
}

void AnimationWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    //保证飞机分绘画位置
    painter.drawPixmap(m_x, 0, m_image.width(), m_image.height(), m_image);
}
