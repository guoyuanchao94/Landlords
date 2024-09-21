#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QTimer>
#include <QWidget>

//倒计时闹钟窗口类
class CountDown : public QWidget
{
    Q_OBJECT
public:
    //倒计时窗口类
    explicit CountDown(QWidget *parent = nullptr);
public:
    void showCountDown();
    void stopCountDown();   //终止闹钟倒计时,玩家在闹钟倒计时结束前出牌了
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPixmap m_clock;    //闹钟图片
    QPixmap m_number;   //数字图片
    QTimer *m_timer;
    int m_count;    //定时器总时长,非机器人玩家愣了一会没出牌再显示闹钟提示
signals:
    void notMuchTime();     //提醒用户玩家没有太多时间了
    void clockTimeout();    //闹钟倒计时结束,强制转移出牌权到下一个玩家
};

#endif // COUNTDOWN_H
