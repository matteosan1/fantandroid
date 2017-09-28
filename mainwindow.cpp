#include "mainwindow.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDir>
#include <QPixmap>
#include <QScroller>
#include <QScreen>
#include <QKeyEvent>

#include <QSslSocket>

//#define LOCAL_DB
#define DEBUG

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>
#endif

#include "ComboBoxDelegate.h"
#include "downloadmanager.h"
#include "punteggi.h"
#include "insertgiocatori.h"
#include "team.h"
#include "notificationclient.h"
#include "ranking.h"
#include "stats.h"

#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_db(NULL),
      m_downloading(false)
{
    connect(this, SIGNAL(destroyed()), this, SLOT(close()));
    m_downloadManager = new DownloadManager();
    connect(m_downloadManager, SIGNAL(downloadDone(bool)), this, SLOT(configDownload(bool)));
    connect(m_downloadManager, SIGNAL(sameHash()), this, SLOT(sameHashMsg()));
    connect(m_downloadManager, SIGNAL(cannotOpenFile(QString)), this, SLOT(cannotOpenFileMsg(QString)));

#ifdef LOCAL_DB
    QFile dfile("assets:/team.sqlite");
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
    filePath.append("/team.sqlite");
    if (dfile.exists()) {
        if (QFile::exists(filePath))
            QFile::remove(filePath);
        if (dfile.copy(filePath))
            QFile::setPermissions(filePath, QFile::WriteOwner | QFile::ReadOwner);
    }
    else {
        QMessageBox::critical(this, "Dir", "Cannot copy databases to its dir.", QMessageBox::Ok);
    }
#endif

    m_ui.setupUi(this);
    screenResolution(m_screenWidth, m_screenHeight);
    resize(m_screenWidth, m_screenHeight);
    setMinimumSize(m_screenWidth, m_screenHeight);  // prevent it from collapsing to zero immediately
    adjustSize(); // resize the window
    setMinimumSize(0, 0);  // allow shrinking afterwards

    checkVersion();

    m_notification = new NotificationClient(this);
}

void MainWindow::sameHashMsg()
{
    //QMessageBox::information(this, "Same Hash", "Local and remote DB have the same hash", QMessageBox::Ok);
    initializeApp();
}

void MainWindow::cannotOpenFileMsg(QString filename)
{
    QMessageBox::critical(this, "Cannot open file", "The " + filename + " file can't be opened", QMessageBox::Ok);
}

MainWindow::~MainWindow()
{
    // todo fix destructor
    if (m_db != NULL) {
        if (m_db->isOpen()) {
            m_db->close();
        }

        delete m_db;
    }
}

void MainWindow::checkVersion()
{
//#ifdef Q_OS_ANDROID
//    QString destination = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
//#else
//    QString destination = OUTPUT_DIR;
//#endif

#ifndef LOCAL_DB

    if (m_downloadManager->isConnectedToNetwork()) {
        //QMessageBox::warning(this, "DB issue", "Download about to start.", QMessageBox::Ok);

        m_downloadManager->execute(REMOTE_DB_FILENAME, OUTPUT_DIR);
    }
    else
    {
        QMessageBox::warning(this, "DB issue", "You must be connected to the network to update the DB.", QMessageBox::Ok);
    }
#else
    initializeApp();
#endif
}

QList<Giocatore *> MainWindow::giocatori()
{
    GiocatoriInsert *temp = dynamic_cast<GiocatoriInsert *>(m_model);
    return temp->GetGiocatori();
}

void MainWindow::configDownload(bool exitCode)
{
    // download failed !
    if (!exitCode) {
        QMessageBox::warning(this, "Connessione fallita", "Impossibile scaricare gli aggiornamenti",  QMessageBox::Cancel);
    }
//    else {
//        QMessageBox::information(this, "Aggiornamento DB", "Aggiornamento completato !",  QMessageBox::Ok);
//    }
    initializeApp();
}

void MainWindow::initializeApp()
{
    if (openDB()) {
        chooseYourTeam();
    }
}

bool MainWindow::openDB()
{

//#ifdef Q_OS_ANDROID
//    QString filename = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
//#else
//    QString filename = OUTPUT_DIR;
//#endif
//    filename.append("/team.sqlite");

    QFile file(OUTPUT_DIR + "/team.sqlite");

    if (file.exists()) {
        if (m_db != NULL and m_db->isOpen()) {
            m_db->close();
        }

        m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "MyConnection"));
        m_db->setDatabaseName(OUTPUT_DIR + "/team.sqlite");
        m_db->open();
        m_ui.stackedWidget->setCurrentIndex(0);

        return true;
    }
    else {
        return false;
    }
}

