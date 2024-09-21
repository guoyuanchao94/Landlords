#include "mybutton.h"
#include <QMouseEvent>
#include <QPainter>
MyButton::MyButton(QWidget *parent)
    :QPushButton(parent)
{

}

void MyButton::setImage(QString normal, QString hover, QString pressed)
{
    m_normal = normal;
    m_hover = hover;
    m_pressed = pressed;
    //正常情况下加载正常图片
    m_pixmap.load(m_normal);
    update();
}

void MyButton::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pixmap.load(m_pressed);
        update();
    }
    //处理父类的其他点击事件
    QPushButton::mousePressEvent(event);
}

void MyButton::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_pixmap.load(m_normal);
        update();
    }
    //信号和槽的原因,子类重写父类虚函数
    QPushButton::mouseReleaseEvent(event);
}

void MyButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_pixmap.load(m_hover);
    update();
}

void MyButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_pixmap.load(m_normal);
    update();
}

void MyButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawPixmap(rect(), m_pixmap);
}
