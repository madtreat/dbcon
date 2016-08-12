#ifndef DBFIELD_H
#define DBFIELD_H

#include <QString>
#include <QVariant>
#include <QList>


enum DBFieldType {
  FT_INT = 0,
  FT_DOUBLE,
  FT_BOOL,
  FT_DATE,
  FT_TIME,
  // FT_TIMESTAMP,
  FT_VARCHAR,
  FT_TEXT,
  NUM_FIELD_TYPES
};

struct DBField {
  QString     name;
  DBFieldType type;
  int         maxSize;
  // bool        allowNull;
  // bool        key;  // PRI if a private key in MySQL
  QVariant    defaultValue;  // NULL or valid value in MySQL
};

typedef QList<DBField*> DBFieldList;

#endif  // DBFIELD_H
