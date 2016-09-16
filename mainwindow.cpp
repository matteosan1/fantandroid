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

// #define DEBUG

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>
#endif

#include "ComboBoxDelegate.h"
#include "downloadmanager.h"
#include "punteggi.h"
#include "insertgiocatori.h"
#include "team.h"

#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_db(NULL),
      m_versionChecked(false),
      m_downloading(false),
      m_rankingComputed(false),
      m_round(-1),
      m_maxRounds(-1)
{
    connect(this, SIGNAL(destroyed()), this, SLOT(close()));
    m_downloadManager = new DownloadManager();
    connect(m_downloadManager, SIGNAL(downloadDone(bool)), this, SLOT(configDownload(bool)));

    if (!QDir(OUTPUT_DIR).exists())
    {
        QDir().mkdir(OUTPUT_DIR);
    }

    m_ui.setupUi(this);
    m_version = checkVersion();
    m_downloadManager->execute("https://dl.dropboxusercontent.com/s/rb1pqlwb0v4h6jl/version.txt?dl=0", OUTPUT_DIR);
}

MainWindow::~MainWindow()
{
    // todo fix destructor
    if (m_db != NULL)
    {
    //qDebug() << m_db->isOpen() << m_db->isValid();
    if (m_db->isOpen())
    {
        m_db->close();
    }
    delete m_db;
    }
}

QList<Giocatore*> MainWindow::giocatori()
{
    GiocatoriInsert* temp = dynamic_cast<GiocatoriInsert*>(m_model);
    return temp->GetGiocatori();
}

void MainWindow::configDownload(bool exitCode)
{
    // download failed !
    if (!exitCode)
    {
        if (!m_versionChecked)
        {
            // assume current version is ok
            m_versionChecked = true;
        }
        else
            QMessageBox::warning(this, "Connessione fallita", "Impossibile scaricare gli aggiornamenti",  QMessageBox::Cancel);
    }
    else
    {
        if (!m_versionChecked)
        {
            m_versionChecked = true;

            if (isNewVersionAvailable())
            {
                if (openDB())
                    chooseYourTeam();

                m_downloadManager->execute("https://dl.dropboxusercontent.com/s/0p5psz455zjaxb1/team.sqlite?dl=0", OUTPUT_DIR);
                return;
            }
        }
    }

    if (openDB())
    {
        //m_sqlModel = new QSqlTableModel(this, *m_db);
        chooseYourTeam();
    }
}

bool MainWindow::openDB()
{
    QString filename = QString(OUTPUT_DIR)+ QString("team.sqlite");
    QFile file(filename);

    if (file.exists())
    {
        m_db = new QSqlDatabase(QSqlDatabase::addDatabase( "QSQLITE","MyConnection" ));
        m_db->setDatabaseName(QString(OUTPUT_DIR)+ QString("team.sqlite"));
        m_db->open();

        return true;
    }
    else {
        if (!m_downloadManager->isConnectedToNetwork())
        {
            QMessageBox::critical(this, "Impossibile scaricare il DB", "Connettiti ad internet e riavvia fantandroid", QMessageBox::Ok);
            m_ui.stackedWidget->setCurrentIndex(0);
            //if (ret == QMessageBox::Ok)
            //    emit stackTo(0);
        }

        return false;
    }

    return true;
}

QString MainWindow::checkVersion()
{
    QString filename = QString(OUTPUT_DIR)+QString("version.txt");
    QFile file(filename);

    if (!file.exists())
    {
        if (!m_downloadManager->isConnectedToNetwork())
        {
            QMessageBox::critical(this, "Impossibile scaricare il file di versione", "Connettiti ad internet e riavvia fantandroid", QMessageBox::Ok);
            m_ui.stackedWidget->setCurrentIndex(0);
            //if (ret == QMessageBox::Ok)
            //    emit stackTo(0);
        }
        else
        {
            m_downloadManager->execute("https://dl.dropboxusercontent.com/s/rb1pqlwb0v4h6jl/version.txt?dl=0", OUTPUT_DIR);
        }

        return "0";
    }

    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error in reading version file", file.errorString(), QMessageBox::Cancel);
        return "0";
    }

    QTextStream in(&file);
    QString version = in.readLine();

    file.close();

    if (m_db != NULL and !m_db->isOpen())
    {
        if (openDB())
        chooseYourTeam();
    }

    return version;
}

