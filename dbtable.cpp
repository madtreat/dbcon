
#include "dbtable.h"

#include <QString>
#include <QSettings>
#include <QStringList>
#include <QMap>

#include "dbfield.h"


DBTable::DBTable(QString _name, QSettings* _settings, QObject* _parent)
: QObject(_parent),
name(_name),
settings(_settings),
numFields(0),
versionMajor(0),
versionMinor(0) {
  QString tableStr = "table-" + name;
  versionMajor = settings->value(tableStr + "/version_major").toInt();
  versionMinor = settings->value(tableStr + "/version_minor").toInt();
  QString copyFrom = settings->value(tableStr + "/copy_from").toString();

  // Read copyFrom tables' fields
  if (copyFrom != "") {
    QStringList copyList = copyFrom.split(", ");
    foreach(QString copyField, copyList) {
      QStringList parts = copyField.split("/");
      QString table = parts.at(0);
      QString fieldNum = parts.at(1);

      if (fieldNum == "all") {
        QString tempGrp = "table-" + table;
        int tableFields = settings->beginReadArray(tempGrp);
        for (int i = 0; i < tableFields; i++) {
          settings->setArrayIndex(i);
          DBField* field = readField();
          fields.append(field);
        }
        settings->endArray();  // [tempGrp]
      } else {
        bool ok = false;
        int num = fieldNum.toInt(&ok);
        if (ok) {
          numFields += 1;
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
  numFields += size;

  for (int i = 0; i < size; i++) {
    settings->setArrayIndex(i);
    DBField* field = readField();
    fields.append(field);
  }
  settings->endArray();  // [tableStr]
}

DBTable::~DBTable() {
  for (int i = 0; i < fields.size(); i++) {
    DBField* field = fields.at(i);
    delete field;
    field = NULL;
  }
}

DBField* DBTable::readField() {
  DBField* field = new DBField();
  field->name         = settings->value("name").toString();
  int type            = settings->value("type").toInt();
  field->type         = (DBFieldType) type;
  field->maxSize      = settings->value("max_size").toInt();
  field->allowNull    = settings->value("allow_null_values").toBool();
  field->key          = settings->value("key").toString();
  field->defaultValue = settings->value("default");
  QString choicesRaw  = settings->value("choices").toString();
  field->parseChoices(choicesRaw);
  return field;
}

