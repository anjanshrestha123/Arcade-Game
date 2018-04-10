/// \file gamerenderer.cpp
/// \brief Direct3D rendering tasks for the game.
/// DirectX stuff that won't change much is hidden away in this file
/// so you won't have to keep looking at it.

#include <algorithm>

#include "gamerenderer.h"
#include "defines.h" 
#include "abort.h"
#include "imagefilenamelist.h"
#include "debug.h"
#include "sprite.h"
#include "object.h"

extern int g_nScreenWidth;
extern int g_nScreenHeight;
extern BOOL g_bWireFrame;
extern CImageFileNameList g_cImageFileName;
extern C3DSprite* g_pPlaneSprite;
extern CGameObject* g_pPlane; 
extern C3DSprite* g_pPlaneSprite2;
extern CGameObject* g_pPlane2;
BOOL KeyboardHandler(WPARAM keystroke);
CGameRenderer::CGameRenderer(): m_bCameraDefaultMode(TRUE){
} //constructor


void CGameRenderer::InitBackground(){
  HRESULT hr;

  //load vertex buffer
  float w = 2.0f*g_nScreenWidth;
  float h = 2.0f*g_nScreenHeight;
  
  //vertex information, first triangle in clockwise order
  BILLBOARDVERTEX pVertexBufferData[6]; 
  pVertexBufferData[0].p = Vector3(w, 0, 0);
  pVertexBufferData[0].tu = 1.0f; pVertexBufferData[0].tv = 0.0f;

  pVertexBufferData[1].p = Vector3(0, 0, 0);
  pVertexBufferData[1].tu = 0.0f; pVertexBufferData[1].tv = 0.0f;

  pVertexBufferData[2].p = Vector3(w, 0, 1500);
  pVertexBufferData[2].tu = 1.0f; pVertexBufferData[2].tv = 1.0f;

  pVertexBufferData[3].p = Vector3(0, 0, 1500);
  pVertexBufferData[3].tu = 0.0f; pVertexBufferData[3].tv = 1.0f;

  pVertexBufferData[4].p = Vector3(w, h, 1500);
  pVertexBufferData[4].tu = 1.0f; pVertexBufferData[4].tv = 0.0f;

  pVertexBufferData[5].p = Vector3(0, h, 1500);
  pVertexBufferData[5].tu = 0.0f; pVertexBufferData[5].tv = 0.0f;
  
  //create vertex buffer for background
  m_pShader = new CShader(2);
    
  m_pShader->AddInputElementDesc(0, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION");
  m_pShader->AddInputElementDesc(12, DXGI_FORMAT_R32G32_FLOAT,  "TEXCOORD");
  m_pShader->VSCreateAndCompile(L"VertexShader.hlsl", "main");
  m_pShader->PSCreateAndCompile(L"PixelShader.hlsl", "main");
    
  // Create constant buffers.
  D3D11_BUFFER_DESC constantBufferDesc = { 0 };
  constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
  constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  constantBufferDesc.CPUAccessFlags = 0;
  constantBufferDesc.MiscFlags = 0;
  constantBufferDesc.StructureByteStride = 0;
    
  m_pDev2->CreateBuffer(&constantBufferDesc, nullptr, &m_pConstantBuffer);
    
  D3D11_BUFFER_DESC VertexBufferDesc;
  VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  VertexBufferDesc.ByteWidth = sizeof(BILLBOARDVERTEX)* 6;
  VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  VertexBufferDesc.CPUAccessFlags = 0;
  VertexBufferDesc.MiscFlags = 0;
  VertexBufferDesc.StructureByteStride = 0;
    
  D3D11_SUBRESOURCE_DATA subresourceData;
  subresourceData.pSysMem = pVertexBufferData;
  subresourceData.SysMemPitch = 0;
  subresourceData.SysMemSlicePitch = 0;
    
  hr = m_pDev2->CreateBuffer(&VertexBufferDesc, &subresourceData, &m_pBackgroundVB);
} //InitBackground

/// Draw the game background.

void CGameRenderer::DrawBackground(){
  UINT nVertexBufferOffset = 0;
  
  UINT nVertexBufferStride = sizeof(BILLBOARDVERTEX);
  m_pDC2->IASetVertexBuffers(0, 1, &m_pBackgroundVB, &nVertexBufferStride, &nVertexBufferOffset);
  m_pDC2->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  m_pShader->SetShaders();

  //draw floor
  if(g_bWireFrame)
    m_pDC2->PSSetShaderResources(0, 1, &m_pWireframeTexture); //set wireframe texture
  else
    m_pDC2->PSSetShaderResources(0, 1, &m_pFloorTexture); //set floor texture
  
  SetWorldMatrix();
  
  ConstantBuffer constantBufferData; ///< Constant buffer data for shader.

  constantBufferData.wvp = CalculateWorldViewProjectionMatrix();
  m_pDC2->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &constantBufferData, 0, 0);
  m_pDC2->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
  m_pDC2->Draw(4, 0);

  //draw backdrop
  if(!g_bWireFrame)
    m_pDC2->PSSetShaderResources(0, 1, &m_pWallTexture);

  constantBufferData.wvp = CalculateWorldViewProjectionMatrix();
  m_pDC2->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &constantBufferData, 0, 0);
  m_pDC2->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
  m_pDC2->Draw(4, 2);
} //DrawBackground
 
