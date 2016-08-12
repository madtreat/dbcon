#ifndef DBTABLE_H
#define DBTABLE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>

#include "dbfield.h"


class QSettings;


class DBTable : public QObject {
  Q_OBJECT;

public:
  explicit DBTable(QString _name, QSettings* _settings, QObject* _parent=0);
  explicit DBTable(const DBTable& orig) = delete;
  ~DBTable();

public slots:
  // void readFromSettings();
  // void saveSettings();

private:
  QSettings* settings;

  QString     name;
  DBFieldList fields;
  int         numFields;
  int         versionMajor;
  int         versionMinor;

  DBField* readField();
};

typedef QList<DBTable*> DBTableList;

#endif  // DBTABLE_H
