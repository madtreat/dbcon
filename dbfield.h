#ifndef DBFIELD_H
#define DBFIELD_H

#include <QString>
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QMetaEnum>


typedef QMap<QString, QString> Choices;

class DBField : public QObject {
  Q_OBJECT;

public:
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
  Q_ENUM(DBFieldType);

  static QString     fieldTypeToString(DBFieldType type);
  static DBFieldType fieldTypeFromString(QString type);

  QString     name;
  DBFieldType type;
  int         maxSize;
  bool        allowNull;
  QString     key;  // PRI if a private key in MySQL
  QVariant    defaultValue;  // NULL or valid value in MySQL
  Choices     choices;

  void parseChoices(QString raw);
};

typedef QList<DBField*> DBFieldList;

#endif  // DBFIELD_H
