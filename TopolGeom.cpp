#include <QSet>
#include <QList>
#include <QTextEdit>

#include <qgsgeometry.h>
#include <qgspoint.h>

#include "TopolGeom.h"

TopolGeom::TopolGeom()
{
}

TopolGeom::TopolGeom(QgsVectorLayer *theLayer, QTextEdit *theWindow)
{
  mLayer = theLayer;
  mWindow = theWindow;
    
  getFeatures();
}	

TopolGeom::~TopolGeom()
{
  //TODO: free geometries in mObjects
}

void TopolGeom::getFeatures()
{
  mLayer->select(QgsAttributeList());

  QgsFeature f;

  while (mLayer->nextFeature(f))
    mObjects[f.id()] = *f.geometryAndOwnership();
}

QgsFeatureIds TopolGeom::checkGeometry(CheckType type)
{
  mWindow->append("Check for: " + type);
  QGis::GeometryType layertype = mLayer->geometryType();

  switch (type)
  {
    case Intersection:
      if (layertype != QGis::Point)
        cgIntersect();
      break;
    case Overlap:
      if (layertype != QGis::Point)
        cgOverlap();
      break;
    case Multipart:
      if (layertype != QGis::Point)
        cgMultipart();
      break;
  }

  return mConflicting;
}

void TopolGeom::cgIntersect()
{
  int conflicts = 0;
  QMap<int, QgsGeometry>::Iterator obj1 = mObjects.begin();
  QMap<int, QgsGeometry>::Iterator obj2;
  for (; obj1 != mObjects.end(); ++obj1)
    for (obj2 = mObjects.begin(); obj2 != mObjects.end(); ++obj2)
    {
      if (obj1.key() >= obj2.key())
	continue;

      if (obj1.value().intersects(&obj2.value()))
      {
        ++conflicts;
	mConflicting |= obj1.key();
	mConflicting |= obj2.key();
      }
    }

  QString text = QString("# of intersections: %1\n").arg(conflicts);
  mWindow->append(text);
}

void TopolGeom::cgOverlap()
{
  int conflicts = 0;
  QMap<int, QgsGeometry>::Iterator obj1 = mObjects.begin();
  QMap<int, QgsGeometry>::Iterator obj2;
  for (; obj1 != mObjects.end(); ++obj1)
    for (obj2 = mObjects.begin(); obj2 != mObjects.end(); ++obj2)
    {
      if (obj1.key() >= obj2.key())
	continue;

      QgsGeometry *intersection = obj1.value().intersection(&obj2.value());
      QGis::GeometryType type = intersection->type();

      if (type != QGis::Point && type != QGis::UnknownGeometry)
      {
        ++conflicts;
	mConflicting |= obj1.key();
	mConflicting |= obj2.key();

        QString text = QString("Conflict: %1 %2\n").arg(obj1.key()).arg(obj2.key());
        mWindow->append(text);
      }
    }

  QString text = QString("# of intersections: %1\n").arg(conflicts);
  mWindow->append(text);
}

void TopolGeom::cgMultipart()
{
  int conflicts = 0;
  QMap<int, QgsGeometry>::Iterator obj = mObjects.begin();

  for (; obj != mObjects.end(); ++obj)
    if (obj.value().isMultipart())
    {
      ++conflicts;
      mConflicting |= obj.key();
      
      QString text = QString("Conflict: %1\n").arg(obj.key());
      mWindow->append(text);
    }  

  QString text = QString("# of intersections: %1\n").arg(conflicts);
  mWindow->append(text);
}

//TODO: split or simplify
  // TODO: arcs added more than once
