#include "mainwindow.h"

#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QThread>

class I : public QThread
{
public:
    static void sleep(unsigned long secs) {
        QThread::sleep(secs);
    }
};


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(":/res/photo.jpg"); //Insert your splash page image here
    QSplashScreen splash(pixmap);
    splash.show();
    //splash.showMessage(QObject::tr("Testing..."),
    //                   Qt::AlignLeft | Qt::AlignTop, Qt::black);
    MainWindow w;
    w.setWindowTitle("fantandroid");

    qApp->processEvents();

    I::sleep(2);
    splash.finish(&w);
    splash.raise();


    //w.resize(600, 500);
    w.showMaximized();
    //w.show();

    return a.exec();
}
