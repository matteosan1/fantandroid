QT += gui widgets core sql network

# includes openssl libs onto android build
#android {
#  ANDROID_EXTRA_LIBS += /Users/sani/openssl/android-openssl-qt/prebuilt/armeabi-v7a/libcrypto.so
#  ANDROID_EXTRA_LIBS += /Users/sani/openssl/android-openssl-qt/prebuilt/armeabi-v7a/libssl.so
#}

android: {
    QT += androidextras
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
}

deployment.files += team.sqlite
deployment.path = /assets
INSTALLS += deployment

TEMPLATE = app
TARGET = fantandroid

SOURCES = main.cpp \
          mainwindow.cpp \
          insertgiocatori.cpp \
          ComboBoxDelegate.cpp \
          punteggi.cpp \
          prestazione.cpp \
          giocatore.cpp \
          #androidimagepicker.cpp \
          downloadmanager.cpp \
    team.cpp \
    notificationclient.cpp \
    ranking.cpp \
    stats.cpp


HEADERS += mainwindow.h \
           insertgiocatori.h \
           ComboBoxDelegate.h \
           punteggi.h \
           prestazione.h \
           giocatore.h \
           #androidimagepicker.h \
           downloadmanager.h \
    team.h \
    notificationclient.h \
    ranking.h \
    stats.h

FORMS = forms/mainwindow.ui

RESOURCES += \
    app.qrc

DISTFILES += \
    android-sources/AndroidManifest.xml \
    android-sources/gradle/wrapper/gradle-wrapper.jar \
    android-sources/gradlew \
    android-sources/res/values/libs.xml \
    android-sources/build.gradle \
    android-sources/gradle/wrapper/gradle-wrapper.properties \
    android-sources/gradlew.bat

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../../../openssl_for_android/openssl-1.0.2l/libcrypto.so \
        $$PWD/../../../openssl_for_android/openssl-1.0.2l/libssl.so
}