bool MainWindow::isNewVersionAvailable()
{
    QString newVersion = checkVersion();

    if (newVersion.toInt() > m_version.toInt())
        return true;
    else
        return false;
}

void MainWindow::chooseYourTeam(bool overwrite)
{
    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = "SELECT * from yourTeam;";
    query.exec(qryStr);
    query.first();

    if (m_teamLabel.isEmpty() or overwrite)
    {
        if (query.first() and !overwrite)
        {
            m_teamLabel = query.value("name").toString();
        }
        else
        {
            QString phoneNumber = QInputDialog::getText(NULL, QString("Inserisci il tuo numero di telefono:"), QString("Numero di Telefono (senza 0039):"));
            //m_sqlModel->setFilter("phone='"+phonenumber+"';");
            //if (m_sqlModel->select())

            qryStr = QString("SELECT name from teamName WHERE phone=\""+phoneNumber+"\";");

            query.exec(qryStr);
            if (query.first())
            {
                m_teamLabel = query.value("name").toString();
            }
            else
            {
                m_teamLabel = "";
                QMessageBox::critical(this, "ERRORE", "Impossibile trovare il tuo numero nel DB", QMessageBox::Ok);
            }
        }
    }

    if (!m_teamLabel.isEmpty())
    {
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
    int size = std::min(int(width*0.5), int(height*0.3));
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
    m_ui.comboBoxModulo->addItem(m_points->m_schemiString[0]);
    for (int i=1; i<NUMERO_SCHEMI; i++)
    {
        //if (m_punteggi->m_schemi[i-1] == 1)
        m_ui.comboBoxModulo->addItem(m_points->m_schemiString[i]);
    }

    connect(m_ui.comboBoxModulo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModules(int)));

    m_ui.pushButtonAggiungiDifensore->setObjectName("buttonDifesa");
    m_ui.pushButtonAggiungiCentrocampista->setObjectName("buttonCentrocampo");
    m_ui.pushButtonAggiungiAttaccante->setObjectName("buttonAttacco");
    connect(m_ui.pushButtonAggiungiDifensore, SIGNAL(clicked(bool)), this, SLOT(aggiungiPushed()));
    connect(m_ui.pushButtonAggiungiCentrocampista, SIGNAL(clicked(bool)), this, SLOT(aggiungiPushed()));
    connect(m_ui.pushButtonAggiungiAttaccante, SIGNAL(clicked(bool)), this, SLOT(aggiungiPushed()));

    m_ui.listPanchinariDifesa->setObjectName("listDifesa");
    m_ui.listPanchinariCentrocampo->setObjectName("listCentrocampo");
    m_ui.listPanchinariAttacco->setObjectName("listAttacco");
    connect(m_ui.listPanchinariDifesa, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removePanchinaro(QListWidgetItem*)));
    connect(m_ui.listPanchinariCentrocampo, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removePanchinaro(QListWidgetItem*)));
    connect(m_ui.listPanchinariAttacco, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removePanchinaro(QListWidgetItem*)));

    m_ui.pushButtonLineupOK->setObjectName("LineupOK");
    connect(m_ui.pushButtonLineupOK, SIGNAL(pressed()), this, SLOT(changeStack()));
    m_ui.pushButtonCancelLineup->setObjectName("LineupCancel");
    connect(m_ui.pushButtonCancelLineup, SIGNAL(pressed()), this, SLOT(changeStack()));

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
    foreach (QComboBox* combo, m_combos)
    {
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

    m_ui.comboBoxRosterChoser->addItem(EMPTY_PLAYER);

    qryStr = QString("SELECT * from teamName;");
    query.exec(qryStr);
    while (query.next())
    {
        m_ui.comboBoxRosterChoser->addItem(query.value("name").toString());
    }
    connect(m_ui.comboBoxRosterChoser, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectRoster(QString)));

    // setup settings stack
    connect(m_ui.pushButtonTeamUpdate, SIGNAL(clicked(bool)), this, SLOT(updateTeamChoice()));

    // setup ranking stack
    m_ui.pushButtonRanking->setObjectName("Ranking");
    connect(m_ui.pushButtonRanking, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    m_ui.pushButtonRankingExit->setObjectName("RankingExit");
    connect(m_ui.pushButtonRankingExit, SIGNAL(clicked(bool)), this, SLOT(changeStack()));
    connect(m_ui.pushButtonRoundUp, SIGNAL(clicked(bool)), this, SLOT(changeRoundUp()));
    connect(m_ui.pushButtonRoundDown, SIGNAL(clicked(bool)), this, SLOT(changeRoundDown()));

    m_ui.stackedWidget->setCurrentIndex(0);
}

