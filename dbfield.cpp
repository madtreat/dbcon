
#include "dbfield.h"

#include <QString.h>
#include <QStringList>
#include <QMetaEnum>
#include <QVariant>


// DBField::DBField(QObject* _parent)
// : QObject(_parent) {
// }

// DBField::~DBField() {
// }

QString DBField::fieldTypeToString(DBFieldType type) {
  return QString(QMetaEnum::fromType<DBField::DBFieldType>().key(type));
}

DBField::DBFieldType DBField::fieldTypeFromString(QString type) {
  return (DBFieldType) QMetaEnum::fromType<DBField::DBFieldType>().keyToValue(type.toLatin1().data());
}

void DBField::parseChoices(QString raw) {
  if (raw == "") {
    return;
  }
  QStringList list = raw.split(", ");
  foreach(QString choice, list) {
    QStringList parts = choice.split(" :: ");
    choices.insert(parts.at(0), parts.at(1));
  }
}

