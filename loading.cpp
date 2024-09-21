 #include "loading.h"

#include "gamepanel.h"

#include <QPainter>
#include <QTimer>

Loading::Loading(QWidget *parent)
    : QWidget{parent}
{
    m_back.load(":/resource/images/loading.png");
    this->setFixedSize(m_back.size());

    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    //背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    QPixmap pixmap(":/resource/images/progress.png");
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]()
    {
        m_progress = pixmap.copy(0, 0, m_distance, pixmap.height());
        update();
        if(m_distance >= pixmap.width())
        {
            timer->stop();
            timer->deleteLater();
            //加载游戏主窗口
            GamePanel *panel = new GamePanel;
            panel->show();
            close();
        }
        m_distance += 5;
    });
    timer->start(15);
}

void Loading::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.drawPixmap(rect(),m_back);
    painter.drawPixmap(62, 417, m_progress.width(), m_progress.height(), m_progress);
}
