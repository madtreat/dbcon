
#include "dbconfwindow.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHostInfo>
#include <QHostAddress>
#include <QList>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>

#include "dbconsts.h"
#include "dbsettings.h"
#include "dbcon.h"

// TODO: Add lines for warnings while entering text, after (partial) validation


DBConfWindow::DBConfWindow(DBSettings* _dbs, QFrame* _parent)
: QFrame(_parent),
dbSettings(_dbs),
saved(false) {
  // First, check if the DBSettings object is valid.  This constructor does not
  // need a valid settings object to begin with (if this is the first run of
  // an app, the database settings have probably not been configured yet).
  if (!dbSettings) {
    dbSettings = new DBSettings("/tmp/dbc/dbsettings.ini", this);
    tempSettings = true;
  }
  else {
    tempSettings = false;
  }

  QFormLayout* form = new QFormLayout();

  // Database type widget
  dbTypeWidget = new QComboBox();
  // NOTE: Order of widgets matches order defined by DBSettings::DBType
  dbTypeWidget->addItem(tr("MySQL"));
  dbTypeWidget->addItem(tr("Microsoft SQL or Similar"));
  dbTypeWidget->addItem(tr("Postgre SQL"));
  dbTypeWidget->addItem(tr("SQLite v2"));
  dbTypeWidget->addItem(tr("SQLite v3"));
  // Have to connect after adding the widgets or segfaults happen (b/c)
  // the other widgets are not created yet when trying to enable/disable them
  // in dbTypeChanged().
  connect(dbTypeWidget, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), 
          this,         &DBConfWindow::dbTypeChanged);

  // Database host/IP widget
  dbHostWidget = new QLineEdit();
  dbHostWidget->setPlaceholderText(dbSettings->dbHost().toString());
  // TODO: check out potential for QValidator, confirm correct signal
  connect(dbHostWidget, &QLineEdit::textChanged,
          this,         &DBConfWindow::dbHostChanged);

  // Database port widget
  dbPortWidget = new QSpinBox();
  dbPortWidget->setMinimum(0);
  dbPortWidget->setMaximum(65535);

  // Database name widget
  dbNameWidget = new QLineEdit();
  dbNameWidget->setPlaceholderText(dbSettings->dbName());
  connect(dbNameWidget, &QLineEdit::textChanged,
          this,         &DBConfWindow::dbNameChanged);

  // Database user widget
  dbUserWidget = new QLineEdit();
  dbUserWidget->setPlaceholderText(dbSettings->dbUser());
  connect(dbUserWidget, &QLineEdit::textChanged,
          this,         &DBConfWindow::dbUserChanged);

  // Database password widget
  dbPassWidget = new QLineEdit();
  dbPassWidget->setPlaceholderText(dbSettings->dbPass());
  dbPassWidget->setEchoMode(QLineEdit::Password);
  connect(dbPassWidget, &QLineEdit::textChanged,
          this,         &DBConfWindow::dbPassChanged);

  showPassButton = new QPushButton("Show Password");
  showPassButton->setCheckable(true);
  connect(showPassButton, &QPushButton::clicked, this, &DBConfWindow::showPass);
  showPassButton->setChecked(false);

  dbPassWGroup = new QWidget();
  // dbPassWGroup->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* passLayout = new QHBoxLayout(dbPassWGroup);
  passLayout->setContentsMargins(0, 0, 0, 0);
  passLayout->addWidget(dbPassWidget);
  passLayout->addWidget(showPassButton);

  // Database file widget
  dbFileWidget = new QLineEdit();
  dbFileWidget->setPlaceholderText("File path");
  connect(dbFileWidget, &QLineEdit::textChanged,
          this,         &DBConfWindow::dbFileChanged);

  dbFileLocateButton = new QPushButton("Find File");
  connect(dbFileLocateButton, &QPushButton::clicked, 
          this,               &DBConfWindow::locateDBFile);

  dbFileWGroup = new QWidget();
  // dbFileWGroup->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* fileLayout = new QHBoxLayout(dbFileWGroup);
  fileLayout->setContentsMargins(0, 0, 0, 0);
  fileLayout->addWidget(dbFileWidget);
  fileLayout->addWidget(dbFileLocateButton);

  // Now that everything is created and pointers are validated, we can set
  // some defaults
  if (tempSettings) {
    dbTypeWidget->setCurrentIndex((int)DBSettings::MYSQL);
    dbPortWidget->setValue(getDefaultPort());
  }
  else {
    dbTypeWidget->setCurrentIndex((int)dbSettings->dbType());
    dbPortWidget->setValue(dbSettings->dbPort());
    // TODO: set dbFile also
  }
  dbTypeChanged((int)DBSettings::MYSQL);

  // Add the input widgets to the form layout
  form->addRow(tr("Database &Type:"),   dbTypeWidget);
  form->addRow(tr("IP/&Hostname:"),     dbHostWidget);
  form->addRow(tr("P&ort Number:"),     dbPortWidget);
  form->addRow(tr("Database &Name:"),   dbNameWidget);
  form->addRow(tr("Database &User:"),   dbUserWidget);
  form->addRow(tr("User's &Password:"), dbPassWGroup);
  form->addRow(tr("SQLite 2/3 &File:"), dbFileWGroup);

  // Create the "Do not show this window again" button
  // TODO: Make this checkbox actually hide the window in the future
  QCheckBox* hideWindowInFuture = new QCheckBox("Do not show this window at startup.");
  hideWindowInFuture->setChecked(false);

  // Create the test and save buttons
  QHBoxLayout* buttons = new QHBoxLayout();
  testButton = new QPushButton("Test Connection");
  connect(testButton, &QPushButton::pressed,
          this,       &DBConfWindow::testConnection);
  saveButton = new QPushButton("Save Settings");
  connect(saveButton, &QPushButton::pressed,
          this,       &DBConfWindow::saveDBSettings);
  buttons->addWidget(testButton);
  buttons->addWidget(saveButton);

  // Create the status message area
  statusBox = new QTextEdit();
  statusBox->setReadOnly(true);
  // statusBox->setStyleSheet("QTextArea {background-color: black;}");
  // statusBox->setTextBackgroundColor(Qt::black);
  // statusBox->setTextColor(Qt::green);
  statusBox->setVisible(false);

  // Add everything to the main layout
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addLayout(form);

  QFrame* line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line);

  layout->addWidget(hideWindowInFuture);
  layout->addLayout(buttons);
  layout->addWidget(statusBox);

  show();
}

