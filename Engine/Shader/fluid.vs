////////////////////////////////////////////////////////////////////////////////
// Filename: fluid.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4  hColor : HCOLOR;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TerrainVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
	//take the position of the vertex before doing vie and projection matrix calculations
	float height = output.position.y;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);
	//set the blue to be between two versions of blue
	output.hColor = lerp(float4(0.0,0.0,0.1,1.0),float4(0.0,0.0,1.0,0.0),height/30);
    return output;
}