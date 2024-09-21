#include "gamepanel.h"

#include <QApplication>
#include "loading.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Cards>("Cards");
    qRegisterMetaType<Cards>("Cards&");
    Loading w;
    w.show();
    return a.exec();
}