DBConfWindow::~DBConfWindow() {
  if (!saved) {
    QMessageBox msgBox;
    msgBox.setText("The database configuration has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    switch (ret) {
      case QMessageBox::Save:
          // Save was clicked
          saveDBSettings();
          break;
      case QMessageBox::Discard:
          // Don't Save was clicked
          break;
      case QMessageBox::Cancel:
          // Cancel was clicked
          // TODO: this message box thing should not be in the destructor, but should trap the close signal
          break;
      default:
          // should never be reached
          break;
    }
  }
}

void DBConfWindow::testConnection() {
  statusBox->setVisible(true);
  QString dbTypeStr = dbSettings->dbTypeStr();
  statusBox->insertPlainText(tr("Checking database driver for %1 is installed...").arg(dbTypeStr));
  bool isValidDBDriver = DBCon::isValidDatabaseDriver("Q" + dbTypeStr);
  if (isValidDBDriver) {
    statusBox->insertPlainText(tr("yes\n"));
  }
  else {
    statusBox->setTextColor(Qt::red);
    statusBox->insertPlainText(tr("no"));
    statusBox->setTextColor(Qt::black);
    statusBox->insertPlainText("\n");
  }
  statusBox->insertPlainText(tr("Attempting to connect to database...\n"));
  DBCon dbc(dbSettings, "test-database");
  if (dbc.isValid()) {
    statusBox->insertPlainText(tr("Database connection driver is valid\n"));
  }
  if (dbc.isOpenError()) {
    statusBox->setTextColor(Qt::red);
    statusBox->insertPlainText(tr("Error opening database connection:\n"));
    statusBox->insertPlainText(dbc.lastError());
    statusBox->setTextColor(Qt::black);
    statusBox->insertPlainText("\n");
  }
  else if (dbc.isOpen()) {
    statusBox->insertPlainText(tr("Connection to database has been opened successfully!\n"));
    statusBox->insertPlainText(tr("You should save the database settings now.\n"));
    QString tableCheck = "SHOW TABLES;";
    statusBox->insertPlainText(tr("Checking tables...\n"));
    QSqlQuery results = dbc.execQuery(tableCheck);
    while (results.next()) {
      statusBox->insertPlainText("-> " + results.value(0).toString() + "\n");
    }
  }
}