void MainWindow::chooseYourTeam(bool overwrite)
{
    QString qryStr;
    QSqlQuery query(*m_db);

//    qryStr = "SELECT * from yourTeam;";
//    query.exec(qryStr);
//    query.first();

    QFile file(OUTPUT_DIR + "squadra.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        m_teamLabel = stream.readLine();
        file.close();
    }

    if (m_teamLabel.isEmpty() or overwrite) {
//        if (query.first() and !overwrite) {
//            m_teamLabel = query.value("name").toString();
//        }
//        else {
        QString phoneNumber = QInputDialog::getText(NULL, QString("Inserisci il tuo numero di telefono:"), QString("Numero di Telefono (senza 0039):"));
        qryStr = QString("SELECT name from teamName WHERE phone=\"" + phoneNumber + "\" or secondphone=\"" + phoneNumber + "\";");

        query.exec(qryStr);

        if (query.first()) {

            m_teamLabel = query.value("name").toString();

            if (file.open(QIODevice::WriteOnly)) {
                QTextStream stream(&file);
                stream << m_teamLabel;
                file.close();
            }
        }
        else {
            m_teamLabel = "";
            QMessageBox::critical(this, "ERRORE", "Impossibile trovare il tuo numero nel DB", QMessageBox::Ok);
        }
    //}
    }

    if (!m_teamLabel.isEmpty()) {
        qryStr = QString("DELETE FROM yourTeam;");
        query.exec(qryStr);
        qryStr = QString("INSERT INTO yourTeam (name) VALUES (\"") + m_teamLabel + QString("\");");
        query.exec(qryStr);

        init();
    }
}

void MainWindow::selectTeam()
{
    m_teamLabel = m_ui.comboBoxSelectTeam->currentText();

    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = QString("DELETE * FROM yourTeam;");
    query.exec(qryStr);
    qryStr = QString("INSERT INTO yourTeam (name) VALUES (\"") + m_teamLabel + QString("\");");
    query.exec(qryStr);

    init();
}

