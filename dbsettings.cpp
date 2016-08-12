#include "dbsettings.h"

#include <QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QMetaEnum>
#include <QHostInfo>

#include "dbconsts.h"
#include "dbtable.h"


#define DEBUG_SETTINGS 0

DBSettings::DBSettings(QString _filename, QObject* _parent)
: QObject(_parent) {
  settings = NULL;
  m_userHomeDir = QDir::home().absolutePath();

  // Get config file from [application root directory]/config
  QString appBinDir = QCoreApplication::applicationDirPath();
  QDir appRoot(appBinDir);

  appRoot.cdUp();
  m_appRootDir = appRoot.absolutePath();
  m_configDir = appRoot.absolutePath() + "/config";

  if (_filename == "") {
    m_settingsFile = m_configDir + "/db-settings.ini";
  }
  else {
    m_settingsFile = _filename;
    m_configDir = QFileInfo(m_settingsFile).absolutePath();
  }

  if (m_settingsFile.contains("/tmp")) {
    qDebug() << "Creating temporary settings in" << m_settingsFile;
    setDefaults();
    return;
  }
  else if (!QFile::exists(m_settingsFile)) {
    qWarning() << "Warning: Settings file" << m_settingsFile << "does not exist.";
    m_configDir = m_userHomeDir + "/.dbc";
    m_settingsFile = m_configDir + "/db-settings.ini";
    qWarning() << "   Trying" << m_settingsFile << "...";
    if (!QFile::exists(m_settingsFile)) {
      qWarning() << "   File not found\n";
      qWarning() << "Warning: No valid config files found.";
      qWarning() << "NOTE: you can specify a config file with the '--db-config <settings.ini>' option.";
      qWarning() << "This will be an invalid config until it is set up properly.";
      setDefaults();
      return;
    }
  }

  loadSettingsFile(m_settingsFile);
  qDebug() << "Loaded settings file:" << m_settingsFile;
}

DBSettings::~DBSettings() {
}

QString DBSettings::dbTypeToString(DBType type) {
  return QString(QMetaEnum::fromType<DBSettings::DBType>().key(type));
}

void DBSettings::loadSettingsFile(QString _filename) {
  if (settings) {
    qDebug() << "Settings object already exists, recreating it...";
    delete settings;
  }
  settings = new QSettings(_filename, QSettings::IniFormat);
  qDebug() << "Child Keys/Groups:" << settings->childKeys() << "/" << settings->childGroups();

  // Load general settings
  settings->beginGroup("database");
  m_dbType = (DBType) settings->value("type").toInt();
  QString dbh = settings->value("host").toString();
  m_dbHost = checkHost(dbh);
  m_dbPort = settings->value("port").toInt();
  m_dbName = settings->value("name").toString();
  m_dbUser = settings->value("user").toString();
  m_dbPass = settings->value("pass").toString();
  m_dbFile = settings->value("file").toString();
  settings->endGroup();  // "database"

  QStringList childGroups = settings->childGroups();
  foreach(QString child, childGroups) {
    if (child.startsWith("table-")) {
      DBTable* table = new DBTable(child.remove("table-"), settings, this);
      m_tables.append(table);
    }
  }
}

void DBSettings::saveSettingsFile() {
  if (!settings) {
    settings = new QSettings(m_settingsFile, QSettings::IniFormat);
  }
  exportSettingsFile(m_settingsFile, false);
  qDebug() << "Database settings saved.";
}

/*
 *  Export the settings to a new file, keeping the original file in place
 *  unless deleteOld is true.
 */
void DBSettings::exportSettingsFile(QString _filename, bool deleteOld) {
  QSettings* sf = NULL;
  if (deleteOld) {
    if (settings) {
      delete settings;
      QFile::remove(m_settingsFile);
    }
    m_settingsFile = _filename;
    settings = new QSettings(m_settingsFile, QSettings::IniFormat);
    sf = settings;
  }
  else {
    sf = new QSettings(_filename, QSettings::IniFormat);
  }
  
  sf->beginGroup("database");
  sf->setValue("type", dbTypeStr().toLower());
  sf->setValue("host", m_dbHost.toString());
  sf->setValue("port", m_dbPort);
  sf->setValue("name", m_dbName);
  sf->setValue("user", m_dbUser);
  sf->setValue("pass", m_dbPass);
  sf->setValue("file", m_dbFile);
  sf->endGroup();  // "database"
  qDebug() << "Database settings exported to" << _filename;

  // Cleanup
  if (deleteOld) {
    sf = NULL;
  } else {
    delete sf;
  }
}

void DBSettings::setDefaults() {
  m_debug = DEBUG_NONE;
  m_dbType = MYSQL;
  m_dbHost = QHostAddress::LocalHost;
  m_dbPort = DEFAULT_PORT_MYSQL;
  m_dbName = "dbc";
  m_dbUser = "dbc";
  m_dbPass = "dbc";
  m_dbFile = QString();
}

QHostAddress DBSettings::checkHost(QString h) {
  QHostAddress host(h);
  if (debugSettings()) {
    qDebug() << "Validating host" << h;
  }
  // If the host given was a hostname...
  if (host == QHostAddress("")) {
    QHostInfo info = QHostInfo::fromName(h);
    QList<QHostAddress> addrs = info.addresses();
    if (debugSettings() && addrs.size()) {
      qDebug() << "Found hosts for reverse lookup of" << h << ":" << addrs;
    }
    if (!addrs.size()) {
      qWarning() << "Warning: host not found, skipping...";
      return QHostAddress();
    }
    return addrs.at(0);
  } 
  // ...else, if the host was an IP...
  else {
    return host;
  }
}
