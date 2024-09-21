#ifndef LOADING_H
#define LOADING_H

#include <QWidget>

class Loading : public QWidget
{
    Q_OBJECT
public:
    explicit Loading(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QPixmap m_back;
    QPixmap m_progress;
    int m_distance = 15;    //进度条的默认宽度
signals:

};

#endif // LOADING_H