void MainWindow::init()
{
    QString qryStr;
    QSqlQuery query(*m_db);
    qryStr = QString("SELECT * from teamName where name = \"" + m_teamLabel + "\";");
    query.exec(qryStr);
    query.first();

    m_teamImage = query.value("image").toByteArray();
    m_ui.labelTeamName->setText(m_teamLabel);
    int height, width;
    screenResolution(width, height);
    int size = std::min(int(width*0.5), int(height*0.2));
    setTeamLogo(m_ui.labelPicture, size, m_teamImage);
    fillYourTeam();

    m_ui.pushButtonTeam->setObjectName("Team");
    m_ui.pushButtonLineup->setObjectName("Lineup");
    connect(m_ui.pushButtonLineup, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    connect(m_ui.pushButtonTeam, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    connect(this, SIGNAL(stackTo(int)), m_ui.stackedWidget, SLOT(setCurrentIndex(int)));

    // setup lineup stack
    m_ui.comboBoxModulo->clear();
    m_points = new Punteggi();
    m_points->loadPunteggi(m_db);

    foreach (QString modulo, m_points->schemi()) {
        m_ui.comboBoxModulo->addItem(modulo);
    }

    connect(m_ui.comboBoxModulo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModules(int)));

    m_ui.pushButtonAggiungiPanchinaro->setObjectName("buttonPanchinaro");
    connect(m_ui.pushButtonAggiungiPanchinaro, SIGNAL(clicked(bool)), this, SLOT(aggiungiPushed()));

    m_ui.listPanchinari->setObjectName("listPanchinari");
    connect(m_ui.listPanchinari, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(removePanchinaro(QListWidgetItem *)));

    m_ui.pushButtonLineupOK->setObjectName("LineupOK");
    connect(m_ui.pushButtonLineupOK, SIGNAL(pressed()), this, SLOT(changeStack()));
    m_ui.pushButtonCancelLineup->setObjectName("LineupCancel");
    connect(m_ui.pushButtonCancelLineup, SIGNAL(pressed()), this, SLOT(changeStack()));

    m_combos.clear();
    m_combos.append(m_ui.comboBoxPortiere);
    m_combos.append(m_ui.comboBoxDifensoreDx);
    m_combos.append(m_ui.comboBoxDifensoreSx);
    m_combos.append(m_ui.comboBoxDifensoreCx1);
    m_combos.append(m_ui.comboBoxDifensoreCx2);
    m_combos.append(m_ui.comboBoxDifensoreCx3);
    m_combos.append(m_ui.comboBoxCentrocampistaDx);
    m_combos.append(m_ui.comboBoxCentrocampistaSx);
    m_combos.append(m_ui.comboBoxCentrocampistaCx1);
    m_combos.append(m_ui.comboBoxCentrocampistaCx2);
    m_combos.append(m_ui.comboBoxCentrocampistaCx3);
    m_combos.append(m_ui.comboBoxTrequartista1);
    m_combos.append(m_ui.comboBoxTrequartista2);
    m_combos.append(m_ui.comboBoxTrequartista3);
    m_combos.append(m_ui.comboBoxAttaccanteDx);
    m_combos.append(m_ui.comboBoxAttaccanteSx);
    m_combos.append(m_ui.comboBoxAttaccanteCx1);
    m_combos.append(m_ui.comboBoxAttaccanteCx2);
    m_combos.append(m_ui.comboBoxAttaccanteCx3);

    int index = 0;

    foreach (QComboBox *combo, m_combos) {
        combo->setObjectName(QString::number(index));
        connect(combo, SIGNAL(currentIndexChanged(QString)), this, SLOT(playerSelected(QString)));
        index++;
    }

    disableAllCombo();

    // setup roster stack
    m_ui.comboBoxRosterChoser->clear();
    m_model = new GiocatoriInsert();
    m_ui.pushButtonRosterExit->setObjectName("RosterExit");
    connect(m_ui.pushButtonRosterExit, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    connect(m_ui.tableViewRosters, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(singlePlayerStat(QModelIndex)));

    m_ui.comboBoxRosterChoser->addItem(EMPTY_PLAYER);

    qryStr = QString("SELECT * from teamName;");
    query.exec(qryStr);

    while (query.next()) {
        m_ui.comboBoxRosterChoser->addItem(query.value("name").toString());
    }

    connect(m_ui.comboBoxRosterChoser, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectRoster(QString)));

    // setup settings stack
    connect(m_ui.pushButtonTeamUpdate, SIGNAL(clicked(bool)), this, SLOT(updateTeamChoice()));
    //connect(m_ui.pushButtonStat, SIGNAL(clicked(bool)), this, SLOT(updateTeamChoice()));
    connect(m_ui.pushButtonNotification, SIGNAL(clicked(bool)), this, SLOT(updateAndroidNotification()));

    // setup ranking stack
    m_ranking = new Ranking(m_screenWidth*0.9, m_db, m_ui.labelScores, m_ui.tableWidgetRanking, m_ui.tableWidgetScores);
    m_ranking->init();
    m_ranking->computeRanking();
    m_ranking->reportResults();
    m_ui.pushButtonRanking->setObjectName("Ranking");
    connect(m_ui.pushButtonRanking, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    m_ui.pushButtonRankingExit->setObjectName("RankingExit");
    connect(m_ui.pushButtonRankingExit, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    connect(m_ui.pushButtonRoundUp, SIGNAL(clicked(bool)), m_ranking, SLOT(changeRoundUp()));
    connect(m_ui.pushButtonRoundDown, SIGNAL(clicked(bool)), m_ranking, SLOT(changeRoundDown()));

    // setup stat stack
    m_stats = new Stats(m_db, m_ui.textEditTopScorer, m_ui.textEditTopWeek, m_ui.textEditFlopWeek, m_ui.textEditTopOverall);
    m_stats->init();
    m_ui.pushButtonStat->setObjectName("Stats");
    connect(m_ui.pushButtonStat, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    m_ui.pushButtonStatExit->setObjectName("StatsExit");
    connect(m_ui.pushButtonStatExit, SIGNAL(clicked(bool)), this, SLOT(changeStack()));

    m_ui.stackedWidget->setCurrentIndex(0);
}

void MainWindow::fillYourTeam()
{
    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = QString("SELECT * from roster where team=\""+ m_teamLabel + "\" order by role, surname, name;");
    query.exec(qryStr);

    m_players.clear();

    while (query.next()) {
        Giocatore *player = new Giocatore();
        player->setCognome(query.value("surname").toString());
        player->setNome(query.value("name").toString());
        player->setRuolo(RuoloEnum(query.value("role").toInt()));

        m_players.append(player);
    }
}

void MainWindow::selectRoster(QString team)
{
    if (team == EMPTY_PLAYER) {
        return;
    }

    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = QString("SELECT * from teamName where name = \"" + team + "\";");
    query.exec(qryStr);
    query.next();
    QByteArray imageName = query.value("image").toByteArray();
    int height, width;
    screenResolution(width, height);
    int size = std::min(int(width*0.25), int(height*0.15));
    setTeamLogo(m_ui.labelTeamLogo, size, imageName);

    qryStr = QString("SELECT * from roster where team=\""+ team + "\" order by role, surname, name;");
    query.exec(qryStr);

    m_model->removeRows(0, m_rosters.size());

    for (int i=0; i<m_rosters.size(); ++i) {
        delete(m_rosters[i]);
    }

    m_rosters.clear();

    while (query.next()) {
        Giocatore *player = new Giocatore();
        player->setCognome(query.value("surname").toString().toUpper());
        player->setNome(query.value("name").toString().toUpper());
        player->setRuolo(RuoloEnum(query.value("role").toInt()));
        player->setSquadra(query.value("realTeam").toString().toUpper());
        player->setPrezzo(query.value("price").toInt());

        m_rosters.append(player);
    }

    teamSorter(m_rosters);
    m_model->insertRows(0, m_rosters.size());
    m_model->setPlayers(m_rosters);

    //    // todo connect the DB to the model of the tableview
    //    //    QSqlTableModel *model = new QSqlTableModel(parentObject, database);
    //    //    model->setTable("A");
    //    //    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //    //    model->select();
    //    //    model->removeColumn(0); // don't show the ID
    //    //    model->setHeaderData(0, Qt::Horizontal, tr("Column 1"));
    //    //    model->setHeaderData(1, Qt::Horizontal, tr("Column 2"));

    //    //    // Attach it to the view
    //    //    ui->view->setModel(model);
    m_ui.tableViewRosters->verticalHeader()->hide();
    m_ui.tableViewRosters->setItemDelegateForColumn(2, new ComboBoxDelegate());
    m_ui.tableViewRosters->setModel(m_model);
    m_ui.tableViewRosters->show();

    m_ui.tableViewRosters->setVisible(false);
    QRect vporig = m_ui.tableViewRosters->viewport()->geometry();
    QRect vpnew = vporig;
    vpnew.setWidth(std::numeric_limits<int>::max());
    m_ui.tableViewRosters->viewport()->setGeometry(vpnew);
    m_ui.tableViewRosters->resizeColumnsToContents();
    m_ui.tableViewRosters->resizeRowsToContents();
    m_ui.tableViewRosters->viewport()->setGeometry(vporig);
    m_ui.tableViewRosters->setVisible(true);

#ifdef Q_OS_ANDROID
    QScroller::grabGesture(m_ui.tableViewRosters->viewport(), QScroller::TouchGesture);
#else
    QScroller::grabGesture(m_ui.tableViewRosters->viewport(), QScroller::LeftMouseButtonGesture);
#endif
    QVariant OvershootPolicy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff);
    QScrollerProperties ScrollerProperties = QScroller::scroller(m_ui.tableViewRosters->viewport())->scrollerProperties();
    ScrollerProperties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, OvershootPolicy);
    ScrollerProperties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, OvershootPolicy);
    QScroller::scroller(m_ui.tableViewRosters->viewport())->setScrollerProperties(ScrollerProperties);

#ifdef Q_OS_ANDROID
    QScroller::grabGesture(m_ui.listPanchinari->viewport(), QScroller::TouchGesture);
#else
    QScroller::grabGesture(m_ui.listPanchinari->viewport(), QScroller::LeftMouseButtonGesture);
#endif
    QScrollerProperties ScrollerProperties2 = QScroller::scroller(m_ui.listPanchinari->viewport())->scrollerProperties();
    ScrollerProperties2.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, OvershootPolicy);
    ScrollerProperties2.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, OvershootPolicy);
    QScroller::scroller(m_ui.listPanchinari->viewport())->setScrollerProperties(ScrollerProperties);
}

