#include "downloader.h"
#include "index.h"
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <cassert>
#include <QXmlStreamReader>

using namespace KSyntaxHighlighting;

void AbstractDownloader::setPath(QString singlePath, QString multiPath) {
    this->singlePath = singlePath;
    this->multiPath = multiPath;
    multiLoader.setTargetDir(multiPath);
}

void AbstractDownloader::readDir() {
    QStringList syntaxFiles = QDir(multiPath).entryList(QStringList() << ext, QDir::Files);
    fileSet = QSet<QString>(syntaxFiles.constBegin(), syntaxFiles.constEnd());
}

bool AbstractDownloader::startLoadSingle() {
    if (busy())
        return false;
    singleLoader.start(singleFileUrl);
    return true;
}

bool AbstractDownloader::start() {
    auto dir = QDir(multiPath);
    if (!dir.exists())
        dir.mkpath(".");
    readDir();
    if (busy())
        return false;
    startLoadSingle();
    return true;
}

void AbstractDownloader::finished() {
    Q_EMIT done();
}

SyntaxDownloader::SyntaxDownloader() {
    ext = "*.xml";
    singleFileUrl = "https://kate-editor.org/syntax/update-5.256.xml";
    connect(&singleLoader, &DownloaderUrl::done, this, &SyntaxDownloader::finishedUpdate);
    connect(&multiLoader, &DownloaderUrls::done, this, &AbstractDownloader::finished);
}

bool SyntaxDownloader::mustDownload() {
    if (!QDir(multiPath).exists()) return true;
    return true;
}

ThemesDownloader::ThemesDownloader() {
    ext = "*.theme";
    singleFileUrl = "https://invent.kde.org/frameworks/syntax-highlighting/-/raw/master/data/themes/theme-data.qrc";
    connect(&singleLoader, &DownloaderUrl::done, this, &ThemesDownloader::finishedQrc);
    connect(&multiLoader, &DownloaderUrls::done, this, &AbstractDownloader::finished);
}

bool ThemesDownloader::mustDownload() {
    if (!QDir(multiPath).exists()) return true;
    return true;
}

bool DownloaderUrl::start(QString fileUrl) {
    if (m_busy) return false;
    m_busy = true;
    QUrl url(fileUrl);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        finished(reply);
        reply->deleteLater();
    });
    return true;
}

void DownloaderUrl::finished(QNetworkReply *reply) {
    error = reply->error();
    if (error == QNetworkReply::NoError) {
        result = reply->readAll();
    } else {
    }
    m_busy = false;
    Q_EMIT done(reply->url());
}

bool DownloaderUrls::start(QStringList fileUrls) {
    if (busy())
        return false;
    QSet<QString> urlSet = QSet<QString>(fileUrls.constBegin(), fileUrls.constEnd());
    fileUrls = QStringList(urlSet.constBegin(), urlSet.constEnd());
    if (fileUrls.empty())
        Q_EMIT done();
    else {
        for (const auto &fileUrl: fileUrls) {
            QNetworkRequest request(fileUrl);
            downloadQueue.enqueue(request);
        }
        addPortion();
    }
    return true;
}

void DownloaderUrls::addPortion() {
    if (downloadQueue.isEmpty())
        assert(!downloadQueue.isEmpty());
    while (currentDownloads < maxSimultaneousDownloads && !downloadQueue.isEmpty())
    {
        QNetworkRequest request = downloadQueue.dequeue();
        QNetworkReply *reply = manager.get(request);
        currentDownloads++;
        QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
            finished(reply);
            reply->deleteLater();
        });
    }
}

void DownloaderUrls::finished(QNetworkReply *reply) {
    auto url = reply->url();
    QByteArray data = reply->readAll();
    QString fname = url.fileName();
    QFile file(targetDir+"/" + fname);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
    int dotIndex = fname.lastIndexOf('.');
    QString ext;
    if (dotIndex != -1)
        ext = fname.right(dotIndex);
    else
        ext = "";
    if (ext == ".xml") {
      Index index(targetDir);
      auto langInfo = index.readFromFile(fname);
      QFile fileDat(targetDir+"/versions.dat");
      if (fileDat.open(QIODevice::Append)) {
        langInfo.save(fileDat);
        fileDat.close();
      }
    }
    currentDownloads--;
    if (downloadQueue.empty()) {
        if (!currentDownloads) {
            Q_EMIT done();
        }
    } else
        addPortion();
}

