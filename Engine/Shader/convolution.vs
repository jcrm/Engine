////////////////////////////////////////////////////////////////////////////////
// Filename: convolution.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer ScreenSizeBuffer
{
	float screenWidth;
	float screenHeight;
	float2 padding;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float2 texCoord1 : TEXCOORD1;
	float2 texCoord2 : TEXCOORD2;
	float2 texCoord3 : TEXCOORD3;
	float2 texCoord4 : TEXCOORD4;
	float2 texCoord5 : TEXCOORD5;
	float2 texCoord6 : TEXCOORD6;
	float2 texCoord7 : TEXCOORD7;
	float2 texCoord8 : TEXCOORD8;
	float2 texCoord9 : TEXCOORD9;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ConvolutionVertexShader(VertexInputType input)
{
    PixelInputType output;
	float texelSizeW;
	float texelSizeH;


	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    output.position = input.position;
	output.position.z = 0;

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	// Determine the floating point size of a texel for a screen with this specific width.
	texelSizeW = 1.0f / screenWidth;
	texelSizeH = 1.0f / screenHeight;

	// Create UV coordinates for the pixel and its four horizontal neighbors on either side.
	output.texCoord1 = input.tex + float2(texelSizeW * -1.0f, texelSizeH * -1.0f);
	output.texCoord2 = input.tex + float2(texelSizeW *  0.0f, texelSizeH * -1.0f);
	output.texCoord3 = input.tex + float2(texelSizeW *  1.0f, texelSizeH * -1.0f);
	output.texCoord4 = input.tex + float2(texelSizeW * -1.0f, 0.0f);
	output.texCoord5 = input.tex + float2(texelSizeW *  0.0f, 0.0f);
	output.texCoord6 = input.tex + float2(texelSizeW *  1.0f, 0.0f);
	output.texCoord7 = input.tex + float2(texelSizeW * -1.0f, texelSizeH *  1.0f);
	output.texCoord8 = input.tex + float2(texelSizeW *  0.0f, texelSizeH *  1.0f);
	output.texCoord9 = input.tex + float2(texelSizeW *  1.0f, texelSizeH *  1.0f);

    return output;
}