void MainWindow::changeStack()
{
    QPushButton *sender = (QPushButton *)QObject::sender();

    if (sender != NULL) {
        if (sender->objectName() == "Team") {
            //if (m_db->transaction())
            emit stackTo(3);
        }
        else if (sender->objectName() == "Lineup") {
            emit stackTo(4);
        }
        else if (sender->objectName() == "LineupOK") {
            int retCapitano = QMessageBox::Ok;
            int retPanchina = QMessageBox::Ok;

            if (m_ui.comboBoxCapitano->currentIndex() == 0) {
                retCapitano = QMessageBox::warning(this, "AVVISO", "Non hai selezionato il capitano.", QMessageBox::Ok | QMessageBox::Cancel);
            }

            //if (m_ui.listPanchinariAttacco->count() == 0 && m_ui.listPanchinariDifesa->count() == 0 && m_ui.listPanchinariCentrocampo->count() == 0)
            if (m_ui.listPanchinari->count() == 0) {
                retPanchina = QMessageBox::warning(this, "AVVISO", "Non hai giocatori da mettere in panchina ?", QMessageBox::Ok | QMessageBox::Cancel);
            }

            if (retCapitano == QMessageBox::Ok && retPanchina == QMessageBox::Ok) {
                sendLineup();
                clearTable();
                emit stackTo(0);
            }
        }
        else if (sender->objectName() == "LineupCancel") {
            QString title = "Formazione";
            QMessageBox::warning(this, title, "Formazione non terminata");
            clearTable();
            emit stackTo(0);
        }
        else if (sender->objectName() == "Ranking") {
            emit stackTo(6);
        }
        else if (sender->objectName() == "Stats") {
            m_stats->fillTopScorerRanking();
            m_stats->fillTopFlop();
            emit stackTo(7);
        }
        else if (sender->objectName() == "RosterExit" or
                 sender->objectName() == "RankingExit" or
                 sender->objectName() == "StatsExit") {
            //updateTeam();
            emit stackTo(0);
        }
    }
}

