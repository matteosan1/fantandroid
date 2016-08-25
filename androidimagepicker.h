#ifndef ANDROIDIMAGEPICKER_H
#define ANDROIDIMAGEPICKER_H

#include <QtGlobal>

#ifdef Q_OS_ANDROID

#include <QObject>
#include <QtAndroidExtras>
#include <QImage>

class AndroidImagePicker : public QObject, public QAndroidActivityResultReceiver
{
    Q_OBJECT

public:
    AndroidImagePicker();

    void show();
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject& data);

signals:
    //void imageSelected(QString path);
    void imageSelected(QImage image);
};

#endif
#endif // ANDROIDIMAGEPICKER_H
