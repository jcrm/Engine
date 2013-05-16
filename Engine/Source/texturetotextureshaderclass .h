////////////////////////////////////////////////////////////////////////////////
// Filename: texturetotextureshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURETOTEXTURESHADERCLASS_H_
#define _TEXTURETOTEXTURESHADERCLASS_H_


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
// Class name: TextureToTextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class TextureToTextureShaderClass : public ShaderClass
{
public:

	TextureToTextureShaderClass();
	TextureToTextureShaderClass(const TextureToTextureShaderClass&);
	~TextureToTextureShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);
protected:
	struct MatrixBufferType2
	{
		D3DXMATRIX projection;
	};
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float screenHeight, float screenWidth);
	void RenderShader(ID3D11DeviceContext*, int);
};

#endif