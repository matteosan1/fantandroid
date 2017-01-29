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
    splash.showMessage(QObject::tr("fantandroid v1.3.0"),
                       Qt::AlignLeft | Qt::AlignTop, Qt::white);
    MainWindow w;
    w.setWindowTitle("fantandroid");

    qApp->processEvents();

    I::sleep(2);
    splash.finish(&w);
    splash.raise();


    //w.resize(600, 500);
    //w.showMaximized();
    w.resize (600, 500); // initial size
    w.setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);

    w.show();

    return a.exec();
}
