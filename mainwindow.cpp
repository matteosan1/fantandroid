#include "mainwindow.h"

#include <QMessageBox>
//#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
//#include <QtGlobal>
#include <QPixmap>
//#include <QFileDialog>
#include <QScroller>
#include <QScreen>
#include <QKeyEvent>

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>
#endif

//#include <QDebug>

#include "ComboBoxDelegate.h"
#include "downloadmanager.h"
#include "punteggi.h"
#include "insertgiocatori.h"

#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_versionChecked(false)
{
    connect(this, SIGNAL(destroyed()), this, SLOT(close()));
    m_downloadManager = new DownloadManager();
    connect(m_downloadManager, SIGNAL(downloadDone(bool)), this, SLOT(configDownload(bool)));

    if (!QDir(OUTPUT_DIR).exists())
    {
        QDir().mkdir(OUTPUT_DIR);
    }

    m_version = checkVersion();
    m_downloadManager->execute("https://dl.dropboxusercontent.com/s/rb1pqlwb0v4h6jl/version.txt?dl=0", OUTPUT_DIR);

    m_ui.setupUi(this);
}

MainWindow::~MainWindow()
{
    m_db->close();
    delete m_db;
}

QList<Giocatore*> MainWindow::giocatori()
{
    GiocatoriInsert* temp = dynamic_cast<GiocatoriInsert*>(m_model);
    return temp->GetGiocatori();
}

void MainWindow::configDownload(bool exitCode)
{
    bool goOn = false;

    if (!exitCode)
    {
        if (!m_versionChecked)
        {
            m_versionChecked = true;
            goOn = true;
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
                m_downloadManager->execute("https://dl.dropboxusercontent.com/s/0p5psz455zjaxb1/team.sqlite?dl=0", OUTPUT_DIR);
            else
                goOn = true;
        }
        else
            goOn = true;
    }


    if (goOn)
    {
        m_db = new QSqlDatabase(QSqlDatabase::addDatabase( "QSQLITE","MyConnection" ));
        m_db->setDatabaseName(QString(OUTPUT_DIR)+ QString("team.sqlite"));
        m_db->open();
        chooseYourTeam();
    }
}

QString MainWindow::checkVersion()
{
    QString filename = QString(OUTPUT_DIR)+QString("version.txt");

    if (!QFile::exists(filename))
    {
        if (!m_downloadManager->isConnectedToNetwork())
        {
            int ret = QMessageBox::critical(this, "Error in reading version file", "Connettiti ad internet e riavvia fantandroid", QMessageBox::Ok);
            //qDebug() << ret << QMessageBox::Ok;
            if (ret == QMessageBox::Ok)
                this->close();
            qApp->quit();
        }

        return "";
    }

    QFile file(QString(OUTPUT_DIR)+QString("version.txt"));

    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error in reading version file", file.errorString(), QMessageBox::Cancel);
        return "0";
    }

    QTextStream in(&file);
    QString version = in.readLine();

    file.close();

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

//void MainWindow::loadDB()
//{
////    QString filename = QFileDialog::getOpenFileName(this);

////    if (filename != NULL)
////    {

////        QString destination = QString(OUTPUT_DIR)+filename.split("/").last();
////        if (QFile::exists(destination))
////            QFile::remove(destination);

////        QFile myFile;
////        myFile.setFileName(filename);
////        myFile.copy(destination);
//        m_db->open();
//        chooseYourTeam();
////    }
//}

