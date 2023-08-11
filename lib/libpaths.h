#ifndef KSYNTAXHIGHLIGHTING_LIBPATHS_H
#define KSYNTAXHIGHLIGHTING_LIBPATHS_H

#include <QString>
#include <QStandardPaths>

namespace KSyntaxHighlighting {
struct LibPaths {
    static QString KDE_DATA() {return "org.kde.syntax-highlighting";}
    static QString data() { return generic() + QStringLiteral("/") + KDE_DATA();}
    static QString generic() { return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);}
    static QString syntax() { return data() + "/syntax";}
    static QString syntax_res() { return QStringLiteral(":/") + LibPaths::KDE_DATA() +"/syntax";}
    static QString syntax_res_add() { return QStringLiteral(":/") + LibPaths::KDE_DATA() + "/syntax-addons";}
    static QString themes() { return data() + "/themes";}
    static QString themes_res() { return QStringLiteral(":/") + LibPaths::KDE_DATA() +"/themes";}
    static QString themes_res_add() { return QStringLiteral(":/") + LibPaths::KDE_DATA() + "/themes-addons";}
};
}

#endif //KSYNTAXHIGHLIGHTING_LIBPATHS_H
