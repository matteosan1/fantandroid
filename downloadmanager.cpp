#include "downloadmanager.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>

//#include <QDebug>

DownloadManager::DownloadManager()
{
    m_manager = new QNetworkAccessManager();
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadManager::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply* reply = m_manager->get(request);

    m_currentDownloads.append(reply);
}

QString DownloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

//    if (QFile::exists(basename))
//    {
//        int i=0;
//        basename += '.';
//        while (QFile::exists(basename + QString::number(i)))
//            ++i;

//        basename += QString::number(i);
//    }

    return basename;
}

bool DownloadManager::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(m_destination + filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        //qDebug() << "Couldn't open " << filename << " for writing " << file.errorString();
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

void DownloadManager::execute(const QString& filename, const QString& destinationPath)
{
    m_destination = destinationPath;
    QUrl url = QUrl::fromEncoded(filename.toLocal8Bit());
    doDownload(url);
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error())
    {
        //qDebug() << "Download of " << url.toEncoded().constData() << " failed: " << reply->errorString();
        emit downloadDone(false);
    }
    else
    {
        QString filename = saveFileName(url);
        if (saveToDisk(filename, reply))
        {
            //qDebug() << "Download of " << url.toEncoded().constData() << " succeded (saved to " << filename << ")";
            emit downloadDone(true);
        }
    }

    m_currentDownloads.removeAll(reply);
    reply->deleteLater();

    if (m_currentDownloads.isEmpty())
    {
        this->deleteLater();
    }
}

bool DownloadManager::isConnectedToNetwork()
{

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for (int i = 0; i < ifaces.count(); i++) {

        QNetworkInterface iface = ifaces.at(i);
        if ( iface.flags().testFlag(QNetworkInterface::IsUp)
             && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

//            // details of connection
//            qDebug() << "name:" << iface.name() << endl
//                     << "ip addresses:" << endl
//                     << "mac:" << iface.hardwareAddress() << endl;

            for (int j=0; j<iface.addressEntries().count(); j++) {
//                qDebug() << iface.addressEntries().at(j).ip().toString()
//                         << " / " << iface.addressEntries().at(j).netmask().toString() << endl;

                // got an interface which is up, and has an ip address
                if (result == false)
                    result = true;
            }
        }

    }

    return result;
}
