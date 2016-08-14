/*
 *  TODO: Need a way to ensure all table dependencies are resolved in order.
 *  example: when CREATE TABLE'ing, make sure Foreign Key referenced tables
 *  appear before the referencing tables.
 */

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
          DBField* field = readField(true);
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
          DBField* field = readField(true);
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

/*
 *  Returns a valid CREATE TABLE command for MySQL databases for this table.
 */
QString DBTable::createTableCommand() const {
  QString str = "CREATE TABLE " + m_name + " (\n";

  // Add each field with attributes
  for (int i = 0; i < m_fields.size(); i++) {
    DBField* field = m_fields.at(i);
    QString type = field->fieldTypeStr().remove("FT_");
    str += "  " + field->name + " ";

    if (field->type == DBField::FT_VARCHAR || 
        field->type == DBField::FT_CHOICE) {
      // Add size, if needed, to type
      if (field->type == DBField::FT_CHOICE) {
        type = "VARCHAR";
      }

      str += type + "(" + QString::number(field->maxSize) + ")";
    } else {
      str += type;
    }

    if (!field->allowNull) {
      str += " NOT NULL";
    }
    if (field->autoIncrement) {
      str += " AUTO_INCREMENT";
    }

    if (i < m_fields.size() - 1) {
      str += ", \n";
    }
  }

  // Add Primary Key constraints
  if (m_pks.size() == 1) {
    str += ", \n  PRIMARY KEY (" + m_pks.at(0)->name + ")";
  }
  else if (m_pks.size() > 1) {
    QString pkName = "pk_";
    QString pkList;
    for (int i = 0; i < m_pks.size(); i++) {
      QString name = m_pks.at(i)->name;
      pkList += name;
      pkName += name;
      if (i < m_pks.size() - 1) {
        pkList += ",";
      }
    }
    str += ", \n  CONSTRAINT " + pkName + " PRIMARY KEY (" + pkList + ")";
  }

  // Add Unique Key constraints
  if (m_uks.size() == 1) {
    str += ", \n  UNIQUE (" + m_uks.at(0)->name + ")";
  }
  else if (m_uks.size() > 1) {
    QString ukName = "uk_";
    QString ukList;
    for (int i = 0; i < m_uks.size(); i++) {
      QString name = m_uks.at(i)->name;
      ukList += name;
      ukName += name;
      if (i < m_uks.size() - 1) {
        ukList += ",";
      }
    }
    str += ", \n  CONSTRAINT " + ukName + " UNIQUE (" + ukList + ")";
  }

  // Add Foreign Key constraints
  if (m_fks.size() > 0) {
    foreach (DBField* f, m_fks) {
      str += ", \n  ";
      if (f->fkConstraint != "") {
        str += "CONSTRAINT " + f->fkConstraint + " ";
      }
      str += "FOREIGN KEY (" + f->name + ") REFERENCES " + f->fkRef;
    }
  }
  
  str += "\n);\n";
  return str;
}

DBField* DBTable::readField(bool ignorePK) {
  DBField* field = new DBField();
  field->autoIncrement = false;
  field->name         = settings->value("name").toString();
  QString type        = settings->value("type").toString().toUpper();
  field->type         = DBField::fieldTypeFromString("FT_" + type);
  field->maxSize      = settings->value("max_size").toInt();
  field->allowNull    = settings->value("allow_null").toBool();
  field->key          = settings->value("key").toString();
  if (ignorePK && field->key == "PRI") {
    field->key = "";
  }
  field->defaultValue = settings->value("default");
  field->fkRef        = settings->value("fk_ref").toString();
  field->fkConstraint = settings->value("fk_constraint").toString();
  QString choicesRaw  = settings->value("choices").toString();
  field->parseChoices(choicesRaw);

  if (field->key == "PRI") {
    m_pks.append(field);
    if (field->name == "id" && field->type == DBField::FT_INT) {
      field->autoIncrement = true;
    }
  }
  else if (field->key == "UNI") {
    m_uks.append(field);
  }
  else if (field->key == "FOR") {
    m_fks.append(field);
  }
  return field;
}

