#include "index.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

using namespace KSyntaxHighlighting;

Index::Index(QString syntaxDir):syntaxDir(syntaxDir) {
}

LanguageInfo Index::readFromFile(QString fileName) {
    QString filePath = syntaxDir + "/" + fileName;
    LanguageInfo info;
    QFile file(filePath);
    bool found = false;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader xmlReader(&file);
        while (!xmlReader.atEnd() && !xmlReader.hasError()) {
            QXmlStreamReader::TokenType token = xmlReader.readNext();
            if (token!=QXmlStreamReader::StartElement)
                continue;
            QStringView elementName = xmlReader.name();
            if (elementName.toString()=="language") {
                found = true;
                break;
            }
        }
        if (found) {
            QXmlStreamAttributes attributes = xmlReader.attributes();
            info.fill(attributes);
            info.fileName = fileName;
        }
        file.close();
    }
    return info;
}

QList<UpdateInfo> Index::readUpdateData(QByteArray &xmlData) {
    QList<UpdateInfo> list;
    bool found = false;
    QXmlStreamReader xmlReader(xmlData);
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();
        if (token!=QXmlStreamReader::StartElement)
            continue;
        QStringView elementName = xmlReader.name();
        if (elementName.toString()=="Definition") {
            QXmlStreamAttributes attributes = xmlReader.attributes();
            UpdateInfo info;
            info.fill(attributes);
            list.append(info);
        }
    }
    return list;
}

void LanguageInfo::fill(QXmlStreamAttributes &attributes) {
    foreach (const QXmlStreamAttribute &attr, attributes) {
        QString attrName = attr.name().toString();
        QString value = attr.value().toString();
        if (attrName=="name")
            langName = value;
        else if (attrName=="section")
            section = value;
        else if (attrName=="version")
            version = attr.value().toInt();
        else if (attrName=="extensions")
            extensions = value;
        }
}

void LanguageInfo::save(QFile &file) {
    QString versionStr = QString::number(version);
    QString row = fileName + "|" + langName + "|" + section  + "|" + versionStr + "|" + extensions + "\n";
    std::string str = row.toUtf8().toStdString();
    file.write(str.c_str(),str.length());
}


void UpdateInfo::fill(QXmlStreamAttributes &attributes) {
    foreach (const QXmlStreamAttribute &attr, attributes) {
            QString attrName = attr.name().toString();
            QString value = attr.value().toString();
            if (attrName=="name")
                langName = value;
            else if (attrName=="version")
                version = attr.value().toInt();
            else if (attrName=="url") {
                url = value;
                qsizetype pos = url.lastIndexOf('/');
                fileName = url.mid(pos+1);
            }
        }
}
