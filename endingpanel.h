#ifndef ENDINGPANEL_H
#define ENDINGPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "scorepanel.h"
class EndingPanel : public QWidget
{
    Q_OBJECT
public:
    explicit EndingPanel(bool isLord, bool isWin, QWidget *parent = nullptr);
    void setPlayerScore(int left, int right, int user);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPixmap m_pixmap;
    QLabel *m_title;
    ScorePanel *m_score;
    QPushButton *m_continue;    //继续游戏按钮
signals:
    void continueGame();    //点击继续游戏按钮发射的信号
};

#endif // ENDINGPANEL_H
