#include "androidimagepicker.h"

#include <QDebug>

#ifdef Q_OS_ANDROID

AndroidImagePicker::AndroidImagePicker()
{}

void AndroidImagePicker::show()
{
//    QAndroidJniObject ACTION_PICK = QAndroidJniObject::fromString("android.intent.action.GET_CONTENT");
////    QAndroidJniObject EXTERNAL_CONTENT_URI = QAndroidJniObject::getStaticObjectField("android/provider/MediaStore$Images$Media",
////                                                                                     "EXTERNAL_CONTENT_URI", "Landroid/net/Uri;");

////    QAndroidJniObject intent = QAndroidJniObject("android/content/Intent", "(Ljava/lang/String;Landroid/net/Uri;)V",
////                                                 ACTION_PICK.object<jstring>(), EXTERNAL_CONTENT_URI.object<jobject>());

////    if (ACTION_PICK.isValid() && intent.isValid())
////    {
////        intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;",
////                                QAndroidJniObject::fromString("image/*").object<jstring>());
////        QtAndroid::startActivity(intent.object<jobject>(), 101, this);
////    }
//    QAndroidJniObject intent("android/content/Intent");
//    if (ACTION_PICK.isValid() && intent.isValid())
//    {
//        intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", ACTION_PICK.object<jstring>());
//        intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", QAndroidJniObject::fromString("image/*").object<jstring>());
//        QtAndroid::startActivity(intent.object<jobject>(), 101, this);
//    }

    QAndroidJniObject ACTION_PICK = QAndroidJniObject::fromString("android.intent.action.GET_CONTENT");
    QAndroidJniObject intent("android/content/Intent");
    if (ACTION_PICK.isValid() && intent.isValid())
    {
        intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", ACTION_PICK.object<jstring>());
        intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", QAndroidJniObject::fromString("image/*").object<jstring>());
        QtAndroid::startActivity(intent.object<jobject>(), 101, this);
    }
}

void AndroidImagePicker::handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject & data)
{
    jint RESULT_OK = QAndroidJniObject::getStaticField<jint>("android/app/Activity", "RESULT_OK");

    if (receiverRequestCode == 101 && resultCode == RESULT_OK) {
        //QString path = data.callObjectMethod("getData", "()Landroid/net/Uri;").callObjectMethod("getPath", "()Ljava/lang/String;").toString();

//        QAndroidJniObject imagePath = QAndroidJniObject::callStaticObjectMethod("com.example.android.tools.QImagePicker",
//                                                                                "getRealPathFromURI",
//                                                                                "(Landroid/net/Uri;)Ljava/lang/String;",
//                                                                                uri.object<jobject>());
//        emit imageSelected(path);
        QAndroidJniObject activity = QtAndroid::androidActivity();
        QAndroidJniObject bytes = QAndroidJniObject::callStaticObjectMethod("com.examples.android.tools.ImageFromActivity",
                                                                            "getImage",
                                                                            "(Landroid/content/Intent;Lorg/qtprojects/qt5/android/bindings/QtActivity;)[B",
                                                                            data.object<jobject>(),
                                                                            activity.object<jobject>());
        QAndroidJniEnvironment env;
        jbyteArray imageDataArray = bytes.object<jbyteArray>();
        if (imageDataArray)
        {
            jsize imageSize = env->GetArrayLength(imageDataArray);
            if (imageSize > 0)
            {
                jboolean isCopy;
                jbyte* imageBytes = env->GetByteArrayElements(imageDataArray, &isCopy);
                QImage image = QImage(QImage::fromData((uchar*)imageBytes, imageSize, "J"));
                env->ReleaseByteArrayElements(imageDataArray, imageBytes, JNI_ABORT);
                emit imageSelected(image);
            }
        }
    }
}

#endif


















