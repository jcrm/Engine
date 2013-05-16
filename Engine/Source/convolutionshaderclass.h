////////////////////////////////////////////////////////////////////////////////
// Filename: convolutionshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVOLUTIONSHADERCLASS_H_
#define _CONVOLUTIONSHADERCLASS_H_

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
// Class name: ConvolutionhaderClass
////////////////////////////////////////////////////////////////////////////////
class ConvolutionShaderClass : public ShaderClass
{
private:
	struct ScreenSizeBufferType{
		float screenHeight;
		float screenWidth;
		D3DXVECTOR2 padding;
	};
	struct ConvolutuionBufferType{
		float weight0;
		float weight1;
		float weight2;
		float weight3;

		float weight4;
		float weight5;
		float weight6;
		float weight7;

		float weight8;
		D3DXVECTOR3 padding;
		//add padding to make multiple of 16 bytes
	};

public:
	ConvolutionShaderClass();
	ConvolutionShaderClass(const ConvolutionShaderClass&);
	~ConvolutionShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float screenHeight, float screenWidth);
protected:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, float screenHeight, float screenWidth);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11Buffer* m_screenSizeBuffer;
	ID3D11Buffer* m_convolutionBuffer;
};

#endif