/***************************************************************************
  dockModel.cpp 
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dockModel.h"
#include "topolError.h"

DockModel::DockModel(ErrorList& theErrorList, QObject *parent = 0) : mErrorlist(theErrorList)
{
  mHeader << "Error" << "Feature ID #1" << "Feature ID #2";
}

int DockModel::rowCount(const QModelIndex &parent) const
{
  return mErrorlist.count();
}

int DockModel::columnCount(const QModelIndex &parent) const
{
  return 3;
}

QVariant DockModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Vertical) //row
    {
      return QVariant(section);
    }
    else
    {
      return mHeader[section];
    }
  }
  else return QVariant();
}

QVariant DockModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || (role != Qt::TextAlignmentRole && role != Qt::DisplayRole && role != Qt::EditRole) )
    return QVariant();
	
  int row = index.row();
  int column = index.column();
  //if (!column)
  //QVariant::Type fldType = mLayer->dataProvider()->fields()[column].type();
  //QVariant::Type fldType = mLayer->dataProvider()->fields()[index.column()].type();
  //bool fldNumeric = (fldType == QVariant::Int || fldType == QVariant::Double);
  
  if (role == Qt::TextAlignmentRole)
  {
    if (column)
      return QVariant(Qt::AlignRight);
    else
      return QVariant(Qt::AlignLeft);
  }
  
  QVariant val;
  switch (column)
  {
    case 0:
      val = mErrorlist[row]->name();
    break;
    case 1:
      val = mErrorlist[row]->featurePairs().first().feature.id();
    break;
    case 2:
      val = mErrorlist[row]->featurePairs()[1].feature.id();
    break;
    default:
      val = QVariant();
  }
  
  if (val.isNull())
  {
    return QVariant();
  }
  
  // convert to QString from some other representation
  // this prevents displaying greater numbers in exponential format
  return val.toString();
}

bool DockModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  return false;
}

Qt::ItemFlags DockModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  Qt::ItemFlags flags = QAbstractItemModel::flags(index);
  return flags;
}

void DockModel::resetModel()
{
  reset();
}

void DockModel::reload(const QModelIndex &index1, const QModelIndex &index2)

{
  emit dataChanged(index1, index2);
}