/// Load the background and sprite textures.

void CGameRenderer::LoadTextures(){ 
  LoadTexture(m_pWallTexture, g_cImageFileName[0]);
  LoadTexture(m_pFloorTexture, g_cImageFileName[1]);
  LoadTexture(m_pWireframeTexture, g_cImageFileName[2]); //black for wireframe
} //LoadTextures

/// All textures used in the game are released - the release function is kind
/// of like a destructor for DirectX entities, which are COM objects.

void CGameRenderer::Release(){ 
  g_pPlaneSprite->Release();
  g_pPlaneSprite2->Release();

  SAFE_RELEASE(m_pWallTexture);
  SAFE_RELEASE(m_pFloorTexture);
  SAFE_RELEASE(m_pWireframeTexture);
  SAFE_RELEASE(m_pBackgroundVB);

  SAFE_DELETE(m_pShader);
  
  CRenderer::Release();
} //Release

/// Move all objects, then draw them.
/// \return TRUE if it succeeded

void CGameRenderer::ComposeFrame(){
  //prepare to draw
  m_pDC2->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
  float clearColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
  m_pDC2->ClearRenderTargetView(m_pRTV, clearColor);
  m_pDC2->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  //draw
  DrawBackground(); //draw background
  g_pPlane->draw(); //draw plane
  g_pPlane2->draw();
} //ComposeFrame
 
/// Compose a frame of animation and present it to the video card.

void CGameRenderer::ProcessFrame() {


	if (g_pPlane->m_vPos.y != 300.0f )
	{
		g_pPlane->jump();
		
		if (g_pPlane->m_vPos.y > 315.0f)
			g_pPlaneSprite->Load(g_cImageFileName[16]);
		else
			g_pPlaneSprite->Load(g_cImageFileName[3]);
	}

	if (g_pPlane2->m_vPos.y != 300.0f)
	{
		g_pPlane2->jump();
		if (g_pPlane2->m_vPos.y > 315.0f)
        g_pPlaneSprite2->Load(g_cImageFileName[8]);
		else 
			g_pPlaneSprite2->Load(g_cImageFileName[4]);

	}
		
		ComposeFrame();
		m_pSwapChain2->Present(1, 0); //present it

		
} //ProcessFrame

void CGameRenderer::ProcessFrameForOther()
{

	ComposeFrame();
	m_pSwapChain2->Present(4, 0); //present it
}


/// Toggle between eagle-eye camera (camera pulled back far enough to see
/// backdrop) and the normal game camera.

void CGameRenderer::FlipCameraMode(){
  m_bCameraDefaultMode = !m_bCameraDefaultMode; 
  
  if(m_bCameraDefaultMode)
    SetViewMatrix(Vector3(1024, 384, -350), Vector3(1024, 384, 1000));
  else SetViewMatrix(Vector3(1024, 600, -2000), Vector3(1024, 600, 1000));
} //FlipCameraMode
