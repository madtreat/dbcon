#ifndef DBCONFWINDOW_H
#define DBCONFWINDOW_H

#include <QFrame>

class QWidget;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QTextEdit;

class DBSettings;
// class DBCon;


class DBConfWindow : public QFrame {
  Q_OBJECT;

public:
  explicit DBConfWindow(DBSettings* _dbs = 0, QFrame* _parent = 0);
  explicit DBConfWindow(const DBConfWindow& orig) = delete;
  ~DBConfWindow();

public slots:
  void testConnection();
  void saveDBSettings();

  void dbTypeChanged(int     index);
  void dbHostChanged(QString hostText);
  void dbPortChanged(quint16 port);
  void dbNameChanged(QString name);
  void dbUserChanged(QString user);
  void dbPassChanged(QString pass);
  void dbFileChanged(QString file);

  void showPass(bool checked);
  void locateDBFile();

signals:
  void savedDBSettings();

private:
  DBSettings* dbSettings;
  bool saved;  // Current settings are saved?
  bool tempSettings; // are these settings in a temporary file? (app first run)

  QComboBox*    dbTypeWidget;
  QLineEdit*    dbHostWidget;
  QSpinBox*     dbPortWidget;
  QLineEdit*    dbNameWidget;
  QLineEdit*    dbUserWidget;
  QLineEdit*    dbPassWidget;
  QWidget*      dbPassWGroup;  // widget group
  QLineEdit*    dbFileWidget;
  QWidget*      dbFileWGroup;  // widget group

  QPushButton*  showPassButton;
  QPushButton*  dbFileLocateButton;

  QPushButton*  testButton;
  QPushButton*  saveButton;

  QTextEdit*    statusBox;

  quint16 getDefaultPort();
};

#endif  // DBCONFWINDOW_H
