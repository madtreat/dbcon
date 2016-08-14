
#include "dbcon.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include "dbconsts.h"
#include "dbsettings.h"


DBCon::DBCon(DBSettings* _settings, QString _dbName, QObject* _parent)
: QObject(_parent),
settings(_settings),
dbName(_dbName) {
  db = connectToDB(dbName);
  if (db.isValid()) {
    qDebug() << "Connection to database established!";
  }
}

DBCon::~DBCon() {
  db.close();
  QSqlDatabase::removeDatabase(dbName);
}

bool DBCon::isValidDatabaseDriver(QString type) {
  QStringList availableDrivers = QSqlDatabase::drivers();
  return availableDrivers.contains(type);
}

QSqlDatabase DBCon::connectToDB(QString connectionName) {
  QString dbType = "Q" + settings->dbTypeStr().toUpper();
  if (!isValidDatabaseDriver(dbType)) {
    qWarning() << "Warning: The database you are attempting to use ("
               << dbType << ") is not supported by this system.";
    qWarning() << "You may not have the correct database client installed.";
    qWarning() << "Available database types are:" << QSqlDatabase::drivers();
    return QSqlDatabase();
  }

  QSqlDatabase dbc = QSqlDatabase::addDatabase(dbType, connectionName);
  dbc.setHostName(settings->dbHost().toString());
  dbc.setPort(settings->dbPort());
  dbc.setDatabaseName(settings->dbName());

  // NOTE: Do not setting the DB's user and password on the object for security.
  // If they are set explicitly on the object, Qt stores them in RAM.
  // Instead, using the following open() call, Qt passes them directly to the
  // driver and then discards the values.
  bool ok = dbc.open(settings->dbUser(), settings->dbPass());
  if (!ok) {
    qWarning() << "Error opening connection to database:" << connectionName;
    qWarning() << "Error code:" << dbc.lastError().nativeErrorCode();
    qWarning() << dbc.lastError().text();
  }
  return dbc;
}

QSqlQuery DBCon::execQuery(QString queryStr) {
  QSqlQuery q(queryStr, db);
  bool ok = q.exec();
  if (!ok) {
    qWarning() << "Error querying database.";
    qWarning() << "Error code:" << db.lastError().nativeErrorCode();
    qWarning() << db.lastError();
    return QSqlQuery();
  }
  else {
    if (settings->debugQueries()) {
      qDebug() << "Query successful!";
      if (db.driver()->hasFeature(QSqlDriver::QuerySize)) {
        qDebug() << "Found" << q.size() << "records";
      }
      qDebug() << "Results:";
    }
    // while (q.next()) {
    //   // QString name = q.value(0).toString();
    //   // int salary = q.value(1).toInt();
    //   // qDebug() << name << salary;
    // }
    return q;
  }
}

