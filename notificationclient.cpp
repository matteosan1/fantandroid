#include "notificationclient.h"

#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#endif

NotificationClient::NotificationClient(QObject *parent)
    : QObject(parent)
{
    connect(this, SIGNAL(notificationChanged()), this, SLOT(updateAndroidNotification()));
    m_notification = "";
}

void NotificationClient::setNotification(QString notification)
{
    if (m_notification == notification)
        return;

    m_notification = notification;
    emit notificationChanged();
}

QString NotificationClient::notification() const
{
    return m_notification;
}

void NotificationClient::updateAndroidNotification()
{
#ifdef Q_OS_ANDROID
    m_notification = "PIPPO PIPPO";
    //QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject javaNotification = QAndroidJniObject::fromString(m_notification);

    QAndroidJniObject::callStaticMethod<void>("org/qtproject/example/notification/NotificationClient",
                                              "notify",
                                              "(Ljava/lang/String;V)",//,Lorg/qtproject/qt5/android/bindings/QtActivity;)V",
                                              javaNotification.object<jstring>());
                                              //activity.object<jobject>());
    qDebug() << m_notification;

#endif
}
