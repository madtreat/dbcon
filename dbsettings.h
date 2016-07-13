#ifndef DBSETTINGS_H
#define DBSETTINGS_H

#include <QObject>
#include <QString>
#include <QHostAddress>


class QSettings;


class DBSettings : public QObject {
  Q_OBJECT;

public:
  explicit DBSettings(QString _filename, QObject* _parent = 0);
  explicit DBSettings(const DBSettings& orig) = delete;
  ~DBSettings();

  QString   configDir()       const {return m_configDir;}
  QString   appRootDir()      const {return m_appRootDir;}
  QString   userHomeDir()     const {return m_userHomeDir;}
  QString   settingsFile()    const {return m_settingsFile;}

  enum DebugLevel {
    DEBUG_NONE      = 0,
    DEBUG_SETTINGS  = 1 << 0,
    DEBUG_QUERIES   = 1 << 1
  };
  Q_DECLARE_FLAGS(DebugLevels, DebugLevel);
  Q_FLAG(DebugLevels);

  bool debugSettings() const {return m_debug & DEBUG_SETTINGS;}
  bool debugQueries()  const {return m_debug & DEBUG_QUERIES;}

  // Database connection settings
  enum DBType {
    MYSQL = 0,
    ODBC,
    PSQL,
    SQLITE2,
    SQLITE
  };
  Q_ENUM(DBType);

  static QString dbTypeToString(DBType type);

  DBType        dbType() const {return m_dbType;}
  QHostAddress  dbHost() const {return m_dbHost;}
  quint16       dbPort() const {return m_dbPort;}
  QString       dbName() const {return m_dbName;}
  QString       dbUser() const {return m_dbUser;}
  QString       dbPass() const {return m_dbPass;}
  QString       dbFile() const {return m_dbFile;}

  QString    dbTypeStr() const {return dbTypeToString(m_dbType);}

  void setDBType(DBType type)       {m_dbType = type;}
  void setDBHost(QHostAddress host) {m_dbHost = host;}
  void setDBPort(quint16 port)      {m_dbPort = port;}
  void setDBName(QString name)      {m_dbName = name;}
  void setDBUser(QString user)      {m_dbUser = user;}
  void setDBPass(QString pass)      {m_dbPass = pass;}
  void setDBFile(QString file)      {m_dbFile = file;}

public slots:
  void loadSettingsFile(QString _filename);
  // void saveSettingsFile();
  // void exportSettingsFile(QString _filename);

private:
  QSettings*  settings;
  QString     m_configDir;
  QString     m_appRootDir;
  QString     m_userHomeDir;
  QString     m_settingsFile;

  // Debug type
  DebugLevel  m_debug;

  // Database connection settings
  DBType        m_dbType;
  QHostAddress  m_dbHost;
  quint16       m_dbPort;
  QString       m_dbName;
  QString       m_dbUser;
  QString       m_dbPass;
  QString       m_dbFile;

  void setDefaults();
  QHostAddress checkHost(QString h);
};

#endif  // DBSETTINGS_H