void DownloaderUrls::setTargetDir(QString targetDir) {
    this->targetDir = targetDir;
}

std::pair<InfoList,InfoList> SyntaxDownloader::divideToTwoSets(InfoList &infoList) {
    InfoList nonExists;
    InfoList toConsider;

    foreach (const UpdateInfo& info, infoList)
        if (fileSet.contains(info.fileName))
            toConsider.append(info);
        else
            nonExists.append(info);

    return std::make_pair(nonExists, toConsider);
}

InfoList SyntaxDownloader::filterNewVersions(InfoList &infoList) {
    InfoList filtered;
    Index index(multiPath);
    foreach (const UpdateInfo& info, infoList) {
        auto langInfo = index.readFromFile(info.fileName);
        if (info.version>langInfo.version)
            filtered.append(info);
    }
    return filtered;
}

QStringList SyntaxDownloader::getUrls(InfoList &infoList) {
    QStringList fileUrls;
    foreach (const UpdateInfo& info, infoList)
            fileUrls.append(info.url);
    return fileUrls;
}

void SyntaxDownloader::finishedUpdate(const QUrl& url) {
    QFile file(singlePath+"/"+url.fileName());
    if (file.open(QIODevice::WriteOnly)) {
      file.write(singleLoader.result);
      file.close();
    }
    auto infoList = Index::readUpdateData(singleLoader.result);
    auto p = divideToTwoSets(infoList);
    auto newVersions = filterNewVersions(p.second);
    infoList = p.first + newVersions;
    QStringList urlList = getUrls(infoList);
    multiLoader.start(urlList);
}


QStringList ThemesDownloader::addPath(QStringList list, QString path) {
    QStringList result;
    foreach (QString item, list)
        result.append(path+item);
    return result;
}

void ThemesDownloader::finishedQrc(const QUrl& url) {
    QFile file(singlePath+"/"+url.fileName());
    if (file.open(QIODevice::WriteOnly)) {
      file.write(singleLoader.result);
      file.close();
    }
    auto urlList = readQrcData(singleLoader.result);
    urlList = filter(urlList, fileSet);
    urlList = addPath(urlList, "https://invent.kde.org/frameworks/syntax-highlighting/-/raw/master/data/themes/");
    multiLoader.start(urlList);
}

QStringList ThemesDownloader::readQrcData(QByteArray& xmlData) {
    QStringList result;
    QXmlStreamReader inputStream(xmlData);
    while (!inputStream.atEnd() && !inputStream.hasError()) {
        inputStream.readNext();
        if (inputStream.isStartElement()) {
            QString name = inputStream.name().toString();
            if (name == "file")
                result.append(inputStream.readElementText());
        }
    }
    return result;
}

QStringList ThemesDownloader::filter(QStringList fileList, QSet<QString> &set) {
    QStringList result;
    foreach (QString item, fileList)
    if (!set.contains(item))
        result.append(item);
    return result;
}

DoubleDownloader::DoubleDownloader() {
    downloaders.push_back(&themesDownloader);
    downloaders.push_back(&syntaxDownloader);
    connect(downloaders[0], &AbstractDownloader::done, this, &DoubleDownloader::finishedFirst);
    connect(downloaders.back(), &AbstractDownloader::done, this, &DoubleDownloader::finishedLast);
}

void DoubleDownloader::setPath(QString path) {
    this->path = path;
    themesDownloader.setPath(path, path+"/themes");
    syntaxDownloader.setPath(path, path+"/syntax");
}

bool DoubleDownloader::mustDownload() {
    foreach (auto downloader, downloaders)
        if (downloader->mustDownload())
            return true;
    return false;
}

bool DoubleDownloader::start() {
    auto dir = QDir(path);
    if (!dir.exists())
        dir.mkpath(".");
    if (busy())
        return false;
    downloaders[0]->start();
    return true;
}

bool DoubleDownloader::busy() {
    foreach (auto downloader, downloaders)
        if (downloader->busy())
            return true;
    return false;
}

void DoubleDownloader::finishedFirst() {
    downloaders.back()->start();
}

void DoubleDownloader::finishedLast() {
    Q_EMIT done();
}

