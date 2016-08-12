#ifndef DBFIELD_H
#define DBFIELD_H

#include <QString>
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QMap>


enum DBFieldType {
  FT_INT = 0,
  FT_DOUBLE,
  FT_BOOL,
  FT_DATE,
  FT_TIME,
  // FT_TIMESTAMP,
  FT_VARCHAR,
  FT_TEXT,
  FT_CHOICE  // text, but limited to certain options
};

typedef QMap<QString, QString> Choices;

struct DBField {
  QString     name;
  DBFieldType type;
  int         maxSize;
  bool        allowNull;
  QString     key;  // PRI if a private key in MySQL
  QVariant    defaultValue;  // NULL or valid value in MySQL
  Choices     choices;

  void parseChoices(QString raw) {
    if (raw == "") {
      return;
    }
    QStringList list = raw.split(", ");
    foreach(QString choice, list) {
      QStringList parts = choice.split(" :: ");
      choices.insert(parts.at(0), parts.at(1));
    }
  }
};

typedef QList<DBField*> DBFieldList;

#endif  // DBFIELD_H