void MainWindow::fillYourTeam()
{
    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = QString("SELECT * from roster where team=\""+ m_teamLabel + "\" order by role, surname, name;");
    query.exec(qryStr);

    m_players.clear();
    while(query.next())
    {
        Giocatore* player = new Giocatore();
        player->setCognome(query.value("surname").toString());
        player->setNome(query.value("name").toString());
        player->setRuolo(RuoloEnum(query.value("role").toInt()));

        m_players.append(player);
    }
}

void MainWindow::selectRoster(QString team)
{
    if (team == EMPTY_PLAYER)
        return;

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
    for (int i=0; i<m_rosters.size(); ++i)
    {
        delete(m_rosters[i]);
    }
    m_rosters.clear();

    while(query.next())
    {
        Giocatore* player = new Giocatore();
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
    QScroller::grabGesture(m_ui.listPanchinariDifesa->viewport(), QScroller::TouchGesture);
#else
    QScroller::grabGesture(m_ui.listPanchinariDifesa->viewport(), QScroller::LeftMouseButtonGesture);
#endif
    QScrollerProperties ScrollerProperties2 = QScroller::scroller(m_ui.listPanchinariDifesa->viewport())->scrollerProperties();
    ScrollerProperties2.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, OvershootPolicy);
    ScrollerProperties2.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, OvershootPolicy);
    QScroller::scroller(m_ui.listPanchinariDifesa->viewport())->setScrollerProperties(ScrollerProperties);

#ifdef Q_OS_ANDROID
    QScroller::grabGesture(m_ui.listPanchinariCentrocampo->viewport(), QScroller::TouchGesture);
#else
    QScroller::grabGesture(m_ui.listPanchinariCentrocampo->viewport(), QScroller::LeftMouseButtonGesture);
#endif
    QScrollerProperties ScrollerProperties3 = QScroller::scroller(m_ui.listPanchinariCentrocampo->viewport())->scrollerProperties();
    ScrollerProperties3.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, OvershootPolicy);
    ScrollerProperties3.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, OvershootPolicy);
    QScroller::scroller(m_ui.listPanchinariCentrocampo->viewport())->setScrollerProperties(ScrollerProperties);

#ifdef Q_OS_ANDROID
    QScroller::grabGesture(m_ui.listPanchinariAttacco->viewport(), QScroller::TouchGesture);
#else
    QScroller::grabGesture(m_ui.listPanchinariAttacco->viewport(), QScroller::LeftMouseButtonGesture);
#endif
    QScrollerProperties ScrollerProperties4 = QScroller::scroller(m_ui.listPanchinariAttacco->viewport())->scrollerProperties();
    ScrollerProperties4.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, OvershootPolicy);
    ScrollerProperties4.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, OvershootPolicy);
    QScroller::scroller(m_ui.listPanchinariAttacco->viewport())->setScrollerProperties(ScrollerProperties);
}

