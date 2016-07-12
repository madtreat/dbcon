#ifndef DBCON_H
#define DBCON_H

#include <QObject>
#include <QSqlDatabase>


class DBSettings;


// A Database connection instance
class DBCon : public QObject {
  Q_OBJECT;

public:
  explicit DBCon(DBSettings* _settings, QString _dbName, QObject* _parent=0);
  explicit DBCon(const DBCon& orig) = delete;
  ~DBCon();

  bool isValidDatabaseDriver(QString type);
  QSqlDatabase connectToDB(QString connectionName);

  QSqlDatabase getDatabaseObject() const {return db;}

public slots:
  void execQuery(QString query);

private:
  DBSettings*   settings;
  QString       dbName;
  QSqlDatabase  db;
};

#endif  // DBCON_H
