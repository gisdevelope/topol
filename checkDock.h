#ifndef CHECKDOCK_H
#define CHECKDOCK_H

#include <QDockWidget>

#include <qgsvectorlayer.h>

#include "ui_checkDock.h"
#include "rulesDialog.h"
#include "validationDock.h"

class checkDock : public QDockWidget, public Ui::checkDock
{
Q_OBJECT

public:
  checkDock(const QString &tableName, QgsVectorLayer *theLayer, rulesDialog* theConfigureDialog, QWidget *parent = 0);
  ~checkDock();

private:
  QgsVectorLayer *mLayer;
  rulesDialog* mConfigureDialog;
  validationDock* mValidationDock;
  QMap<validationError, QString> mErrorNameMap;
  QMap<validationError, QString> mErrorFixMap;
  QMap<int, QgsRectangle> mErrorRectangleMap;

  void initErrorMaps();
  void checkForIntersections(QgsFeatureList featureList);
  void updateValidationDock(int row, validationError errorType);

private slots:
  void configure();
  void validateAll();
  void validateExtent();
};

#endif
