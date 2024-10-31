#ifndef DOWNLOADER_DOWNLOADER_H
#define DOWNLOADER_DOWNLOADER_H

#include <QNetworkReply>
#include <QQueue>
#include <QDir>
#include "index.h"

namespace KSyntaxHighlighting {
class DownloaderUrl : public QObject {
Q_OBJECT
    QNetworkAccessManager manager;

    void finished(QNetworkReply *reply);

    bool m_busy = false;
public:
    QByteArray result;
    QNetworkReply::NetworkError error;

    bool start(QString fileUrl);

    bool busy() { return m_busy; }

signals:

    void done();
};

class DownloaderUrls : public QObject {
Q_OBJECT
    QNetworkAccessManager manager;

    void finished(QNetworkReply *reply);

    const int maxSimultaneousDownloads = 6;
    int currentDownloads = 0;
    QQueue<QNetworkRequest> downloadQueue;

    void addPortion();

    QString targetDir;
public:
    void setTargetDir(QString targetDir);

    bool start(QStringList fileUrls);

    bool busy() { return currentDownloads || !downloadQueue.empty(); }

signals:

    void done();
};


class AbstractDownloader : public QObject {
Q_OBJECT
protected:
    QString singlePath;
    QString multiPath;
    QString ext;
    QString singleFileUrl;
    DownloaderUrl singleLoader;
    DownloaderUrls multiLoader;
    QSet<QString> fileSet;

    void readDir();

    bool startLoadSingle();

public:
    bool busy() { return singleLoader.busy() || multiLoader.busy(); }

    void setPath(QString singlePath, QString multiPath);

    bool mustDownload();

    bool start();

    void finished();

signals:

    void done();
};

class SyntaxDownloader : public AbstractDownloader {
    void finishedUpdate();

    std::pair<InfoList, InfoList> divideToTwoSets(InfoList &infoList);

    InfoList filterNewVersions(InfoList &infoList);

    QStringList getUrls(InfoList &infoList);

public:
    SyntaxDownloader();
};

class ThemesDownloader : public AbstractDownloader {
    void finishedQrc();

    QStringList readQrcData(QByteArray &xmlData);

    static QStringList addPath(QStringList list, QString path);

    static QStringList filter(QStringList fileList, QSet<QString> &set);

public:
    ThemesDownloader();
};

class DoubleDownloader : public QObject {
Q_OBJECT
    QVector<AbstractDownloader *> downloaders;
    ThemesDownloader themesDownloader;
    SyntaxDownloader syntaxDownloader;
    QString path;
public:
    DoubleDownloader();

    void setPath(QString path);

    bool mustDownload();

    bool start();

    void finishedFirst();

    void finishedLast();

    bool busy();

signals:

    void done();
};
}

#endif //DOWNLOADER_DOWNLOADER_H
