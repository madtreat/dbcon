#ifndef DBCON_H
#define DBCON_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>


class DBSettings;


// A Database connection instance
class DBCon : public QObject {
  Q_OBJECT;

public:
  explicit DBCon(DBSettings* _settings, QString _dbName, QObject* _parent=0);
  explicit DBCon(const DBCon& orig) = delete;
  ~DBCon();

  bool isValid()      const {return db.isValid();}
  bool isOpen()       const {return db.isOpen();}
  bool isOpenError()  const {return db.isOpenError();}
  QString lastError() const {return db.lastError().text();}

  static bool isValidDatabaseDriver(QString type);
  QSqlDatabase connectToDB(QString connectionName);
  QSqlDatabase getDatabaseObject() const {return db;}

public slots:
  QSqlQuery execQuery(QString query);

private:
  DBSettings*   settings;
  QString       dbName;
  QSqlDatabase  db;
};

#endif  // DBCON_H
