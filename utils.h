/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef UTILS_H
#define UTILS_H

//#include <QtCore>

#include <QString>
#include <QLocale>

bool file_exists (const QString &fileName);
QString change_file_ext (const QString &s, const QString &ext);
QString qstring_load (const QString &fileName, const char *enc);

inline QString locale_name()
{
  return QLocale::system().name().left(2).toLower();
}

#endif // UTILS_H