void MainWindow::chooseYourTeam(bool overwrite)
{
    QString qryStr;
    QSqlQuery query(*m_db);

    qryStr = "SELECT * from yourTeam";
    query.exec(qryStr);
    query.first();

    if (query.first() and !overwrite)
    {
        m_teamLabel = query.value("name").toString();
        init();
    }
    else
    {
        m_ui.comboBoxSelectTeam->clear();
        qryStr = QString("SELECT * from teamName;");
        query.exec(qryStr);
        while (query.next())
        {
            m_ui.comboBoxSelectTeam->addItem(query.value("name").toString());
        }

        connect(m_ui.pushButtonSelectTeam, SIGNAL(clicked(bool)), this, SLOT(selectTeam()));
        m_ui.stackedWidget->setCurrentIndex(1);
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
    query.next();
    m_teamImage = query.value("image").toString();
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
    connect(m_ui.pushButtonAggiungii, SIGNAL(clicked(bool)), this, SLOT(aggiungiPushed()));
    connect(m_ui.listPanchinari, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removePanchinaro(QListWidgetItem*)));

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
    m_combos.append(m_ui.comboBoxAttaccanteDx);
    m_combos.append(m_ui.comboBoxAttaccanteSx);
    m_combos.append(m_ui.comboBoxAttaccanteCx);

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
    QString imageName = query.value("image").toString();
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
}

//void MainWindow::insertRow()
//{
//    m_ui.tableViewPlayers->model()->insertRows(0, 1);
//    QModelIndex index = m_ui.tableViewPlayers->currentIndex();
//    int row = index.row();
//    int column = 0;
//    QModelIndex newIndex = m_ui.tableViewPlayers->model()->index(row,column);
//    m_ui.tableViewPlayers->setCurrentIndex(newIndex);
//    m_ui.tableViewPlayers->setFocus();
//    m_ui.tableViewPlayers->edit(newIndex);
//    m_isModified = true;
//}

//void MainWindow::removeRow()
//{
//    QModelIndex index = m_ui.tableViewPlayers->currentIndex();
//    QString name = index.sibling(index.row(), 1).data().toString();
//    QString surname = index.sibling(index.row(), 0).data().toString();
//    int role = index.sibling(index.row(), 2).data().toInt();

//    QString qryStr = "DELETE FROM roster WHERE name=\"" + name + "\" and surname = \"" + surname + "\" and role = " + QString::number(role) + ";";
//    QSqlQuery query(*m_db);


//    query.exec(qryStr);

//    int row = m_ui.tableViewPlayers->currentIndex().row();
//    m_ui.tableViewPlayers->model()->removeRows(row, 1);
//    m_isModified = true;
//}

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
            sendLineup();
            emit stackTo(0);
        }
        else if (sender->objectName() == "LineupCancel")
        {
            QString title = "Formazione";
            QMessageBox::warning(this, title, "Formazione non terminata");
            emit stackTo(0);
        }
        else if (sender->objectName() == "RosterExit")
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

void MainWindow::updateTeam()
{
    if (m_isModified) {
        m_players = giocatori();

        int ret = QMessageBox::warning(this, "Fantandroid Avvertimento",
                                       tr("Vuoi salvare i cambiamenti ?"),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No | QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
        {
            //m_db->commit();
            fileSave();
        }
        //else
            //m_db->rollback();
    }

    for(int i=0; i<m_players.size(); ++i) {
        if (m_players[i]->cognome() == "")
            m_players[i]->setCognome(QString("GIOCATORE %d").arg(i));
        if (m_players[i]->nome() == "")
            m_players[i]->setNome(" ");
        if (m_players[i]->squadra() == "")
            m_players[i]->setSquadra(" ");
    }

    teamSorter(m_players);
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
    //if (player == EMPTY_PLAYER)
    //    return;

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
        if (index != -1)
            nGiocano++;
    }

    if (nGiocano == 11)
    {
        // todo check nGiocano, capitano settato e panchina fatta
        m_ui.pushButtonLineupOK->setEnabled(true);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinari->clear();
        foreach (Giocatore* p, m_players)
        {
            if (p->partita(0).Formazione() != -1)
                m_ui.comboBoxCapitano->addItem(p->nomeCompleto());
            else
                m_ui.comboBoxPanchinari->addItem(p->nomeCompleto());
        }
    }
    else
    {
        m_ui.pushButtonLineupOK->setEnabled(false);
        m_ui.comboBoxCapitano->clear();
        m_ui.comboBoxPanchinari->clear();
    }
}

