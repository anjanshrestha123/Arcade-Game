/// \file object.cpp
/// \brief Code for the game object class CGameObject.

#include "object.h"

#include "defines.h" 
#include "timer.h" 
#include "debug.h"
#include "imagefilenamelist.h"
#include "sound.h"
#include "GameRenderer.h"

extern CSoundManager* g_pSoundManager;
extern CTimer g_cTimer;
extern int g_nScreenWidth;
extern int g_nScreenHeight;
extern XMLElement* g_xmlSettings;
BOOL isOnPlatformOrGround(float x, float& y);
BOOL isUnderPlatform(float x, float& y);
float dy = 0;
extern C3DSprite* g_pPlaneSprite; ///< Pointer to the plane sprite.
extern CGameObject* g_pPlane; ///< Pointer to the plane object.
extern C3DSprite* g_pPlaneSprite2; ///< Pointer to the plane sprite.
extern CGameObject* g_pPlane2; ///< Pointer to the plane object.
extern CImageFileNameList g_cImageFileName;


/// Initialize a game object. Gets object-dependent settings from g_xmlSettings
/// from the "object" tag that has the same "name" attribute as parameter name.
/// Assumes that the sprite manager has loaded the sprites already.
/// \param s Initial location of object 
/// \param v Initial velocity
/// \param sprite Pointer to the sprite for this object.

CGameObject::CGameObject(const Vector3& s, const Vector3& v, C3DSprite *sprite){ 
  m_nLastMoveTime = 0; //time
  m_vPos =s; //location
  m_vVelocity = v; //velocity
  m_pSprite = sprite; //sprite pointer
} //constructor

/// Draw the current sprite frame at the current position, then
/// compute which frame is to be drawn next time.

void CGameObject::draw(){ //draw
  if(m_pSprite) //if there is a sprite
    m_pSprite->Draw(m_vPos); //draw in correct place
} //draw
 
/// The distance that an object moves depends on its speed, 
/// and the amount of time since it last moved.

void CGameObject::moveLeft(){ //move object
	m_vPos.x -= 5;

	
	 if (m_vPos.x < 338.0f)            //limits on the edge
	{
		m_vPos.x = 338.0f;
	}

	 if (g_pPlane->m_vPos.x < g_pPlane2->m_vPos.x + 35.0f)      // prevents passing of left player through another player
	 {
		 g_pPlane->m_vPos.x = g_pPlane2->m_vPos.x + 35.0f;
	 }




} //move

void CGameObject::moveRight() { //move object
	m_vPos.x += 5;
	
	if (m_vPos.x > 688.0f)   //limits on the edge
	{
		m_vPos.x = 688.0f;
	}

	if (g_pPlane2->m_vPos.x > g_pPlane->m_vPos.x - 35.0f)      //prevents passing of right player through another player
	{
		g_pPlane2->m_vPos.x = g_pPlane->m_vPos.x - 35.0f;
	}

} //move


void CGameObject::leftpunch() {
	g_pSoundManager->play(0);

	
}

void CGameObject::leftkick() {
	g_pSoundManager->play(1);
	
}

void CGameObject:: rightpunch() {
	g_pSoundManager->play(0);

	
}

void CGameObject::rightkick() {
	g_pSoundManager->play(1);

}

void CGameObject::jump() {

	if (m_vPos.y <= 350.0f && m_vPos.y >= 300.0f)
	{
		dy += 1.0f;
		m_vPos.y += dy;
	}
	if (m_vPos.y > 350)
	{
		dy -= 1.0f;
		m_vPos.y += dy;
	}

} //jump
