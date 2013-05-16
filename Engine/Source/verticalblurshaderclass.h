////////////////////////////////////////////////////////////////////////////////
// Filename: verticalblurshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _VERTICALBLURSHADERCLASS_H_
#define _VERTICALBLURSHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;
#include "ShaderClass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: VerticalBlurShaderClass
////////////////////////////////////////////////////////////////////////////////
class VerticalBlurShaderClass : public ShaderClass
{
private:
	struct ScreenSizeBufferType
	{
		float screenHeight;
		D3DXVECTOR3 padding;
	};

public:
	VerticalBlurShaderClass();
	VerticalBlurShaderClass(const VerticalBlurShaderClass&);
	~VerticalBlurShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, ID3D11ShaderResourceView*, float);
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float screenHeight, float screenWidth);
private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, ID3D11ShaderResourceView*, float);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_screenSizeBuffer;
};

#endif