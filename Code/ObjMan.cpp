/// \file objman.cpp
/// \brief Code for the object manager class CObjectManager.

#include "objman.h"
#include "debug.h"
#include "defines.h"
#include "timer.h"
#include "crow.h"

extern int g_nScreenWidth;
extern int g_nScreenHeight;
extern CTimer g_cTimer; 

/// Comparison for depth sorting game objects.
/// To compare two game objects, simply compare their Z coordinates.
/// \param p0 Pointer to game object 0.
/// \param p1 Pointer to game object 1.
/// \return true If object 0 is behind object 1.

bool ZCompare(const CGameObject* p0, const CGameObject* p1){
  return p0->m_vPos.z > p1->m_vPos.z;
} //ZCompare

CObjectManager::CObjectManager(){ 
  m_stlObjectList.clear();
  m_stlNameToObject.clear();
  m_stlNameToObjectType.clear();
  m_nLastGunFireTime = 0;
} //constructor

CObjectManager::~CObjectManager(){ 
  for(auto i=m_stlObjectList.begin(); i!=m_stlObjectList.end(); i++)
    delete *i;
} //destructor

/// Insert a map from an object name string to an object type enumeration.
/// \param name Name of an object type
/// \param t Enumerated object type corresponding to that name.

void CObjectManager::InsertObjectType(const char* name, ObjectType t){
  m_stlNameToObjectType.insert(pair<string, ObjectType>(name, t)); 
} //InsertObjectType

/// Get the ObjectType corresponding to a type name string. Returns NUM_OBJECT_TYPES
/// if the name is not in m_stlNameToObjectType.
/// \param name Name of an object type
/// \return Enumerated object type corresponding to that name.

ObjectType CObjectManager::GetObjectType(const char* name){
  unordered_map<string, ObjectType>::iterator i = 
    m_stlNameToObjectType.find(name);
  if(i == m_stlNameToObjectType.end()) //if name not in map
    return NUM_OBJECT_TYPES; //error return
  else return i->second; //return object type
} //GetObjectType

/// Create a new instance of a game object.
/// \param obj The type of the new object
/// \param name The name of object as found in name tag of XML settings file
/// \param s Location.
/// \param v Velocity.
/// \return Pointer to object created.

CGameObject* CObjectManager::createObject(ObjectType obj, const char* name, const Vector3& s, const Vector3& v){
  CGameObject* p;

  if(obj == CROW_OBJECT) 
    p = new CCrowObject(name, s, v);
  else p = new CGameObject(obj, name, s, v);    

  m_stlObjectList.push_front(p); //insert in object list

  if(m_stlNameToObject.find(name) == m_stlNameToObject.end()) //if name not in map
    m_stlNameToObject.insert(pair<string, CGameObject*>(name, p)); //put it there

  return p;
} //createObject

/// Create a new instance of a game object with velocity zero.
/// \param objname The name of the new object's type
/// \param name The name of object as found in name tag of XML settings file
/// \param s Location.
/// \return Pointer to object created.

CGameObject* CObjectManager::createObject(const char* objname, const char* name, const Vector3& s){
  ObjectType obj = GetObjectType(objname);
  return createObject(obj, name, s, Vector3(0.0f));
} //createObject

/// Move all game objects, while making sure that they wrap around the world correctly.

void CObjectManager::move(){
  const float dX = (float)g_nScreenWidth; // Wrap distance from plane.

  //find the plane
  unordered_map<string, CGameObject*>::iterator planeIterator = 
    m_stlNameToObject.find((string)"plane");
  CGameObject* planeObject = planeIterator->second;

  //move nonplayer objects
  for(auto i=m_stlObjectList.begin(); i!=m_stlObjectList.end(); i++){ //for each object
    CGameObject* curObject = *i; //current object
    curObject->move(); //move it

    //wrap objects a fixed distance from plane
    if(curObject != planeObject){ //not the plane
      float planeX=0.0f; //plane's X coordinate
      if(planeIterator != m_stlNameToObject.end())
        planeX = planeObject->m_vPos.x;

      float& x = curObject->m_vPos.x; //X coordinate of current object

      if(x > planeX + dX) //too far behind
        x -= 2.0f*dX;
      else if(x < planeX - dX) //too far ahead
        x += 2.0f*dX;
    } //if
  } //for
  
  CollisionDetection(); //collision detection
  cull(); //cull old objects
  GarbageCollect(); //bring out yer dead!
} //move

/// Draw the objects from the object list and the player object. Care
/// must be taken to draw them from back to front.

void CObjectManager::draw(){
  m_stlObjectList.sort(ZCompare); //depth sort
  for(auto i = m_stlObjectList.begin(); i != m_stlObjectList.end(); i++) //for each object
    (*i)->draw();
} //draw

/// Get a pointer to an object by name, nullptr if it doesn't exist.
/// \param name Name of object.
/// \return Pointer to object created with that name, if it exists.

CGameObject* CObjectManager::GetObjectByName(const char* name){ 
  unordered_map<string, CGameObject*>::iterator 
    current = m_stlNameToObject.find((string)name);
  if(current != m_stlNameToObject.end())
    return current->second;
  else return nullptr;
} //GetObjectByName

/// Distance between objects.
/// \param pointer to first object 
/// \param pointer to second object
/// \return distance between the two objects

