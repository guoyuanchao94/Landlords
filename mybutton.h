#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QMouseEvent>
class MyButton : public QPushButton
{
    Q_OBJECT
public:
    MyButton(QWidget *parent = nullptr);
public:
    //给按钮的正常状态、点击状态、按压状态分别设置对应的图片
    void setImage(QString normal,QString hover,QString pressed);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    //鼠标进入和离开事件
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    QString m_normal;
    QString m_hover;
    QString m_pressed;
    QPixmap m_pixmap;
};

#endif // MYBUTTON_H