void MainWindow::updateTeamChoice()
{
    chooseYourTeam(true);
}

void MainWindow::sendLineup()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject jsSubject = QAndroidJniObject::fromString("Formazione " + m_teamLabel);
    QAndroidJniObject jsContent = QAndroidJniObject::fromString(finalizeMessage());

    QAndroidJniObject::callStaticMethod<void>("com.example.android.tools.QShareUtil",
            "share",
            "(Ljava/lang/String;Ljava/lang/String;)V",
            jsSubject.object<jstring>(),
            jsContent.object<jstring>()
                                             );
#else
    finalizeMessage();
#endif
}

void MainWindow::playerSelected(QString player)
{
    QComboBox *sender = (QComboBox *)QObject::sender();

    if (sender != NULL) {
        int senderId = sender->objectName().toInt();

        if (player == EMPTY_PLAYER) {
            foreach (Giocatore *p, m_players) {
                int index = p->partita(0).Formazione();

                if (index == senderId) {
                    p->partita(0).SetFormazione(-1);
                    break;
                }
            }
        }
        else {
            foreach (Giocatore *p, m_players) {
                if (p->nomeCompleto() == player) {
                    int index = p->partita(0).Formazione();

                    foreach (Giocatore *q, m_players) {
                        if (q->partita(0).Formazione() == senderId) {
                            q->partita(0).SetFormazione(-1);
                            break;
                        }
                    }

                    if (index != -1) {
                        m_combos[index]->setCurrentIndex(0);
                    }

                    p->partita(0).SetFormazione(senderId);
                    break;
                }
            }
        }
    }

    int nGiocano = 0;

    foreach (Giocatore *p, m_players) {
        int index = p->partita(0).Formazione();

        if (index != -1 and m_combos[index]->isVisible()) {
            nGiocano++;
        }
    }

    if (nGiocano == 11) {
        m_ui.pushButtonLineupOK->setEnabled(true);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinari->clear();
        m_ui.comboBoxCapitano->addItem(EMPTY_PLAYER);
        m_ui.comboBoxCapitano->setCurrentIndex(0);

        foreach (Giocatore *p, m_players) {
            int index = p->partita(0).Formazione();

            if (index != -1 and m_combos[index]->isVisible()) {
                m_ui.comboBoxCapitano->addItem(p->nomeCompleto());
            }
            else {
                int ruolo = p->ruolo()/10;

                if (ruolo >= 2) {
                    m_ui.comboBoxPanchinari->addItem(p->nomeCompleto());
                }
            }
        }
    }
    else {
        m_ui.pushButtonLineupOK->setEnabled(false);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinari->clear();
    }
}

void MainWindow::clearTable()
{
    clearLineup();
    clearBench();
}

void MainWindow::clearLineup()
{
    m_ui.comboBoxModulo->setCurrentIndex(0); // move somewherelse

    foreach (QComboBox *combo, m_combos) {
        combo->setCurrentIndex(0);
    }

    foreach (Giocatore *p, m_players) {
        p->partita(0).SetFormazione(-1);
    }
}