void MainWindow::changeStack()
{
    QPushButton* sender = (QPushButton*)QObject::sender();
    if (sender != NULL)
    {
        if (sender->objectName() == "Team")
        {
            //if (m_db->transaction())
                emit stackTo(3);
        }
        else if (sender->objectName() == "Lineup")
        {
            emit stackTo(4);
        }
        else if (sender->objectName() == "LineupOK")
        {
            int retCapitano = QMessageBox::Ok;
            int retPanchina = QMessageBox::Ok;

            if (m_ui.comboBoxCapitano->currentIndex() == 0)
            {
                retCapitano = QMessageBox::warning(this, "AVVISO", "Non hai selezionato il capitano.", QMessageBox::Ok | QMessageBox::Cancel);
            }

            if (m_ui.listPanchinariAttacco->count() == 0 && m_ui.listPanchinariDifesa->count() == 0 && m_ui.listPanchinariCentrocampo->count() == 0)
            {
                retPanchina = QMessageBox::warning(this, "AVVISO", "Non hai giocatori da mettere in panchina ?", QMessageBox::Ok | QMessageBox::Cancel);
            }

            if (retCapitano == QMessageBox::Ok && retPanchina == QMessageBox::Ok)
            {
                sendLineup();
                clearTable();
                emit stackTo(0);
            }
        }
        else if (sender->objectName() == "LineupCancel")
        {
            QString title = "Formazione";
            QMessageBox::warning(this, title, "Formazione non terminata");
            clearTable();
            emit stackTo(0);
        }
        else if (sender->objectName() == "Ranking")
        {
            computeRanking();
            emit stackTo(6);
        }
        else if (sender->objectName() == "RosterExit" or sender->objectName() == "RankingExit")
        {
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
#endif
}

void MainWindow::playerSelected(QString player)
{
    QComboBox* sender = (QComboBox*)QObject::sender();
    if (sender != NULL)
    {
        int senderId = sender->objectName().toInt();

        if (player == EMPTY_PLAYER)
        {
            foreach (Giocatore* p, m_players)
            {
                int index = p->partita(0).Formazione();

                if (index == senderId)
                {
                    p->partita(0).SetFormazione(-1);
                }
            }
        }
        else {
            foreach (Giocatore* p, m_players)
            {
                if (p->nomeCompleto() == player)
                {
                    int index = p->partita(0).Formazione();
                    p->partita(0).SetFormazione(senderId);
                    if (index != -1)
                    {
                        m_combos.at(index)->setCurrentIndex(0);
                        break;
                    }
                }
            }
        }
    }

    int nGiocano = 0;
    foreach (Giocatore* p, m_players)
    {
        int index = p->partita(0).Formazione();
        if (index != -1 and m_combos[index]->isVisible())
            nGiocano++;
    }

    //qDebug() << nGiocano;
    if (nGiocano == 11)
    {
        m_ui.pushButtonLineupOK->setEnabled(true);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinariDifesa->clear();
        m_ui.comboBoxPanchinariCentrocampo->clear();
        m_ui.comboBoxPanchinariAttacco->clear();

        m_ui.comboBoxCapitano->addItem(EMPTY_PLAYER);
        m_ui.comboBoxCapitano->setCurrentIndex(0);

        foreach (Giocatore* p, m_players)
        {
            int index = p->partita(0).Formazione();
            if (index != -1 and m_combos[index]->isVisible())
                m_ui.comboBoxCapitano->addItem(p->nomeCompleto());
            else
            {
                int ruolo = p->ruolo()/10;
                if (ruolo == 2)
                    m_ui.comboBoxPanchinariDifesa->addItem(p->nomeCompleto());
                else if (ruolo == 3)
                    m_ui.comboBoxPanchinariCentrocampo->addItem(p->nomeCompleto());
                else if (ruolo == 4)
                    m_ui.comboBoxPanchinariAttacco->addItem(p->nomeCompleto());
            }
        }
    }
    else
    {
        m_ui.pushButtonLineupOK->setEnabled(false);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinariDifesa->clear();
        m_ui.comboBoxPanchinariCentrocampo->clear();
        m_ui.comboBoxPanchinariAttacco->clear();
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
    foreach (QComboBox* combo, m_combos)
        combo->setCurrentIndex(0);

    foreach (Giocatore* p, m_players)
        p->partita(0).SetFormazione(-1);
}

void MainWindow::clearBench()
{
    m_ui.comboBoxCapitano->setCurrentIndex(0);
    m_ui.comboBoxPanchinariDifesa->setCurrentIndex(0);
    m_ui.comboBoxPanchinariCentrocampo->setCurrentIndex(0);
    m_ui.comboBoxPanchinariAttacco->setCurrentIndex(0);
    m_ui.listPanchinariDifesa->clear();
    m_ui.listPanchinariCentrocampo->clear();
    m_ui.listPanchinariAttacco->clear();
}

void MainWindow::updateModules(const int& chosenModule)
{
    clearBench();

    int realModule = -1;
    int counter = 0;
    for (int i=1; i<NUMERO_SCHEMI; i++)
    {
        //if (m_punteggi->m_schemi[i-1] != 0)
        counter++;

        if (counter == chosenModule)
            realModule = i-1;
    }

    disableAllCombo();
    enableCombo(m_ui.comboBoxPortiere, Portiere);

    switch(realModule)
    {
    case 0: // 5-4-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        break;
    case 1: // 5-3-1-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxTrequartista3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteMovimento);
        break;
    case 2: // 5-3-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 3: // 4-5-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        break;
    case 4: // 4-4-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 5: // 4-3-2-1
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxTrequartista1, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxTrequartista2, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        break;
    case 6: // 4-3-1-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxTrequartista3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteCentrale, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteCentrale, AttaccanteMovimento);
        break;
    case 7: // 4-3-3
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        break;
    case 8: // 4-2-3-1
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxTrequartista3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxTrequartista1, CentrocampistaEsterno, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxTrequartista2, CentrocampistaEsterno, CentrocampistaTrequartista);
        break;
    case 9: // 4-2-1-3
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxTrequartista3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        break;
    case 10: // 4-2-4
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx1, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx2, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, CentrocampistaEsterno);
        break;
    case 11: // 3-5-2
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 12: // 3-4-1-2
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxTrequartista3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento, AttaccanteCentrale);
        break;
    case 13: // 3-4-3
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx3, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno);
        enableCombo(m_ui.comboBoxAttaccanteCx3, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        break;
    }
}

