////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MERGETEXTURESHADERCLASS_H_
#define _MERGETEXTURESHADERCLASS_H_


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
// Class name: TextureShaderClass
////////////////////////////////////////////////////////////////////////////////
class MergeTextureShaderClass : public ShaderClass
{
public:
	struct MergeBufferType
	{
		float strength;
		D3DXVECTOR3 padding;
	};
	MergeTextureShaderClass();
	MergeTextureShaderClass(const MergeTextureShaderClass&);
	~MergeTextureShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2);
protected:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2);
	void RenderShader(ID3D11DeviceContext*, int);
	ID3D11Buffer* mMergeBuffer;
};

#endif