void MainWindow::clearBench()
{
    m_ui.comboBoxCapitano->setCurrentIndex(0);
    m_ui.comboBoxPanchinari->setCurrentIndex(0);
    m_ui.listPanchinari->clear();
}

QComboBox *MainWindow::returnComboBox(const int index)
{
//    m_combos.append(m_ui.comboBoxPortiere);
//    m_combos.append(m_ui.comboBoxDifensoreDx);        // 1
//    m_combos.append(m_ui.comboBoxDifensoreSx);        // 2
//    m_combos.append(m_ui.comboBoxDifensoreCx1);       // 3
//    m_combos.append(m_ui.comboBoxDifensoreCx2);       // 4
//    m_combos.append(m_ui.comboBoxDifensoreCx3);       // 5
//    m_combos.append(m_ui.comboBoxCentrocampistaDx);   // 6
//    m_combos.append(m_ui.comboBoxCentrocampistaSx);   // 7
//    m_combos.append(m_ui.comboBoxCentrocampistaCx1);  // 8
//    m_combos.append(m_ui.comboBoxCentrocampistaCx2);  // 9
//    m_combos.append(m_ui.comboBoxCentrocampistaCx3);  // 10
//    m_combos.append(m_ui.comboBoxTrequartista1);      // 11
//    m_combos.append(m_ui.comboBoxTrequartista2);      // 12
//    m_combos.append(m_ui.comboBoxTrequartista3);      // 13
//    m_combos.append(m_ui.comboBoxAttaccanteDx);       // 14
//    m_combos.append(m_ui.comboBoxAttaccanteSx);       // 15
//    m_combos.append(m_ui.comboBoxAttaccanteCx1);      // 16
//    m_combos.append(m_ui.comboBoxAttaccanteCx2);      // 17
//    m_combos.append(m_ui.comboBoxAttaccanteCx3);      // 18

#ifdef DEBUG
    qDebug() << index;
#endif
    return m_combos[index];
}

void MainWindow::assignPositionToPlayer(const int realModule)
{
    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = "SELECT * FROM availableModules WHERE id="+QString::number(realModule)+";";

    if (query.exec(qryStr)) {
        while (query.next()) {
            if (query.value(2).toInt() != 0) {
                enableCombo(returnComboBox(query.value(1).toInt()), (RuoloEnum)query.value(2).toInt(), (RuoloEnum)query.value(3).toInt());
            }
            else {
                enableCombo(returnComboBox(query.value(1).toInt()), (RuoloEnum)query.value(2).toInt());
            }
        }
    }
    else {
        qDebug() << query.lastError().text();
    }
}

void MainWindow::updateModules(const int &chosenModule)
{
    clearBench();

    disableAllCombo();
    enableCombo(m_ui.comboBoxPortiere, Portiere);

    assignPositionToPlayer(chosenModule);
}

void MainWindow::enableCombo(QComboBox *combo, RuoloEnum ruolo1, RuoloEnum ruolo2)
{
    int comboId = combo->objectName().toInt();
    combo->setVisible(true);

    QList<int> list;
    list.append(int(ruolo1));
    list.append(int(ruolo2));

    // check if admitted roles are the same
    // if so return otherwise change list of players
    if (list.size() == m_roleComboMap[comboId].size()) {
        bool sameRoles = true;

        for (int i=0; i<list.size(); i++) {
            if (list[i] != m_roleComboMap[comboId].at(i)) {
                sameRoles = false;
            }
        }

        if (sameRoles) {
            return;
        }
    }

    m_roleComboMap[comboId] = list;
    combo->clear();
    combo->addItem(EMPTY_PLAYER);

    int lineupIndex = -1;
    int i=0;

    foreach (Giocatore *p, m_players) {
        if (p->ruolo() == ruolo1 or p->ruolo() == ruolo2) {
            combo->addItem(p->nomeCompleto());
            i++;
        }
        else if (p->ruolo()/10 == (ruolo1/10) or p->ruolo()/10 == (ruolo2/10)) {
            combo->addItem(p->nomeCompleto());
            combo->setItemData(i+1, QColor(Qt::red), Qt::TextColorRole);
            i++;
        }

        if (p->partita(0).Formazione() == comboId) {
            lineupIndex = i-1;
        }
    }

    if (lineupIndex > 0) {
        combo->setCurrentIndex(lineupIndex);
    }

    int width = combo->minimumSizeHint().width();
    combo->view()->setMinimumWidth(width);
}

