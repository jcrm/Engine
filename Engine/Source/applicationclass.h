////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "inputclass.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "terrainclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "fpsclass.h"
#include "cpuclass.h"
#include "fontshaderclass.h"
#include "textclass.h"
#include "terrainshaderclass.h"
#include "lightclass.h"
#include "fluidclass.h"
#include "fluidshaderclass.h"
#include "textureshaderclass.h"
#include "rendertextureclass.h"
#include "modelclass.h"
#include "verticalblurshaderclass.h"
#include "horizontalblurshaderclass.h"
#include "orthowindowclass.h"
#include "convolutionshaderclass.h"
#include "ShaderClass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ApplicationClass
////////////////////////////////////////////////////////////////////////////////
class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool HandleInput(float);
	bool RenderGraphics();
	bool Render(float rotation);
	bool RenderSceneToTexture(RenderTextureClass* mWrite, float rotation);
	bool RenderTexture(ShaderClass *mShader, RenderTextureClass *mReadTexture, RenderTextureClass *mWriteTexture, OrthoWindowClass *mWindow);
	bool Render2DTextureScene(RenderTextureClass* mRead);
private:
	InputClass* m_Input;
	D3DClass* m_Direct3D;
	CameraClass* m_Camera;
	TerrainClass* m_Terrain;
	ModelClass* m_Model[4];
	FluidClass* mFluid;
	TimerClass* m_Timer;
	PositionClass* m_Position;
	FpsClass* m_Fps;
	CpuClass* m_Cpu;
	FontShaderClass* m_FontShader;
	TextClass* m_Text;
	TerrainShaderClass* m_TerrainShader;
	FluidShaderClass* mFluidShader;
	LightClass* m_Light;
	RenderTextureClass *m_RenderTexture, *m_DownSampleTexure, *m_HorizontalBlurTexture, *m_VerticalBlurTexture, *m_UpSampleTexure, *m_ConvolutionTexture;
	OrthoWindowClass *m_SmallWindow, *m_FullScreenWindow;
	TextureShaderClass* m_TextureShader;
	ConvolutionShaderClass* m_ConvolutionShader;
	VerticalBlurShaderClass* m_VerticalBlurShader;
	HorizontalBlurShaderClass* m_HorizontalBlurShader;

};

#endif