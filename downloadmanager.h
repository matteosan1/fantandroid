#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QString>

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

private:
    QString                m_destination;
    QNetworkAccessManager* m_manager;
    QList<QNetworkReply*>  m_currentDownloads;
};

#endif // DOWNLOADMANAGER_H
