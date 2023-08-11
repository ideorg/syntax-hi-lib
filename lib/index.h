#ifndef DOWNLOADER_INDEX_H
#define DOWNLOADER_INDEX_H

#include <QString>
#include <QXmlStreamAttributes>

namespace KSyntaxHighlighting {
struct UpdateInfo {
    QString fileName;
    QString langName;
    QString url;
    int version = 0;

    void fill(QXmlStreamAttributes &attributes);
};

struct LanguageInfo {
    QString fileName;
    QString langName;
    QString section;
    int version = 0;
    QString extensions;

    void fill(QXmlStreamAttributes &attributes);
};

using InfoList = QList<UpdateInfo>;

class Index {
    QString syntaxDir;
public:
    Index(QString syntaxDir);

    LanguageInfo readFromFile(QString fileName);

    static InfoList readUpdateData(QByteArray &xmlData);
};
}

#endif //DOWNLOADER_INDEX_H