void MainWindow::enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2)
{
    int comboId = combo->objectName().toInt();
    QList<int> list;
    list.append(int(ruolo1));
    list.append(int(ruolo2));
    m_roleComboMap[comboId] = list;

    combo->setEnabled(true);
    combo->setVisible(true);

    combo->clear();
    combo->addItem(EMPTY_PLAYER);

    int lineupIndex = -1;
    int i=0;
    foreach (Giocatore* p, m_players)
    {
        if (p->ruolo() == ruolo1 or p->ruolo() == ruolo2)
        {
            combo->addItem(p->nomeCompleto());
            i++;
        }
        else if (p->ruolo()/10 == (ruolo1/10) or p->ruolo()/10 == (ruolo2/10))
        {
            combo->addItem(p->nomeCompleto());
            combo->setItemData(i+1, QColor(Qt::red), Qt::TextColorRole);
            i++;
        }

        if (p->partita(0).Formazione() == comboId)
            lineupIndex = i-1;
    }

    if (lineupIndex > 0)
        combo->setCurrentIndex(lineupIndex);
}

void MainWindow::disableAllCombo()
{
    m_ui.comboBoxPortiere->setEnabled(false);
    m_ui.comboBoxDifensoreSx->setEnabled(false);
    m_ui.comboBoxDifensoreDx->setEnabled(false);
    m_ui.comboBoxDifensoreCx1->setEnabled(false);
    m_ui.comboBoxDifensoreCx2->setEnabled(false);
    m_ui.comboBoxDifensoreCx3->setEnabled(false);
    m_ui.comboBoxCentrocampistaCx1->setEnabled(false);
    m_ui.comboBoxCentrocampistaCx2->setEnabled(false);
    m_ui.comboBoxCentrocampistaCx3->setEnabled(false);
    m_ui.comboBoxCentrocampistaSx->setEnabled(false);
    m_ui.comboBoxCentrocampistaDx->setEnabled(false);
    m_ui.comboBoxAttaccanteCx1->setEnabled(false);
    m_ui.comboBoxAttaccanteCx2->setEnabled(false);
    m_ui.comboBoxAttaccanteCx3->setEnabled(false);
    m_ui.comboBoxTrequartista1->setEnabled(false);
    m_ui.comboBoxTrequartista2->setEnabled(false);
    m_ui.comboBoxTrequartista3->setEnabled(false);
    m_ui.comboBoxAttaccanteDx->setEnabled(false);
    m_ui.comboBoxAttaccanteSx->setEnabled(false);

    m_ui.comboBoxPortiere->setVisible(false);
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
    QPushButton* button = (QPushButton*)QObject::sender();
    if (button != NULL)
    {
        QListWidget* list;
        QComboBox* combo;
        if (button->objectName() == "buttonDifesa")
        {
            list  = m_ui.listPanchinariDifesa;
            combo = m_ui.comboBoxPanchinariDifesa;
        }
        else if (button->objectName() == "buttonCentrocampo")
        {
            list  = m_ui.listPanchinariCentrocampo;
            combo = m_ui.comboBoxPanchinariCentrocampo;
        }
        else if (button->objectName() == "buttonAttacco")
        {
            list  = m_ui.listPanchinariAttacco;
            combo = m_ui.comboBoxPanchinariAttacco;
        }

        QString player = combo->currentText();

        bool alreadyIn = false;
        for (int i=0; i<list->count(); ++i)
        {
            if (player == list->item(i)->text())
            {
                alreadyIn = true;
                break;
            }
        }

        if (!alreadyIn)
        {
            int row = list->currentRow();
            if (row == -1)
                row = list->count() + 1;
            list->insertItem(row+1, player);
        }
    }
}

