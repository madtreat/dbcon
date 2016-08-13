
#include "dbtable.h"

#include <QString>
#include <QSettings>
#include <QStringList>
#include <QMap>
#include <QDebug>

#include "dbfield.h"


DBTable::DBTable(QString _name, QSettings* _settings, QObject* _parent)
: QObject(_parent),
m_name(_name),
settings(_settings),
m_numFields(0),
m_versionMajor(0),
m_versionMinor(0) {
  QString tableStr = "table-" + m_name;
  m_versionMajor = settings->value(tableStr + "/version_major").toInt();
  m_versionMinor = settings->value(tableStr + "/version_minor").toInt();
  QString copyFrom = settings->value(tableStr + "/copy_fields").toString();

  // Read copyFrom tables' m_fields
  if (copyFrom != "") {
    QStringList copyList = copyFrom.split(", ");
    foreach(QString copyField, copyList) {
      QStringList parts = copyField.split("/");
      QString table = parts.at(0);
      QString fieldNum = parts.at(1);
      qDebug() << "Copying settings from" << table << "/" << fieldNum;

      if (fieldNum == "all") {
        QString tempGrp = "table-" + table;
        int tableFields = settings->beginReadArray(tempGrp);
        for (int i = 0; i < tableFields; i++) {
          settings->setArrayIndex(i);
          DBField* field = readField();
          m_fields.append(field);
        }
        settings->endArray();  // [tempGrp]
      } else {
        bool ok = false;
        int num = fieldNum.toInt(&ok);
        if (ok) {
          m_numFields += 1;
          QString tempGrp = "table-" + copyField;
          settings->beginGroup(tempGrp);
          DBField* field = readField();
          settings->endGroup();  // [tempGrp]
        }
      }
    }
  }

  // Read this table
  int size   = settings->beginReadArray(tableStr);
  m_numFields += size;

  for (int i = 0; i < size; i++) {
    settings->setArrayIndex(i);
    DBField* field = readField();
    m_fields.append(field);
  }
  settings->endArray();  // [tableStr]
}

DBTable::~DBTable() {
  for (int i = 0; i < m_fields.size(); i++) {
    DBField* field = m_fields.at(i);
    delete field;
    field = NULL;
  }
}

QString DBTable::getPrintableFields() const {
  QString str = "Table name: " + m_name + "\n";
  qDebug() << "Table name:" << m_name;
  foreach (DBField* field, m_fields) {
    str += "  + " + field->name + " (" + DBField::fieldTypeToString(field->type).remove("FT_") + ")\n";
    qDebug() << "Type read:"<< field->type << "(" << (int) field->type << ")";
  }
  return str;
}

DBField* DBTable::readField() {
  DBField* field = new DBField();
  field->name         = settings->value("name").toString();
  QString type        = settings->value("type").toString().toUpper();
  field->type         = DBField::fieldTypeFromString("FT_" + type);
  field->maxSize      = settings->value("max_size").toInt();
  field->allowNull    = settings->value("allow_null_values").toBool();
  field->key          = settings->value("key").toString();
  field->defaultValue = settings->value("default");
  QString choicesRaw  = settings->value("choices").toString();
  field->parseChoices(choicesRaw);
  return field;
}

