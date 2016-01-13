/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#include <QFile>
#include <QTextStream>


#include "utils.h"



bool file_exists (const QString &fileName)
{
  if (fileName.isNull() || fileName.isEmpty())
     return false;

  return QFile::exists (fileName);
}


QString change_file_ext (const QString &s, const QString &ext)
{
  int i = s.lastIndexOf (".");
  if (i == -1)
      return s;

  QString r (s);
  r.truncate (++i);
  r.append (ext);
  return r;
}


QString qstring_load (const QString &fileName, const char *enc)
{
  QFile file (fileName);

  if (! file.open (QFile::ReadOnly | QFile::Text))
     return QString();

  QTextStream in(&file);
  in.setCodec (enc);

  return in.readAll();
}
