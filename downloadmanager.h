#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QString>
#include <QCryptographicHash>
#include <QByteArray>

#define HASH_ALGORITHM QCryptographicHash::Sha1

class QIODevice;

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    DownloadManager();

    void doDownload(const QUrl& url);
    QString saveFileName(const QUrl& url);
    bool saveToDisk(const QString& filename, QIODevice* data);
    bool isConnectedToNetwork();

public slots:
    void execute(const QString& filename, const QString& destinationPath);
    void downloadFinished(QNetworkReply* reply);

signals:
    void downloadDone(bool exitCode);
    void sameHash();
    void cannotOpenFile(QString filename);

private:
    QByteArray computeHash(const QString& filename);

    QString                m_destination;
    QNetworkAccessManager* m_manager;
    QList<QNetworkReply*>  m_currentDownloads;
};

#endif // DOWNLOADMANAGER_H
