#ifndef _ShaderClass_h_
#define _ShaderClass_h_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// Class name: ShaderClass
////////////////////////////////////////////////////////////////////////////////
class ShaderClass
{
protected:
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

public:
	ShaderClass(void);
	ShaderClass(const ShaderClass&);
	virtual ~ShaderClass(void);


	virtual bool Initialize(ID3D11Device*, HWND);
	virtual void Shutdown();
	virtual bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);
	virtual	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, ID3D11ShaderResourceView*, float, float);
protected:
	virtual bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	virtual void ShutdownShader();
	virtual void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	virtual bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*);
	virtual void RenderShader(ID3D11DeviceContext*, int);

protected:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;

};

#endif // _ShaderClass_h_