void MainWindow::disableAllCombo()
{
    //m_ui.comboBoxPortiere->setVisible(false);
    m_ui.comboBoxDifensoreSx->setVisible(false);
    m_ui.comboBoxDifensoreDx->setVisible(false);
    m_ui.comboBoxDifensoreCx1->setVisible(false);
    m_ui.comboBoxDifensoreCx2->setVisible(false);
    m_ui.comboBoxDifensoreCx3->setVisible(false);
    m_ui.comboBoxCentrocampistaCx1->setVisible(false);
    m_ui.comboBoxCentrocampistaCx2->setVisible(false);
    m_ui.comboBoxCentrocampistaCx3->setVisible(false);
    m_ui.comboBoxCentrocampistaSx->setVisible(false);
    m_ui.comboBoxCentrocampistaDx->setVisible(false);
    m_ui.comboBoxAttaccanteCx1->setVisible(false);
    m_ui.comboBoxAttaccanteCx2->setVisible(false);
    m_ui.comboBoxAttaccanteCx3->setVisible(false);
    m_ui.comboBoxTrequartista1->setVisible(false);
    m_ui.comboBoxTrequartista2->setVisible(false);
    m_ui.comboBoxTrequartista3->setVisible(false);
    m_ui.comboBoxAttaccanteDx->setVisible(false);
    m_ui.comboBoxAttaccanteSx->setVisible(false);
}

void MainWindow::aggiungiPushed()
{
    QListWidget *list = m_ui.listPanchinari;
    QComboBox *combo  = m_ui.comboBoxPanchinari;
    QString player = combo->currentText();

    bool alreadyIn = false;

    for (int i=0; i<list->count(); ++i) {
        if (player == list->item(i)->text()) {
            alreadyIn = true;
            break;
        }
    }

    if (!alreadyIn) {
        int row = list->currentRow();

        if (row == -1) {
            row = list->count() + 1;
        }

        list->insertItem(row+1, player);
    }
}

void MainWindow::removePanchinaro(QListWidgetItem *item)
{
    delete item;
}

QString MainWindow::finalizeMessage()
{
    QString text;
    QString capitano = m_ui.comboBoxCapitano->currentText();

    text += m_teamLabel.toUpper() + "\n";
    text += "Modulo: " + m_ui.comboBoxModulo->currentText() + "\n";
    text += "++++++++++++\n";

    foreach (QComboBox *combo, m_combos) {
        if (combo->isVisible()) {
            QString player = combo->currentText();
            RuoloEnum ruolo = Portiere;

            bool isMisplaced = true;

            foreach (Giocatore *p, m_players) {
                if (p->nomeCompleto() == player) {
                    ruolo = p->ruolo();
                    int comboId = combo->objectName().toInt();

                    foreach (int l, m_roleComboMap[comboId]) {
                        if (int(p->ruolo()) == l) {
                            isMisplaced = false;
                        }
                    }
                }
            }

            if (player == capitano) {
                text += "*";
            }

            if (isMisplaced) {
                text += "_" + player.toLower() + " [" + Giocatore::ruoloToString(ruolo) + "]_";
            }
            else {
                text += player + " [" + Giocatore::ruoloToString(ruolo) + "]";
            }

            if (player == capitano) {
                text += " (C)*\n";
            }
            else {
                text += "\n";
            }
        }
    }

    text = text + "--------------\n";
    QListWidget *list = m_ui.listPanchinari;

    for (int i=0; i<list->count(); ++i) {
        QString player = list->item(i)->text();

        foreach (Giocatore *p, m_players) {
            if (player == p->nomeCompleto()) {
                text += player + " [" + Giocatore::ruoloToString(p->ruolo()) + "]\n";
                break;
            }
        }
    }

#ifdef DEBUG
    qDebug() << text;
#endif

    return text;
}

void MainWindow::fileSave()
{
    QString qryStr;
    QSqlQuery query(*m_db);
    QSqlQuery query2(*m_db);

    for (int i=0; i<m_players.size(); ++i) {
        qryStr = "SELECT * FROM roster WHERE name=\""+m_players.at(i)->nome()+"\" and surname=\""+m_players.at(i)->cognome()+"\";";

        if (query.exec(qryStr)) {
            if (!query.first()) {
                qryStr = "INSERT INTO roster (name, surname, role) VALUES (\""+m_players.at(i)->nome()+"\",\""
                         + m_players.at(i)->cognome()+"\","+QString::number(int(m_players.at(i)->ruolo()))+");";

                query2.exec(qryStr);
            }
            else {
                qryStr = QString("UPDATE roster SET ")
                         + QString("name=\"")+m_players.at(i)->nome()+QString("\",")
                         + QString("surname=\"")+m_players.at(i)->cognome()+QString("\",")
                         + QString("role=")+QString::number(int(m_players.at(i)->ruolo()))
                         + QString("WHERE name=\"")+m_players.at(i)->nome()+QString("\" and surname=\"")+m_players.at(i)->cognome()+QString("\";");

                query2.exec(qryStr);
            }
        }
    }

    m_isModified = false;
}

