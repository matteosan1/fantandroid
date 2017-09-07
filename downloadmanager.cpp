#include "downloadmanager.h"

#include <QMainWindow>

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QDir>

//#include <QDebug>

DownloadManager::DownloadManager()
{
    m_manager = new QNetworkAccessManager();
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

void DownloadManager::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = m_manager->get(request);

    m_currentDownloads.append(reply);
}

QString DownloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty()) {
        basename = "download";
    }

    return basename;
}

bool DownloadManager::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(m_destination + filename);

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    else {

        file.write(data->readAll());
        file.close();

        return true;
    }
}

void DownloadManager::execute(const QString &filename, const QString &destinationPath)
{
    m_destination = destinationPath;
    QUrl url = QUrl::fromEncoded(filename.toLocal8Bit());
    doDownload(url);
}

QByteArray DownloadManager::computeHash(const QString &filename)
{
    QFile file(filename);

    if (file.open(QFile::ReadOnly)) {
        QCryptographicHash hash(HASH_ALGORITHM);
        QByteArray data = file.readAll();
        hash.addData(data);

        return hash.result();
    }
    else {
        //emit cannotOpenFile(filename);
    }

    return QByteArray();
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{

    QUrl url = reply->url();

    if (reply->error()) {
        emit downloadDone(false);
    }
    else
    {
        QString filename = saveFileName(url);

        saveToDisk("/temp.sqlite", reply);
        QByteArray localHash = computeHash(m_destination + "/" + filename);
        QByteArray remoteHash = computeHash(m_destination + "/temp.sqlite");

        if (remoteHash != localHash) {
            QFile dfile(m_destination + "/temp.sqlite");
            QString filePath = m_destination + "/team.sqlite";
            if (dfile.exists()) {
                if (QFile::exists(filePath)) {
                    QFile::remove(filePath);
                }
                if (dfile.copy(filePath)) {
                    QFile::setPermissions(filePath, QFile::WriteOwner | QFile::ReadOwner);
                    emit downloadDone(true);
                }
            }
            else {
            }
        }
        else {
            // emit sameHash();
        }
    }

    m_currentDownloads.removeAll(reply);
    reply->deleteLater();

    if (m_currentDownloads.isEmpty()) {
        this->deleteLater();
    }
}

bool DownloadManager::isConnectedToNetwork()
{

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for (int i = 0; i < ifaces.count(); i++) {

        QNetworkInterface iface = ifaces.at(i);

        if (iface.flags().testFlag(QNetworkInterface::IsUp)
                && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

//            // details of connection
//            qDebug() << "name:" << iface.name() << endl
//                     << "ip addresses:" << endl
//                     << "mac:" << iface.hardwareAddress() << endl;

            for (int j=0; j<iface.addressEntries().count(); j++) {
//                qDebug() << iface.addressEntries().at(j).ip().toString()
//                         << " / " << iface.addressEntries().at(j).netmask().toString() << endl;

                // got an interface which is up, and has an ip address
                if (result == false) {
                    result = true;
                }
            }
        }

    }

    return result;
}
