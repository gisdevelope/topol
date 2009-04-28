#ifndef CHECKDOCK_H
#define CHECKDOCK_H

#include <QDockWidget>

#include <qgsvectorlayer.h>

#include "ui_checkDock.h"
#include "rulesDialog.h"

enum validationError {
  ErrorIntersection = 1,
  ErrorOverlap,
  ErrorTolerance,
  ErrorDangle
};

class checkDock : public QDockWidget, public Ui::checkDock
{
Q_OBJECT

public:
  checkDock(const QString &tableName, QgsVectorLayer *theLayer, rulesDialog* theConfigureDialog, QWidget *parent = 0);
  ~checkDock();

private slots:
  void configure();
  void validate(QgsRectangle rect);
  void validateAll();
  void validateExtent();
  void errorListClicked(const QModelIndex& index);

private:
  QgsVectorLayer *mLayer;
  rulesDialog* mConfigureDialog;
  QMap<validationError, QString> mErrorNameMap;
  QMap<validationError, QString> mErrorFixMap;
  QMap<int, QgsRectangle> mErrorRectangleMap;
  QgsGeometryMap mGeometryMap;

  void initErrorMaps();
  void checkIntersections();
  void checkDanglingEndpoints();
  void updateValidationDock(int row, validationError errorType);
};

#endif