void MainWindow::teamSorter(QList<Giocatore *> &players)
{
    for (int i=0; i<players.size()-1; ++i) {
        for (int j=i+1; j<players.size(); ++j) {
            if ((players[i]->ruolo() > players[j]->ruolo()) ||
                    ((players[i]->nomeCompleto() > players[j]->nomeCompleto()) &&
                     (players[i]->ruolo() == players[j]->ruolo()))) {
                Giocatore *temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }
}

void MainWindow::setTeamLogo(QLabel *qLabel, int size, QByteArray image)
{
//    QImage myImage;
//    myImage.load(imageName);
//    myImage = myImage.scaled(size, size);
//    qLabel->setPixmap(QPixmap::fromImage(myImage));

    QPixmap pixmap;

    if (pixmap.loadFromData(image)) {
        pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    qLabel->setPixmap(pixmap);
}

void MainWindow::screenResolution(int &theWidth, int &theHeight)
{
    QScreen *screen = QApplication::screens().at(0);
    theWidth = screen->availableSize().width();
    theHeight = screen->availableSize().height();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QMainWindow::keyReleaseEvent(event);

    if (event->key() == Qt::Key_Back) {
        event->accept();

        if (m_ui.stackedWidget->currentIndex() == 0 or m_ui.stackedWidget->currentIndex() == 1) {
            qApp->quit();
        }
        else {
            emit stackTo(0);
        }
    }
    else if (event->key() == Qt::Key_Menu) {
        event->accept();

        if (m_ui.stackedWidget->currentIndex() == 0) {
            emit stackTo(5);
        }
    }
}

void MainWindow::singlePlayerStat(QModelIndex index)
{
    int row = index.row();
    QString playerName = m_model->data(m_model->index(row, 1), Qt::DisplayRole).toString();
    QString playerSurname = m_model->data(m_model->index(row, 1), Qt::DisplayRole).toString();
    QString fullName = playerSurname + " " + playerName.left(1) + ".";

    QSqlQuery query(*m_db);
    QString qryStr = "SELECT roster.surname||' '||substr(roster.name,1,1)||'.' AS fn, roster.role,";
    qryStr += "SUM(played),";
    qryStr += "SUM(yellowCard),";
    qryStr += "SUM(redCard),";
    qryStr += "SUM(scored),";
    qryStr += "SUM(conceded),";
    qryStr += "SUM(savedPenalty),";
    qryStr += "SUM(failedPenalty),";
    qryStr += "AVG(vote),";
    qryStr += "AVG(finalVote) AS v";
    qryStr += "FROM roster, playerStats WHERE fn=playerStats.name and playerStats.name=" + fullName + ";";

    query.exec(qryStr);

    if (query.first()) {
        int role = query.value(2).toInt();
        QString textToShow = "Giocate: " + query.value(3).toString() + "\n";
        textToShow += "Giocate: " + query.value(4).toString() + "\n";
        textToShow += "Ammonizioni: " + query.value(5).toString() + "\n";
        textToShow += "Esplusioni: " + query.value(6).toString() + "\n";
        textToShow += "Assist: " + query.value(14).toString() + "\n";

        if (role != 10) {
            textToShow += "Goal: " + query.value(7).toString() + "\n";
        }
        else {
            textToShow += "Goal: " + query.value(8).toString() + "\n";
        }

        if (role != 10) {
            textToShow += "Rigori sbagliati: " + query.value(9).toString() + "\n";
        }
        else {
            textToShow += "Rigori parati: " + query.value(10).toString() + "\n";
        }

        textToShow += "Autogoal: " + query.value(11).toString() + "\n";
        textToShow += "Media voto: " + query.value(12).toString() + "\n";
        textToShow += "Fantamedia: " + query.value(13).toString() + "\n";

        QMessageBox::information(this, fullName, textToShow, QMessageBox::Ok);
    }
}

void MainWindow::updateAndroidNotification()
{
    m_notification->setNotification("hello world");
}
