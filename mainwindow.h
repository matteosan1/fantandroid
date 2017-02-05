#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include "giocatore.h"

#define EMPTY_PLAYER "-------------"

#ifdef Q_OS_ANDROID
#define OUTPUT_DIR "/data/data/org.qtproject.fantandroid/databases/"
#else
#define OUTPUT_DIR "/home/sani/"
#endif

class Punteggi;
class GiocatoriInsert;
class DownloadManager;
class QSqlDatabase;
class QSqlQuery;
class QComboBox;
class QListWidgetItem;
class QKeyEvent;
//class QProgressBar;
class NotificationClient;
class Ranking;
class Stats;

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
    void setTeamLogo(QLabel* qLabel, int size, QByteArray image);
    void fillYourTeam();
    void teamSorter(QList<Giocatore*>& players);
    void checkVersion();
    QByteArray computeMD5(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);
    //bool isNewVersionAvailable();
    void clearTable();
    void clearLineup();
    void clearBench();

public slots:
    //void updateTeam();
    void sendLineup();
    void changeStack();
    void updateModules(const int& chosenModule);
    void playerSelected(QString player);
    void aggiungiPushed();
    void removePanchinaro(QListWidgetItem*);
    void selectTeam();
    void selectRoster(QString teamName);
    void singlePlayerStat(QModelIndex index);
    void configDownload(bool exitCode);
    void initializeApp();

    void updateTeamChoice();

    void keyReleaseEvent(QKeyEvent* event);

signals:
    void stackTo(int pageNumber);

private slots:
    void updateAndroidNotification();
    void sameHashMsg();
    void cannotOpenFileMsg(QString filename);

private:
    QComboBox* returnComboBox(const int index);
    void assignPositionToPlayer(const int realModule);
    void enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2 = Nullo);
    void disableAllCombo();
    void screenResolution(int& theWidth, int& theHeight);
    bool openDB();
    void fillTopScorerRanking();
    void fillTopFlop();
    void fileSave();

    Ui::MainWindowTemp m_ui;

    QString                m_teamLabel;
    QByteArray             m_teamImage;
    bool                   m_isModified;
    QList<Giocatore*>      m_players;
    QList<Giocatore*>      m_rosters;
    QString                m_currentFile;
    Punteggi*              m_points;
    QMap<int, QList<int> > m_roleComboMap;

    GiocatoriInsert*       m_model;
    QSqlDatabase*          m_db;
    QString                m_qryStr;
    QSqlQuery*             m_query;

    QList<QComboBox*>      m_combos;

    DownloadManager*       m_downloadManager;
    bool                   m_downloading;

    NotificationClient*    m_notification;
    Ranking*               m_ranking;
    Stats*                 m_stats;

    int                    m_screenWidth;
    int                    m_screenHeight;
};

#endif // MAINWINDOW_H