void MainWindow::updateModules(const int& chosenModule)
{
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
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
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
        enableCombo(m_ui.comboBoxAttaccanteCx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
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
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
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
        enableCombo(m_ui.comboBoxAttaccanteDx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteSx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        break;
    case 6: // 4-3-1-2
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteCentrale);
        break;
    case 7: // 4-3-3
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
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
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaDx, CentrocampistaEsterno, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxCentrocampistaSx, CentrocampistaEsterno, CentrocampistaTrequartista);
        break;
    case 9: // 4-2-1-3
        enableCombo(m_ui.comboBoxDifensoreCx1, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreCx2, DifensoreCentrale);
        enableCombo(m_ui.comboBoxDifensoreDx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxDifensoreSx, DifensoreTerzino);
        enableCombo(m_ui.comboBoxCentrocampistaCx1, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx2, CentrocampistaCentrale);
        enableCombo(m_ui.comboBoxCentrocampistaCx3, CentrocampistaTrequartista);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
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
        enableCombo(m_ui.comboBoxCentrocampistaCx3, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
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
        enableCombo(m_ui.comboBoxAttaccanteCx, CentrocampistaTrequartista);
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
        enableCombo(m_ui.comboBoxAttaccanteCx, AttaccanteCentrale);
        enableCombo(m_ui.comboBoxAttaccanteSx, AttaccanteMovimento);
        enableCombo(m_ui.comboBoxAttaccanteDx, AttaccanteMovimento);
        break;
    }
}

void MainWindow::enableCombo(QComboBox* combo, RuoloEnum ruolo1, RuoloEnum ruolo2)
{
    combo->setEnabled(true);
    combo->setVisible(true);

    combo->clear();
    combo->addItem(EMPTY_PLAYER);

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
    }
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
    m_ui.comboBoxAttaccanteCx->setEnabled(false);
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
    m_ui.comboBoxAttaccanteCx->setVisible(false);
    m_ui.comboBoxAttaccanteDx->setVisible(false);
    m_ui.comboBoxAttaccanteSx->setVisible(false);
}

void MainWindow::aggiungiPushed()
{
    QString player = m_ui.comboBoxPanchinari->currentText();

    bool alreadyIn = false;
    for (int i=0; i<m_ui.listPanchinari->count(); ++i)
    {
        if (player == m_ui.listPanchinari->item(i)->text())
        {
            alreadyIn = true;
            break;
        }
    }

    if (!alreadyIn)
    {
        int row = m_ui.listPanchinari->currentRow();
        if (row == -1)
            row = m_ui.listPanchinari->count() + 1;
        m_ui.listPanchinari->insertItem(row+1, player);
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

    text = text + "Modulo: " + m_ui.comboBoxModulo->currentText() + "\n";
    foreach(QComboBox* combo, m_combos)
    {
        if (combo->isEnabled())
        {
            QString player = combo->currentText();
            RuoloEnum ruolo;

            foreach (Giocatore* p, m_players)
            {
                if (p->nomeCompleto() == player)
                {
                    ruolo = p->ruolo();
                    break;
                }
            }

            if (player == capitano)
                text = text + player + " [" + Giocatore::ruoloToString(ruolo) + "] (C)\n";
            else
                text = text + player + " [" + Giocatore::ruoloToString(ruolo) + "]\n";
        }
    }

    text = text + "\nPANCHINA\n";
    for (int i=0; i<m_ui.listPanchinari->count(); ++i)
    {
        text = text + m_ui.listPanchinari->item(i)->text() + "\n";
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

void MainWindow::setTeamLogo(QLabel* qLabel, int size, QString imageName)
{
    QImage myImage;
    myImage.load(imageName);
    myImage = myImage.scaled(size, size);
    qLabel->setPixmap(QPixmap::fromImage(myImage));
}

//void MainWindow::chooseImage()
//{

//#ifdef Q_OS_ANDROID
//    m_imagePicker.show();
//#endif
//}

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
        emit stackTo(0);
    }
    else if (event->key() == Qt::Key_Menu)
    {
        event->accept();
        if (m_ui.stackedWidget->currentIndex() == 0)
            emit stackTo(5);
    }
}
