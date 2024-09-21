#ifndef SCOREPANEL_H
#define SCOREPANEL_H

#include <QLabel>
#include <QWidget>

namespace Ui {
class ScorePanel;
}

class ScorePanel : public QWidget
{
    Q_OBJECT

public:
    //文字颜色
    enum FontColor
    {
        Black, White, Red, Blue, Green
    };
    explicit ScorePanel(QWidget *parent = nullptr);
    ~ScorePanel();
public:
    // 设置三个玩家的得分
    void setScores(int left, int right, int user);
    //设置字体大小
    void setMyFontSize(int point);
    //设置字体颜色
    void setMyFontColor(FontColor color);
private:
    Ui::ScorePanel *ui;
private:
    QVector<QLabel *> m_list;   //存储分数面板的控件,用于样式表的展示
};

#endif // SCOREPANEL_H
