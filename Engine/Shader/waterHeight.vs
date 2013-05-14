////////////////////////////////////////////////////////////////////////////////
// Filename: terrain.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
};

struct PixelInputType
{
	float old;
	float current;
	float new;
	float padding;
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
	float height = output.position.y;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);
	if(height<=-1){
		output.hColor = lerp(float4(0.0,0.0,0.0,1.0),float4(0.0,0.0,0.0,0.0),height/2);
	}else{
		output.hColor = lerp(float4(0.32,0.2,0.09,1.0),float4(1.0,1.0,1.0,0.0),height/20);
	}
	output.hColor = lerp(float4(0.0,0.0,0.1,1.0),float4(0.0,0.0,1.0,0.0),height/30);
    return output;
}