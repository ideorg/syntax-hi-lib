#ifndef KSYNTAXHIGHLIGHTING_LIBPATHS_H
#define KSYNTAXHIGHLIGHTING_LIBPATHS_H

#include <QString>

namespace KSyntaxHighlighting {
struct LibPaths {
    static QString KDE_DATA() {return "org.kde.syntax-highlighting";}
};
}

#endif //KSYNTAXHIGHLIGHTING_LIBPATHS_H
