QT += gui widgets core sql network

android: {
    QT += androidextras
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
}

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



