////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "textureshaderclass.h"
#include "rendertextureclass.h"
#include "modelclass.h"
#include "verticalblurshaderclass.h"
#include "horizontalblurshaderclass.h"
#include "orthowindowclass.h"
#include "convolutionshaderclass.h"
#include "ShaderClass.h"


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render(float);

	bool RenderSceneToTexture(RenderTextureClass*, float);
	bool Render2DTextureScene(RenderTextureClass*);
	bool RenderTexture(ShaderClass *mShader, RenderTextureClass *mReadTexture, RenderTextureClass *mWriteTexture, OrthoWindowClass *mWindow);
	
	bool DownSampleTexture();
	bool RenderHorizontalBlurToTexture(RenderTextureClass *mTempTexture);
	bool RenderVerticalBlurToTexture(RenderTextureClass *mTempTexture);
	bool UpSampleTexture();
	bool RenderConvolutionToTexture(RenderTextureClass *mTempTexture);
private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model[4];
	RenderTextureClass *m_RenderTexture, *m_DownSampleTexure, *m_HorizontalBlurTexture, *m_VerticalBlurTexture, *m_UpSampleTexure, *m_ConvolutionTexture;
	OrthoWindowClass *m_SmallWindow, *m_FullScreenWindow;
	TextureShaderClass* m_TextureShader;
	ConvolutionShaderClass* m_ConvolutionShader;
	VerticalBlurShaderClass* m_VerticalBlurShader;
	HorizontalBlurShaderClass* m_HorizontalBlurShader;
	
};

#endif