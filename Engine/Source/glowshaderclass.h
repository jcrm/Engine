////////////////////////////////////////////////////////////////////////////////
// Filename: glowshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GLOWSHADERCLASS_H_
#define _GLOWSHADERCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include "ShaderClass.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Class name: GlowShaderClass Inherits ShaderClass
////////////////////////////////////////////////////////////////////////////////
class GlowShaderClass : public ShaderClass
{
public:
	GlowShaderClass();
	GlowShaderClass(const GlowShaderClass&);
	~GlowShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	//first render class is the one the shader uses
	//second is interface that application class uses
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ID3D11ShaderResourceView* texture);
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float screenHeight, float screenWidth);
protected:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture);
	void RenderShader(ID3D11DeviceContext*, int);
};

#endif