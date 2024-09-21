#ifndef ANIMATIONWINDOW_H
#define ANIMATIONWINDOW_H

#include <QWidget>

class AnimationWindow : public QWidget
{
    Q_OBJECT
public:
    //扑克牌类型
    enum Type{ Sequence , Pair};
public:
    explicit AnimationWindow(QWidget *parent = nullptr);
public:
    //显示下注分数
    void showBetScore(int bet);
    //显示顺子和连对
    void showSequence(Type type);
    //显示王炸
    void showJokerBomb();
    //显示炸弹
    void showBomb();
    //显示飞机
    void showPlane();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPixmap m_image;    //加载显示的下注的分数
    int m_index = 0;    //保存加载的第几张图片,炸弹是一个动画效果
    int m_x = 0;        //记录飞机动画的x坐标
signals:

};

#endif // ANIMATIONWINDOW_H