void MainWindow::removePanchinaro(QListWidgetItem* item)
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
    foreach(QComboBox* combo, m_combos)
    {
        if (combo->isEnabled())
        {
            QString player = combo->currentText();
            RuoloEnum ruolo = Portiere;

            bool isMisplaced = true;
            foreach (Giocatore* p, m_players)
            {
                if (p->nomeCompleto() == player)
                {
                    ruolo = p->ruolo();
                    int comboId = combo->objectName().toInt();

                    foreach (int l, m_roleComboMap[comboId])
                    {
                        if (int(p->ruolo()) == l)
                            isMisplaced = false;
                    }
                }
            }

            if (player == capitano)
                text += "*";

            if (isMisplaced)
                text += "_" + player.toLower() + " [" + Giocatore::ruoloToString(ruolo) + "]_";
            else
                text += player + " [" + Giocatore::ruoloToString(ruolo) + "]";

            if (player == capitano)
                text += " (C)*\n";
            else
                text += "\n";
        }
    }

    text = text + "--------------\n";
    QListWidget* list;
    for (int nPanch=0; nPanch<3; nPanch++)
    {
        if (nPanch == 0)
            list = m_ui.listPanchinariDifesa;
        else if (nPanch == 1)
            list = m_ui.listPanchinariCentrocampo;
        else if (nPanch == 2)
            list = m_ui.listPanchinariAttacco;

        for (int i=0; i<list->count(); ++i)
        {
            QString player = list->item(i)->text();
            foreach (Giocatore* p, m_players)
            {
                if (player == p->nomeCompleto())
                {
                    text += player + " [" + Giocatore::ruoloToString(p->ruolo()) + "]\n";
                    break;
                }
            }
        }
    }

    return text;
}