void TopolGeom::buildIntersections()
{
  QMap<int, QgsGeometry>::Iterator obj1 = mObjects.begin();
  QMap<int, QgsGeometry>::Iterator obj2;

  bool intersecting;

  // add all intersection points to nodes list
  for (obj1 = mObjects.begin(); obj1 != mObjects.end(); ++obj1)
  {
    intersecting = false;

    for (obj2 = mObjects.begin(); obj2 != mObjects.end(); ++obj2)
    {
      if (obj1.key() >= obj2.key())
	continue;

      QgsGeometry *intersection = obj1.value().intersection(&obj2.value());
  //std::cout << "intersekci type: " << intersection->type() << "\n" << std::flush;

      QgsPoint pt;
      QgsMultiPoint pts;

      // check if intersection is point or multipoint
      if (intersection->type() == 0)
      {
        if ((pt = intersection->asPoint()) != QgsPoint(0,0))
	  pts.append(pt);
	else
	  pts = intersection->asMultiPoint();
      }
      // no intersections found
      else
      {
        intersecting = true;
        continue;
      }

      if (pts != QgsMultiPoint())
      {
        TopolNode node;
        int at, before, after;
	double dist;
	
	QgsMultiPoint intersections;
	QgsMultiPoint::ConstIterator mit = pts.begin();

	// creates intersection list
	for (; mit != pts.end(); ++mit)
	{
	  intersections.append(*mit);

	  // TODO: this should work on temporary objects, if not solved in a more clever way
          obj1.value().closestVertex(*mit, at, before, after, dist);
          obj1.value().insertVertex(mit->x(), mit->y(), after);
          obj2.value().closestVertex(*mit, at, before, after, dist);
          obj2.value().insertVertex(mit->x(), mit->y(), after);
	} 

        QgsPolyline arc;
        QgsPolyline polyline = obj1.value().asPolyline();
	QgsPolyline::Iterator it =  polyline.begin();

	//TODO: simplify
	// walk along first line and add arcs to intersection nodes
	node.point = polyline.first();
        for (; it != polyline.end(); ++it)
	{
	  arc.append(*it);

          if (intersections.contains(*it)) 
	  {
	    node.arcs.append(arc);
	    mNodes.append(node);
	    node.arcs.clear();

	    node.point = *it;
	    node.arcs.append(arc);
            arc.clear();
	    arc.append(*it);
	  }
	}

	// append arc to current intersection and line endpoint
	node.arcs.append(arc);
	mNodes.append(node);
	node.point = polyline.last();
	node.arcs.clear();
	node.arcs.append(arc);
	mNodes.append(node);

	// walk along second line and add arcs to intersection nodes
	arc.clear();
        polyline = obj2.value().asPolyline();
	node.point = polyline.first();
	node.arcs.clear();

        for (it = polyline.begin(); it != polyline.end(); ++it)
	{
	  arc.append(*it);

          if (intersections.contains(*it)) 
	  {
	    node.arcs.append(arc);
	    mNodes.append(node);
	    node.arcs.clear();

	    node.point = *it;
	    node.arcs.append(arc);
            arc.clear();
	    arc.append(*it);
	  }
	}

	// append arc to current intersection and line endpoint
	node.arcs.append(arc);
	mNodes.append(node);
	node.point = polyline.last();
	node.arcs.clear();
	node.arcs.append(arc);
	mNodes.append(node);
      }
      else
        std::cout << "not multipoint\n" << std::flush;
    }

    if (!intersecting)
    {
      TopolNode node;
      QgsPolyline polyline = obj1.value().asPolyline();

      node.point = polyline.first();
      node.arcs.append(polyline);
      mNodes.append(node);

      node.point = polyline.last();
      mNodes.append(node);
    }
  }
}

// create layers containing nodes and arcs
void TopolGeom::buildGeometry(QgsVectorLayer *nodeLayer, QgsVectorLayer *arcLayer)
{
  mNodes.clear();
  buildIntersections();

  // create node and arc layers
  QgsFeature f;
  int id = 0;

  // node layer
  nodeLayer->startEditing();

  QList<TopolNode>::ConstIterator node = mNodes.begin();
  for (; node != mNodes.end(); ++node)
  {
    f.setGeometry(QgsGeometry::fromPoint(node->point));
    f.addAttribute(0, id);
    nodeLayer->addFeature(f, false);
    ++id;
  }

  nodeLayer->updateExtents();

  // arc layer
  // TODO: arcs added more than once
  arcLayer->startEditing();

  id = 0;
  node = mNodes.begin();
  for (; node != mNodes.end(); ++node)
  {
    QList<QgsPolyline>::ConstIterator arc = node->arcs.begin();
    for (; arc != node->arcs.end(); ++arc)
    {
      f.setGeometry(QgsGeometry::fromPolyline(*arc));
      f.addAttribute(0, id);
      arcLayer->addFeature(f, false);
      ++id;
    }
  }

  arcLayer->updateExtents();
}