void DBConfWindow::saveDBSettings() {
  // TODO: FIXME: do this one
  if (tempSettings) {
    QString appDir = dbSettings->appRootDir();
    // QString filename = QFileDialog::getOpenFileName(this,
    //   tr("Select Database file"), appDir);
    QString newConfigDir = appDir + "/config";
    QString filename = newConfigDir + "/db-settings.ini";
    dbSettings->exportSettingsFile(filename, true);
    dbSettings->setConfigDir(newConfigDir);
    tempSettings = false;
  }
  else {
    dbSettings->saveSettingsFile();
  }
  statusBox->insertPlainText(tr("Database settings saved successfully!\n"));
  saved = true;
  emit savedDBSettings();
}

void DBConfWindow::dbTypeChanged(int index) {
  saved = false;
  DBSettings::DBType type = (DBSettings::DBType) index;
  int defaultPort = getDefaultPort();
  if (defaultPort) {
    // If this is a remote database (anything not SQLite)
    bool remote = (type == DBSettings::MYSQL) ||
                  (type == DBSettings::ODBC) ||
                  (type == DBSettings::PSQL);
    dbHostWidget->setEnabled(remote);
    dbPortWidget->setEnabled(remote);
    dbNameWidget->setEnabled(remote);
    dbUserWidget->setEnabled(remote);
    dbPassWGroup->setEnabled(remote);
    dbFileWGroup->setEnabled(!remote);

    dbPortWidget->setValue(defaultPort);
  }
  dbSettings->setDBType(type);
}

void DBConfWindow::dbHostChanged(QString hostText) {
  saved = false;
  QHostAddress hostAddr;
  // Check if valid IP address
  if (hostAddr.setAddress(hostText)) {
    qDebug() << "IP Address is valid";
  }
  // Check if valid Hostname
  else {
    QHostInfo info = QHostInfo::fromName(hostText);
    QList<QHostAddress> addrs = info.addresses();
    // qDebug() << "Found host(s) for reverse lookup of" << hostText << ":" << addrs;
    if (!addrs.size()) {
      qWarning() << "Warning: Invalid database host";
      return;
    }
    hostAddr = addrs.at(0);
  }
  dbSettings->setDBHost(hostAddr);
}

void DBConfWindow::dbPortChanged(quint16 port) {
  saved = false;
  dbSettings->setDBPort(port);
}

void DBConfWindow::dbNameChanged(QString name) {
  saved = false;
  dbSettings->setDBName(name);
}

void DBConfWindow::dbUserChanged(QString user) {
  saved = false;
  dbSettings->setDBUser(user);
}

void DBConfWindow::dbPassChanged(QString pass) {
  saved = false;
  dbSettings->setDBPass(pass);
}

void DBConfWindow::dbFileChanged(QString file) {
  saved = false;
  dbSettings->setDBFile(file);
}

void DBConfWindow::showPass(bool checked) {
  dbPassWidget->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

void DBConfWindow::locateDBFile() {
  QString currentDir = dbFileWidget->text();
  QString dbFile = QFileDialog::getOpenFileName(this,
    tr("SQLite Database File Select"), currentDir);
  dbFileChanged(dbFile);
}

/*
 *  Sets the default port number for the currently selected database type.
 */
quint16 DBConfWindow::getDefaultPort() {
  DBSettings::DBType type = (DBSettings::DBType) dbTypeWidget->currentIndex();
  switch (type) {
    case DBSettings::MYSQL: return DEFAULT_PORT_MYSQL; break;
    case DBSettings::ODBC:  return DEFAULT_PORT_MSSQL; break;
    case DBSettings::PSQL:  return DEFAULT_PORT_PSQL;  break;
    case DBSettings::SQLITE2:
    case DBSettings::SQLITE:
    default:
      return -1;
  }
}

