#include "ShaderClass.h"


ShaderClass::ShaderClass(void): m_vertexShader(0), m_pixelShader(0), m_layout(0), 
	m_sampleState(0), m_matrixBuffer(0)
{
}


ShaderClass::~ShaderClass(void)
{
}
bool ShaderClass::Initialize(ID3D11Device*, HWND){
	return false;
}
void ShaderClass::Shutdown(){

}
bool ShaderClass::Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, float, float){
	return false;
}
bool ShaderClass::Render(ID3D11DeviceContext*, int, D3DXMATRIX, ID3D11ShaderResourceView*, float, float){
	return false;
}
bool ShaderClass::InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*){
	return false;
}
void ShaderClass::ShutdownShader(){

}
void ShaderClass::OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*){

}
bool ShaderClass::SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*){
	return false;
}
void ShaderClass::RenderShader(ID3D11DeviceContext*, int){

}
