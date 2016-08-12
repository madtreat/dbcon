
#include "dbtable.h"

#include <QString>
#include <QSettings>

#include "dbfield.h"


DBTable::DBTable(QString _name, QSettings* _settings, QObject* _parent)
: QObject(_parent),
name(_name),
settings(_settings),
versionMajor(0),
versionMinor(0) {
  QString tableStr = "table-" + name;
  numFields    = settings->beginReadArray(tableStr);
  versionMajor = settings->value(tableStr + "version_major").toInt();
  versionMinor = settings->value(tableStr + "version_minor").toInt();

  for (int i = 0; i < numFields; i++) {
    settings->setArrayIndex(i);
    DBField* field = new DBField();
    field->name = settings->value("name").toString();
    int type = settings->value("type").toInt();
    field->type = (DBFieldType) type;
    field->maxSize = settings->value("max_size").toInt();
    field->defaultValue = settings->value("default");

    fields.append(field);
  }
  settings->endArray();  // "table-" + <name>
}

DBTable::~DBTable() {
  for (int i = 0; i < numFields; i++) {
    DBField* field = fields.at(i);
    delete field;
    field = NULL;
  }
}