void MainWindow::fileSave()
{
    QString qryStr;
    QSqlQuery query(*m_db);
    QSqlQuery query2(*m_db);

    for (int i=0; i<m_players.size(); ++i)
    {
        qryStr = "SELECT * FROM roster WHERE name=\""+m_players.at(i)->nome()+"\" and surname=\""+m_players.at(i)->cognome()+"\";";
        if (query.exec(qryStr))
        {
            if (!query.first())
            {
                qryStr = "INSERT INTO roster (name, surname, role) VALUES (\""+m_players.at(i)->nome()+"\",\""
                        + m_players.at(i)->cognome()+"\","+QString::number(int(m_players.at(i)->ruolo()))+");";

                query2.exec(qryStr);
            }
            else
            {
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

void MainWindow::teamSorter(QList<Giocatore*>& players)
{
    for(int i=0; i<players.size()-1; ++i)
    {
        for(int j=i+1; j<players.size(); ++j)
        {
            if ((players[i]->ruolo() > players[j]->ruolo()) ||
                    ((players[i]->nomeCompleto() > players[j]->nomeCompleto()) &&
                     (players[i]->ruolo() == players[j]->ruolo()))) {
                Giocatore* temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }
}

void MainWindow::setTeamLogo(QLabel* qLabel, int size, QByteArray imageName)
{
//    QImage myImage;
//    myImage.load(imageName);
//    myImage = myImage.scaled(size, size);
//    qLabel->setPixmap(QPixmap::fromImage(myImage));

    QPixmap pixmap;
    if (pixmap.loadFromData(imageName))
    {
        pixmap = pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    qLabel->setPixmap(pixmap);
}

void MainWindow::screenResolution(int& theWidth, int& theHeight)
{
    QScreen *screen = QApplication::screens().at(0);
    theWidth = screen->availableSize().width();
    theHeight = screen->availableSize().height();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QMainWindow::keyReleaseEvent(event);
    if (event->key() == Qt::Key_Back)
    {
        event->accept();

        if (m_ui.stackedWidget->currentIndex() == 0 or m_ui.stackedWidget->currentIndex() == 1)
            qApp->quit();
        else
            emit stackTo(0);
    }
    else if (event->key() == Qt::Key_Menu)
    {
        event->accept();
        if (m_ui.stackedWidget->currentIndex() == 0)
            emit stackTo(5);
    }
}

void MainWindow::computeRanking()
{
    QSqlQuery query(*m_db);
    QString qryStr;

    if (!m_rankingComputed)
    {
        QMap<QString, Team*> teams;
        // update flag when update has been downloaded

        qryStr = "SELECT name from teamName;";
        query.exec(qryStr);
        while (query.next())
        {
            QString name = query.value(0).toString();
            teams.insert(name, new Team(name));
        }

        qryStr = "SELECT * from fixture;";
        query.exec(qryStr);
        while (query.next())
        {
            if (query.value("played").toInt() == 1)
            {
                Team* team1 = teams[query.value("team1").toString()];
                Team* team2 = teams[query.value("team2").toString()];
                int score1 = query.value("goal1").toInt();
                int score2 = query.value("goal2").toInt();
                float points1 = query.value("points1").toFloat();
                float points2 = query.value("points2").toFloat();

                team1->m_played++;
                team2->m_played++;

                if (score1 > score2)
                {
                    team1->m_points += 2;
                    team1->m_won += 1;
                    team1->m_scored += score1;
                    team1->m_got += score2;
                    team1->m_fantaPoints += points1;

                    //team2.m_points += 2;
                    team2->m_lost += 1;
                    team2->m_scored += score2;
                    team2->m_got += score1;
                    team2->m_fantaPoints += points2;
                }
                else if (score1 < score2)
                {
                    //team1.m_points += 2;
                    team1->m_lost += 1;
                    team1->m_scored += score1;
                    team1->m_got += score2;
                    team1->m_fantaPoints += points1;

                    team2->m_points += 2;
                    team2->m_won += 1;
                    team2->m_scored += score2;
                    team2->m_got += score1;
                    team2->m_fantaPoints += points2;
                }
                else
                {
                    team1->m_points += 1;
                    team1->m_drawn += 1;
                    team1->m_scored += score1;
                    team1->m_got += score2;
                    team1->m_fantaPoints += points1;

                    team2->m_points += 1;
                    team2->m_drawn += 1;
                    team2->m_scored += score2;
                    team2->m_got += score1;
                    team2->m_fantaPoints += points2;
                }
            }
        }

        QList<QString> sortedTeams;
        foreach(QString key, teams.keys())
            sortedTeams.append(key);

        for (int i=0; i<sortedTeams.size()-1; i++)
        {
            for (int j=i+1; j<sortedTeams.size(); j++)
            {
                // todo improve sorting criteria
                if (teams[sortedTeams[i]]->m_points <= teams[sortedTeams[j]]->m_points)
                {
                    QString temp = sortedTeams[i];
                    sortedTeams[i] = sortedTeams[j];
                    sortedTeams[j] = temp;
                }
            }
        }

        m_ui.tableWidgetRanking->setRowCount(teams.size());
        m_ui.tableWidgetRanking->setColumnCount(10);
        QStringList headers;
        headers << "SQUADRA" << "P" << "G" << "V" << "N" << "P" << "GF" << "GS" << "FP" << "";
        m_ui.tableWidgetRanking->setHorizontalHeaderLabels(headers);
        m_ui.tableWidgetRanking->verticalHeader()->setVisible(false);
        //m_ui.tableWidgetRanking->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //m_ui.tableWidgetRanking->setSelectionBehavior(QAbstractItemView::NoSelection);
        //m_ui.tableWidgetRanking->setSelectionMode(QAbstractItemView::SingleSelection);
        //m_ui.tableWidgetRanking->setShowGrid(false);
        //m_ui.tableWidgetRanking->setStyleSheet("QTableView {selection-background-color: red;}");
        //m_ui.tableWidgetRanking->setGeometry(QApplication::desktop()->screenGeometry());

        for (int i=0; i<sortedTeams.size(); i++)
        {
            QString key = sortedTeams[i];
            m_ui.tableWidgetRanking->setItem(i, 0, new QTableWidgetItem(key));
            m_ui.tableWidgetRanking->setItem(i, 1, new QTableWidgetItem(QString::number(teams[key]->m_points)));
            m_ui.tableWidgetRanking->setItem(i, 2, new QTableWidgetItem(QString::number(teams[key]->m_played)));
            m_ui.tableWidgetRanking->setItem(i, 3, new QTableWidgetItem(QString::number(teams[key]->m_won)));
            m_ui.tableWidgetRanking->setItem(i, 4, new QTableWidgetItem(QString::number(teams[key]->m_drawn)));
            m_ui.tableWidgetRanking->setItem(i, 5, new QTableWidgetItem(QString::number(teams[key]->m_lost)));
            m_ui.tableWidgetRanking->setItem(i, 6, new QTableWidgetItem(QString::number(teams[key]->m_scored)));
            m_ui.tableWidgetRanking->setItem(i, 7, new QTableWidgetItem(QString::number(teams[key]->m_got)));
            m_ui.tableWidgetRanking->setItem(i, 8, new QTableWidgetItem(QString::number(teams[key]->m_fantaPoints)));
            m_ui.tableWidgetRanking->setItem(i, 9, new QTableWidgetItem(""));
        }

        //m_ui.tableWidgetRanking->setVisible(false);
        //QRect vporig = m_ui.tableWidgetRanking->viewport()->geometry();
        //QRect vpnew = vporig;
        //vpnew.setWidth(std::numeric_limits<int>::max());
        //m_ui.tableWidgetRanking->viewport()->setGeometry(vpnew);
        m_ui.tableWidgetRanking->resizeColumnsToContents();
        //m_ui.tableWidgetRanking->resizeRowsToContents();
        //m_ui.tableWidgetRanking->viewport()->setGeometry(vporig);
        m_ui.tableWidgetRanking->horizontalHeader()->setStretchLastSection(true);
        //m_ui.tableWidgetRanking->horizontalHeader()->setSectionResizeMode(QHeaderView::AdjustToContents);
        //m_ui.tableWidgetRanking->resizeColumnToContents(0);
        //m_ui.tableWidgetRanking->setVisible(true);
    }

    if (m_round == -1)
    {
        qryStr = "SELECT max(round) FROM fixture WHERE played = 1;";
        query.exec(qryStr);
        if (query.first())
        {
            m_round = query.value(0).toInt();
        }
        else
            m_round++;

        qryStr = "SELECT max(round) FROM fixture;";
        query.exec(qryStr);
        if (query.first())
        {
            m_maxRounds = query.value(0).toInt();
        }
        qDebug() << m_round << m_maxRounds;
    }

    qryStr = "SELECT * FROM fixture where round = " + QString::number(m_round);
    query.exec(qryStr);

    QString results = "<b>Giornata " + QString::number(m_round+1)+"</b><br><pre width=\"30\">";
    while(query.next())
    {
        results += QString("%1\t%2\t  %3 - %4<br>")
                .arg(query.value("team1").toString(), -18, QChar(' '))
                .arg(query.value("team2").toString(), -18, QChar(' '))
                .arg(query.value("goal1").toString())
                .arg(query.value("goal2").toString());
    }
    results += "</pre>";

    m_ui.labelResults->setText(results);
}
