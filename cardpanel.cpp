#include "cardpanel.h"
#include <QPainter>
#include <QMouseEvent>
CardPanel::CardPanel(QWidget *parent)
    : QWidget{parent}
{

}

void CardPanel::setImage(const QPixmap &front, const QPixmap &back)
{
    m_front = front;
    m_back = back;

    //设置为图片的尺寸大小
    setFixedSize(m_front.size());
    //界面重绘
    update();
}

QPixmap CardPanel::getImage()
{
    return m_front;
}

void CardPanel::setFrontSide(bool flag)
{
    //是否以正面显示图片
    m_isFront = flag;
}

bool CardPanel::isFrontSide()
{
    return m_isFront;
}

void CardPanel::setSelected(bool flag)
{
    //扑克牌是否选中
    m_isSelected = flag;
}

bool CardPanel::isSelected()
{
    return m_isSelected;
}

//每一个扑克牌窗口对象都保留着一张扑克牌信息
void CardPanel::setCard(const Card &card)
{
    m_card = card;
}

Card CardPanel::getCard()
{
    return m_card;
}

void CardPanel::setOwner(Player *player)
{
    m_owner = player;
}

Player *CardPanel::getOwner()
{
    return m_owner;
}

void CardPanel::clicked()
{
    emit cardSelected(Qt::LeftButton);
}

void CardPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    if(m_isFront)
    {
        //绘制图片和当前窗口一样大
        painter.drawPixmap(rect(), m_front);
    }
    else
    {
        //绘制背面图片
        painter.drawPixmap(rect(), m_back);
    }
}

void CardPanel::mousePressEvent(QMouseEvent *event)
{
    //游戏主窗口接收这个信号
    emit cardSelected(event->button());
}
