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

  DBField* readField();
};

typedef QList<DBTable*> DBTableList;

#endif  // DBTABLE_H
