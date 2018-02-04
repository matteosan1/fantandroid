#include "mainwindow.h"

#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QThread>

#ifdef Q_OS_ANDROID

#include <android/log.h>

#define ANDROIDQUIRKS
#endif
class I : public QThread
{
public:
    static void sleep(unsigned long secs) {
        QThread::sleep(secs);
    }
};

const char* applicationName="fantandroid";

#ifdef ANDROIDQUIRKS  // Set in my myapp.pro file for android builds
void myMessageHandler(
  QtMsgType type,
  const QMessageLogContext& context,
  const QString& msg
) {
  QString report=msg;
  if (context.file && !QString(context.file).isEmpty()) {
    report+=" in file ";
    report+=QString(context.file);
    report+=" line ";
    report+=QString::number(context.line);
  }
  if (context.function && !QString(context.function).isEmpty()) {
    report+=+" function ";
    report+=QString(context.function);
  }
  const char*const local=report.toLocal8Bit().constData();
  switch (type) {
  case QtDebugMsg:
    __android_log_write(ANDROID_LOG_DEBUG,applicationName,local);
    break;
  case QtInfoMsg:
    __android_log_write(ANDROID_LOG_INFO,applicationName,local);
    break;
  case QtWarningMsg:
    __android_log_write(ANDROID_LOG_WARN,applicationName,local);
    break;
  case QtCriticalMsg:
    __android_log_write(ANDROID_LOG_ERROR,applicationName,local);
    break;
  case QtFatalMsg:
  default:
    __android_log_write(ANDROID_LOG_FATAL,applicationName,local);
    abort();
  }
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef ANDROIDQUIRKS
  qInstallMessageHandler(myMessageHandler);
#endif

    QPixmap pixmap(":/res/photo.jpg"); //Insert your splash page image here
    QSplashScreen splash(pixmap);
    splash.show();
    splash.showMessage(QObject::tr("fantandroid v1.3.4"),
                       Qt::AlignLeft | Qt::AlignTop, Qt::white);
    MainWindow w;
    w.setWindowTitle("fantandroid");

    qApp->processEvents();

    I::sleep(2);
    splash.finish(&w);
    splash.raise();

    //w.showMaximized();
    w.resize (600, 500); // initial size
    w.setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);

    w.show();

    return a.exec();
}