float CObjectManager::distance(CGameObject *g0, CGameObject *g1){ 
  if(g0 == nullptr || g1 == nullptr)return -1; //bail if bad pointer
  const float fWorldWidth = 2.0f * (float)g_nScreenWidth; //world width
  float x = (float)fabs(g0->m_vPos.x - g1->m_vPos.x); //x distance
  float y = (float)fabs(g0->m_vPos.y - g1->m_vPos.y); //y distance
  if(x > fWorldWidth) x -= (float)fWorldWidth; //compensate for wrap-around world
  return sqrtf(x*x + y*y);
} //distance

/// Fire a bullet from the plane's gun.
/// \param name Name of the object that is to fire the gun.

void CObjectManager::FireGun(char* name){   
  std::unordered_map<std::string, CGameObject*>::iterator planeIterator = 
    m_stlNameToObject.find((std::string)"plane");
  if(planeIterator == m_stlNameToObject.end())return; //this should of course never happen
  const CGameObject* planeObject = planeIterator->second;

  if(g_cTimer.elapsed(m_nLastGunFireTime, 200)){ //slow down firing rate
    const float fAngle = planeObject->m_fOrientation;
    const float fSine = sin(fAngle);
    const float fCosine = cos(fAngle);

    //enter the number of pixels from center of plane to gun
    const float fGunDx = -48.0f; 
    const float fGunDy = -17.0f;

    //initial bullet position
    const Vector3 s = planeObject->m_vPos +
      Vector3(fGunDx*fCosine - fGunDy*fSine, fGunDx*fSine - fGunDy*fCosine, 0); 

    //velocity of bullet
    const float BULLETSPEED = 10.0f;
    const Vector3 v = BULLETSPEED * Vector3(-fCosine, -fSine, 0) +
      planeObject->m_vVelocity;

    createObject(BULLET_OBJECT, "bullet", s, v); //create bullet
  } //if
} //FireGun

/// Cull old objects.
/// Run through the objects in the object list and compare their age to
/// their life span. Kill any that are too old. Immortal objects are
/// flagged with a negative life span, so ignore those.

void CObjectManager::cull(){ 
  for(auto i=m_stlObjectList.begin(); i!=m_stlObjectList.end(); i++){
    CGameObject* object = *i; //current object

    //died of old age
    if(object->m_nLifeTime > 0 && //if mortal and ...
    (g_cTimer.time() - object->m_nBirthTime > object->m_nLifeTime)) //...old...
      object->m_bIsDead = TRUE; //slay it

    //one shot animation 
    if(object->m_nFrameCount > 1 && !object->m_bCycleSprite && //if plays one time...
      object->m_nCurrentFrame >= object->m_nAnimationFrameCount){ //and played once already...
        object->m_bIsDead = TRUE; //slay it
        CreateNextIncarnation(object); //create next in the animation sequence
    } //if
  } //for
} //cull

/// Create the object next in the appropriate series (object, exploding
/// object, dead object). If there's no "next" object, do nothing.
/// \param object Pointer to the object to be replaced

void CObjectManager::CreateNextIncarnation(CGameObject* object){ 
  if(object->m_nObjectType == CROW_OBJECT)
    createObject(EXPLODINGCROW_OBJECT, "explodingcrow", object->m_vPos,
      object->m_vVelocity); //create new one
  else if(object->m_nObjectType == EXPLODINGCROW_OBJECT)
    createObject(DEADCROW_OBJECT, "deadcrow", object->m_vPos,
      object->m_vVelocity); //create new one
} //CreateNextIncarnation

/// Master collision detection function.
/// Compare every object against every other object for collision. Only
/// bullets can collide right now.

void CObjectManager::CollisionDetection(){ 
  for(auto i=m_stlObjectList.begin(); i!=m_stlObjectList.end(); i++)
    if((*i)->m_nObjectType == BULLET_OBJECT) //and is a bullet
      CollisionDetection(*i); //check every object for collision with this bullet
} //CollisionDetection

/// Given an object pointer, compare that object against every other 
/// object for collision. If a collision is detected, replace the object hit
/// with the next in series (if one exists), and kill the object doing the
/// hitting (bullets don't go through objects in this game).
/// \param p Pointer to the object to be compared against.

void CObjectManager::CollisionDetection(CGameObject* p){ 
  for(auto j=m_stlObjectList.begin(); j!=m_stlObjectList.end(); j++)
    CollisionDetection(p, *j);
} //CollisionDetection

/// Given 2 object pointers, see whether the objects collide. 
/// If a collision is detected, replace the object hit
/// with the next in series (if one exists), and kill the object doing the
/// hitting (bullets don't go through objects in this game).
/// \param i iterator of the object to compare against

void CObjectManager::CollisionDetection(CGameObject* p0, CGameObject* p1)
{ 
  if(p1->m_bVulnerable && distance(p0, p1) < 15.0f){
    p0->m_bIsDead = p1->m_bIsDead = TRUE; //they're dead, Jim
    CreateNextIncarnation(p1); //replace with dead object, if any
  } //if
} //CollisionDetection

/// Collect garbage, that is, remove dead objects from the object list.

void CObjectManager::GarbageCollect(){
  auto i = m_stlObjectList.begin();
  while(i != m_stlObjectList.end()){
    CGameObject* p = *i; //save pointer to object temporarily
    if(p->m_bIsDead){
      i = m_stlObjectList.erase(i); //remove pointer from list
      delete p; //delete object
    } //if
    if(i != m_stlObjectList.end())
      ++i; //next object
  } //while
} //GarbageCollect