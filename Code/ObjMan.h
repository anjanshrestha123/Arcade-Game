/// \file objman.h
/// \brief Interface for the object manager class CObjectManager.

#pragma once

#include <list>
#include <string>
#include <unordered_map>

#include "object.h"

/// \brief The object manager. 
///
/// The object manager is responsible for the care and feeding of
/// game objects. Objects can be named on creation so that they
/// can be accessed later - this is needed in particular for the player
/// object or objects.

class CObjectManager{
  private:
    list<CGameObject*> m_stlObjectList; ///< List of game objects.
    unordered_map<string, CGameObject*> m_stlNameToObject; ///< Map names to objects.
    unordered_map<string, ObjectType> m_stlNameToObjectType; ///< Map names to object types.
    
    int m_nLastGunFireTime; ///< Time gun was last fired.

    //creation functions
    CGameObject* createObject(const char* obj, const char* name, const Vector3& s); ///< Create new object by name.
    
    //distance functions
    float distance(CGameObject *g0, CGameObject *g1); ///< Distance between objects.

    //collision detection
    void CollisionDetection(); ///< Process all collisions.
    void CollisionDetection(CGameObject* i); ///< Process collisions of all with one object.
    void CollisionDetection(CGameObject* i, CGameObject* j); ///< Process collisions of 2 objects.

    //managing dead objects
    void cull(); ///< Cull dead objects
    void CreateNextIncarnation(CGameObject* object); ///< Replace object by next in series.
    void GarbageCollect(); ///< Collect dead objects from object list.

  public:
    CObjectManager(); ///< Constructor.
    ~CObjectManager(); ///< Destructor.

    CGameObject* createObject(ObjectType obj, const char* name, const Vector3& s, const Vector3& v); ///< Create new object.

    void move(); ///< Move all objects.
    void draw(); ///< Draw all objects.

    CGameObject* GetObjectByName(const char* name); ///< Get pointer to object by name.
    void InsertObjectType(const char* objname, ObjectType t); ///< Map name string to object type enumeration.
    ObjectType GetObjectType(const char* name); ///< Get object type corresponding to name string.
    
    void FireGun(char* name); ///< Fire a gun from named object.
}; //CObjectManager