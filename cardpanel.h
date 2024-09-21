#ifndef CARDPANEL_H
#define CARDPANEL_H

#include <QWidget>
#include "card.h"
#include "player.h"

//单个扑克牌窗口对象
class CardPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CardPanel(QWidget *parent = nullptr);
public:
    //图片有前景图和背景图
    void setImage(const QPixmap &front, const QPixmap &back);
    //获取前景图,毕竟背景图都是一样的
    QPixmap getImage();

    //扑克牌显示哪一面
    void setFrontSide(bool flag);
    //显示的是否是正面
    bool isFrontSide();

    //记录扑克牌是否被选中,扑克牌弹起效果
    void setSelected(bool flag);
    bool isSelected();

    //扑克牌点数和花色信息
    void setCard(const Card &card);
    Card getCard();

    //扑克牌所有者
    void setOwner(Player *player);
    Player *getOwner();

    //模拟扑克牌的点击事件
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QPixmap m_front;  //保存扑克牌前面图片
    QPixmap m_back;   //保存扑克牌后图片
    bool m_isFront = true;   //判断扑克牌是否是正面
    bool m_isSelected = false; //扑克牌是否被选中
    Card m_card;
    Player *m_owner = nullptr;  //扑克牌持有者
signals:
    void cardSelected(Qt::MouseButton);
};

#endif // CARDPANEL_H
