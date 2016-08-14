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

  QString       name() const {return m_name;}
  DBFieldList fields() const {return m_fields;}
  int      numFields() const {return m_numFields;}
  int   versionMajor() const {return m_versionMajor;}
  int   versionMinor() const {return m_versionMinor;}

  QString getPrintableFields() const;
  QString createTableCommand() const;

public slots:
  // void readFromSettings();
  // void saveSettings();

private:
  QSettings* settings;

  QString     m_name;
  DBFieldList m_fields;
  int         m_numFields;
  int         m_versionMajor;
  int         m_versionMinor;

  QString     m_pkName;
  QString     m_ukName;

  DBFieldList m_pks;  // Primary keys
  DBFieldList m_uks;  // Unique keys
  DBFieldList m_fks;  // Foreign keys

  DBField* readField();

  QString keyLine(QString keyType);
};

typedef QList<DBTable*> DBTableList;

#endif  // DBTABLE_H
