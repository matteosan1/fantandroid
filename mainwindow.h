#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QStringList>

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include "giocatore.h"
//#include "androidimagepicker.h"

#define EMPTY_PLAYER "-------------"

#ifdef Q_OS_ANDROID
#define OUTPUT_DIR "/data/data/org.qtproject.fantandroid/databases/"
#else
#define OUTPUT_DIR "/home/sani/databases/"
#endif

class Punteggi;
class GiocatoriInsert;
class DownloadManager;
class QSqlDatabase;
class QComboBox;
class QListWidgetItem;
class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QList<Giocatore*> giocatori();
    QString finalizeMessage();

    void init();
    void loadDB();
    void chooseYourTeam(bool overwrite=false);
    void setTeamLogo(QLabel* qLabel, int size, QString imageName);
    void fillYourTeam();
    void teamSorter(QList<Giocatore*>& players);
    QString checkVersion();
    bool isNewVersionAvailable();

public slots:
    void updateTeam();
    void sendLineup();
    void changeStack();
    //void insertRow();
    //void removeRow();
    void updateModules(const int& chosenModule);
    void playerSelected(QString player);
    void aggiungiPushed();
    void removePanchinaro(QListWidgetItem*);
    //void chooseImage();
    void selectTeam();
    void selectRoster(QString teamName);
    void configDownload(bool exitCode);
    void updateTeamChoice();

    void keyReleaseEvent(QKeyEvent* event);

signals:
    void stackTo(int pageNumber);

private:
    void enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2 = Nullo);
    void disableAllCombo();
    void screenResolution(int& theWidth, int& theHeight);

    //int               m_giornata;

private:
    void fileSave();

    Ui::MainWindowTemp m_ui;

    QString             m_teamLabel;
    QString             m_teamImage;
    bool                m_isModified;
    QList<Giocatore*>   m_players;
    QList<Giocatore*>   m_rosters;
    QString             m_currentFile;
    Punteggi*           m_points;

    GiocatoriInsert*    m_model;
    QSqlDatabase*       m_db;

    QList<QComboBox*>   m_combos;

    bool                m_versionChecked;
    QString             m_version;
    DownloadManager*    m_downloadManager;

//#ifdef Q_OS_ANDROID
//    AndroidImagePicker  m_imagePicker;
//#endif
};

#endif // MAINWINDOW_H
