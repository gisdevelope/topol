#include "topolError.h"

//TODO: fix crashing when no layer under
// fix crashing when feature is deleted
bool TopolError::fix(QString fixName)
{
  std::cout << "fix: \""<<fixName.toStdString()<<"\"\n";
  (this->*mFixMap[fixName])();
}

bool TopolError::fixMove(FeatureLayer fl1, FeatureLayer fl2)
{
  bool ok;
  QgsFeature f1, f2;
  ok = fl1.layer->featureAtId(fl1.feature.id(), f1, true, false);
  ok = ok && fl2.layer->featureAtId(fl2.feature.id(), f2, true, false);

  if (!ok)
    return false;

  // 0 means success
  if(!f1.geometry()->makeDifference(f2.geometry()))
    return fl1.layer->changeGeometry(f1.id(), f1.geometry());

  return false;
}

bool TopolError::fixMoveFirst()
{
  return fixMove(mFeaturePairs.first(), mFeaturePairs[1]);
}

bool TopolError::fixMoveSecond()
{
  return fixMove(mFeaturePairs[1], mFeaturePairs.first());
}

bool TopolError::fixUnion(FeatureLayer fl1, FeatureLayer fl2)
{
  bool ok;
  QgsFeature f1, f2;
  ok = fl1.layer->featureAtId(fl1.feature.id(), f1, true, false);
  ok = ok && fl2.layer->featureAtId(fl2.feature.id(), f2, true, false);

  if (!ok)
    return false;

  QgsGeometry* g = f1.geometry()->combine(f2.geometry());
  if (!g)
    return false;

  if (fl2.layer->deleteFeature(f2.id()))
    return fl1.layer->changeGeometry(f1.id(), g);

  return false;
}

bool TopolError::fixSnap()
{
  bool ok;
  QgsFeature f1, f2;
  FeatureLayer fl = mFeaturePairs[1];
  ok = fl.layer->featureAtId(fl.feature.id(), f2, true, false);
  fl = mFeaturePairs.first();
  ok = ok && fl.layer->featureAtId(fl.feature.id(), f1, true, false);

  if (!ok)
    return false;

  QgsGeometry* ge = f1.geometry();

  QgsPolyline line = ge->asPolyline();
  line.last() = mConflict->asPolyline().last();

  //TODO: this will cause memory leaks - creates new QgsGeometry object
  return fl.layer->changeGeometry(f1.id(), QgsGeometry::fromPolyline(line));
}

bool TopolError::fixUnionFirst()
{
  return fixUnion(mFeaturePairs.first(), mFeaturePairs[1]);
}

bool TopolError::fixUnionSecond()
{
  return fixUnion(mFeaturePairs[1], mFeaturePairs.first());
}

bool TopolError::fixDeleteFirst()
{
  FeatureLayer fl = mFeaturePairs.first();
  return fl.layer->deleteFeature(fl.feature.id());
}

bool TopolError::fixDeleteSecond()
{
//TODO: need to update error list - deleted feature could be involved in many errors!
  FeatureLayer fl = mFeaturePairs[1];
  return fl.layer->deleteFeature(fl.feature.id());
}

TopolErrorIntersection::TopolErrorIntersection(QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs) : TopolError(theBoundingBox, theConflict, theFeaturePairs)
{
  mName = "Intersecting geometries";

  mFixMap["Select automatic fix"] = &TopolErrorIntersection::fixDummy;
  mFixMap["Move blue feature"] = &TopolErrorIntersection::fixMoveFirst;
  mFixMap["Move red feature"] = &TopolErrorIntersection::fixMoveSecond;
  mFixMap["Delete blue feature"] = &TopolErrorIntersection::fixDeleteFirst;
  mFixMap["Delete red feature"] = &TopolErrorIntersection::fixDeleteSecond;

  // allow union only when both features have the same geometry type
  if (theFeaturePairs.first().feature.geometry()->type() == theFeaturePairs[1].feature.geometry()->type())
  {
    mFixMap["Union to blue feature"] = &TopolErrorIntersection::fixUnionFirst;
    mFixMap["Union to red feature"] = &TopolErrorIntersection::fixUnionSecond;
  }
}

TopolErrorDangle::TopolErrorDangle(QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs) : TopolError(theBoundingBox, theConflict, theFeaturePairs)
{
  mName = "Dangling endpoint";

  mFixMap["Select automatic fix"] = &TopolErrorDangle::fixDummy;
  mFixMap["Move blue feature"] = &TopolErrorDangle::fixMoveFirst;
  mFixMap["Move red feature"] = &TopolErrorDangle::fixMoveSecond;
  mFixMap["Snap to segment"] = &TopolErrorDangle::fixSnap;
}

/*TopolErrorContains::TopolErrorContains(QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs) : TopolError(theBoundingBox, theConflict, theFeaturePairs)
{
  mName = "Feature inside polygon";
  mFixMap["Select automatic fix"] = &TopolErrorContains::fixDummy;
  mFixMap["Delete point"] = &TopolErrorContains::fixDeleteSecond;
}*/

TopolErrorCovered::TopolErrorCovered(QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs) : TopolError(theBoundingBox, theConflict, theFeaturePairs)
{
  mName = "Point not covered by segment";
  mFixMap["Select automatic fix"] = &TopolErrorCovered::fixDummy;
  mFixMap["Delete point"] = &TopolErrorCovered::fixDeleteFirst;
}

TopolErrorInside::TopolErrorInside(QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs) : TopolError(theBoundingBox, theConflict, theFeaturePairs)
{
  mName = "Feature inside polygon";
  mFixMap["Select automatic fix"] = &TopolErrorInside::fixDummy;
  mFixMap["Delete feature inside"] = &TopolErrorInside::fixDeleteSecond;
}

TopolErrorShort::TopolErrorShort(QgsRectangle theBoundingBox, QgsGeometry* theConflict, QList<FeatureLayer> theFeaturePairs) : TopolError(theBoundingBox, theConflict, theFeaturePairs)
{
  mName = "Segment too short";
  mFixMap["Select automatic fix"] = &TopolErrorShort::fixDummy;
  mFixMap["Delete feature"] = &TopolErrorShort::fixDeleteFirst;
}
