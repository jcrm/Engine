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
void ShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}
bool ShaderClass::SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*){
	return false;
}
void ShaderClass::RenderShader(ID3D11DeviceContext*, int){

}
