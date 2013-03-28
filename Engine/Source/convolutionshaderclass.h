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
// Class name: VerticalBlurShaderClass
////////////////////////////////////////////////////////////////////////////////
class ConvolutionShaderClass : public ShaderClass
{
private:
	struct ScreenSizeBufferType
	{
		float screenHeight;
		float screenWidth;
		D3DXVECTOR2 padding;
	};
	struct ConvolutuionBufferType
	{
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
		//add padding
	};

public:
	ConvolutionShaderClass();
	ConvolutionShaderClass(const ConvolutionShaderClass&);
	~ConvolutionShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);

protected:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11Buffer* m_screenSizeBuffer;
	ID3D11Buffer* m_convolutionBuffer;
};

#endif