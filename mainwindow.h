#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QByteArray>

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include "giocatore.h"

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
class QSqlQuery;
class QComboBox;
class QListWidgetItem;
class QKeyEvent;
//class QProgressBar;

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
    void setTeamLogo(QLabel* qLabel, int size, QByteArray imageName);
    void fillYourTeam();
    void teamSorter(QList<Giocatore*>& players);
    QString checkVersion();
    bool isNewVersionAvailable();
    void clearTable();

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
    void configDownload(bool exitCode);
    void updateTeamChoice();

    void keyReleaseEvent(QKeyEvent* event);

signals:
    void stackTo(int pageNumber);

private:
    void enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2 = Nullo);
    void disableAllCombo();
    void screenResolution(int& theWidth, int& theHeight);
    bool openDB();

private:
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

    bool                   m_versionChecked;
    QString                m_version;
    DownloadManager*       m_downloadManager;
    //QProgressBar*          m_progressBar;
    bool                   m_downloading;
};

#endif // MAINWINDOW_H
