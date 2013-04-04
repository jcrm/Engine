////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fluidclass.h"
#include <cmath>


FluidClass::FluidClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_heightMap = 0;
	mWave = 1.0f;
	mScale = 10.0f;
	mWaveTime = 1/(2 * sqrt(2.0f));
	m_terrainGeneratedToggle = false;
}


FluidClass::FluidClass(const FluidClass& other)
{
}


FluidClass::~FluidClass()
{
}

bool FluidClass::InitializeTerrain(ID3D11Device* device, int terrainWidth, int terrainHeight)
{
	bool result;

	// Save the dimensions of the terrain.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if(!m_heightMap)
	{
		return false;
	}

	ResetWater();
	//even though we are generating a flat terrain, we still need to normalize it. 
	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if(!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}
bool FluidClass::Initialize(ID3D11Device* device, char* heightMapFilename)
{
	bool result;


	// Load in the height map for the terrain.
	result = LoadHeightMap(heightMapFilename);
	if(!result)
	{
		return false;
	}

	// Normalize the height of the height map.
	NormalizeHeightMap();

	// Calculate the normal's for the terrain data.
	result = CalculateNormals();
	if(!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}


void FluidClass::Shutdown()
{

	// Release the vertex and index buffer.
	ReleaseBuffers();

	// Release the height map data.
	ShutdownHeightMap();

	return;
}


void FluidClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

bool FluidClass::GenerateHeightMap( ID3D11Device* device )
{

	bool result;
	//the toggle is just a bool that I use to make sure this is only called ONCE when you press a key
	//until you release the key and start again. We don t want to be generating the terrain 500
	//times per second. 

	ReleaseBuffers();
	DiminishWater();
	result = CalculateNormals();
	if(!result){
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result){
		return false;
	}
	return true;
}
bool FluidClass::LoadHeightMap(char* filename)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;


	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_terrainWidth = bitmapInfoHeader.biWidth;
	m_terrainHeight = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_terrainWidth * m_terrainHeight * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if(!m_heightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k=0;

	// Read the image data into the height map.
	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			height = bitmapImage[k];
			
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

			k+=3;
		}
	}

	// Release the bitmap image data.
	delete [] bitmapImage;
	bitmapImage = 0;

	return true;
}


void FluidClass::NormalizeHeightMap()
{
	int i, j;


	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			m_heightMap[(m_terrainHeight * j) + i].y /= 15.0f;
		}
	}

	return;
}


bool FluidClass::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	VectorType* normals;


	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new VectorType[(m_terrainHeight-1) * (m_terrainWidth-1)];
	if(!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for(j=0; j<(m_terrainHeight-1); j++){
		for(i=0; i<(m_terrainWidth-1); i++){
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + (i+1);
			index3 = ((j+1) * m_terrainHeight) + i;

			// Get three vertices's from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;
		
			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;
		
			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight-1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices's and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for(j=0; j<m_terrainHeight; j++)
	{
		for(i=0; i<m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if(((i-1) >= 0) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if((i < (m_terrainWidth-1)) && ((j-1) >= 0))
			{
				index = ((j-1) * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if(((i-1) >= 0) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if((i < (m_terrainWidth-1)) && (j < (m_terrainHeight-1)))
			{
				index = (j * (m_terrainHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}
			
			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));
			
			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete [] normals;
	normals = 0;

	return true;
}


void FluidClass::ShutdownHeightMap()
{
	if(m_heightMap)
	{
		delete [] m_heightMap;
		m_heightMap = 0;
	}

	return;
}


bool FluidClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index1, index2, index3, index4;


	// Calculate the number of vertices's in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;

	// Load the vertex and index array with the terrain data.
	for(j=0; j<(m_terrainHeight-1); j++){
		for(i=0; i<(m_terrainWidth-1); i++){
			index1 = (m_terrainHeight * j) + i;          // Bottom left.
			index2 = (m_terrainHeight * j) + (i+1);      // Bottom right.
			index3 = (m_terrainHeight * (j+1)) + i;      // Upper left.
			index4 = (m_terrainHeight * (j+1)) + (i+1);  // Upper right.

			if((i%2 !=0 && j%2 ==0) || (i%2 ==0 && j%2 != 0)){
				// Upper left.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				indices[index] = index;
				index++;

				// Upper right.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom right.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				indices[index] = index;
				index++;

				// Bottom right.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				indices[index] = index;
				index++;

				// Upper left.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				indices[index] = index;
				index++;

			}else{
				// Upper left.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				indices[index] = index;
				index++;

				// Upper right.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				indices[index] = index;
				index++;

				// Bottom left.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				indices[index] = index;
				index++;

				// Upper right.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				indices[index] = index;
				index++;

				// Bottom right.
				vertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				vertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				indices[index] = index;
				index++;
			}
		}
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	
	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


void FluidClass::ReleaseBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void FluidClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
void FluidClass::GenerateRandomHeightMap()
{
	//the toggle is just a bool that I use to make sure this is only called ONCE when you press a key
	//until you release the key and start again. We don t want to be generating the terrain 500
	//times per second. 
	int index;
	float height = (float(rand()%200)/10)-10;

	//loop through the terrain and set the heights how we want. This is where we generate the terrain
	//in this case I will run a sin-wave through the terrain in one axis.

	for(int j=0; j<m_terrainHeight; j++){
		for(int i=0; i<m_terrainWidth; i++){
			float height = (float(rand()%200)/10)-10;
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = height;
			m_heightMap[index].z = (float)j;
		}
	}
}
void FluidClass::Water(){
	static const float WaveAndTime = mWave*mWave*mWaveTime*mWaveTime;
	static const float CurrentWaveAndTime = 2-(4*WaveAndTime);
	for(int i = 1; i<m_terrainWidth-1; i++){
		for(int j = 1; j<m_terrainWidth-1; j++){
			int index = (m_terrainWidth * j) + i;
			float newVal = CurrentWaveAndTime*m_heightMap[index].y;
			newVal-= m_heightMap[index].prevY;
			float sum = ValuesAroundPoint(i,j);
			newVal+= WaveAndTime*sum;
			m_heightMap[index].nextY = newVal;
		}
	}
	UpdateWaterValues();
}
void FluidClass::UpdateWaterValues(){
	for(int i = 0; i <m_terrainWidth; i++){
		for(int j = 0; j <m_terrainWidth; j++){
			int index = (m_terrainWidth * j) + i;
			m_heightMap[index].prevY = m_heightMap[index].y;
			m_heightMap[index].y = m_heightMap[index].nextY;
		}
	}
}
float FluidClass::ValuesAroundPoint(int x, int z){
	float sum = m_heightMap[(m_terrainWidth * (z-1) ) + x].y;
	sum += m_heightMap[(m_terrainWidth * (z+1) ) + x].y;
	sum += m_heightMap[(m_terrainWidth * z) + x-1].y;
	sum += m_heightMap[(m_terrainWidth * z) + x+1].y;
	return sum;
}
/*generate texture and do calculations*/
/*then generate new texture based upon old data*/

void FluidClass::DiminishWater(){
	static const float UDeltaTime = mWaveTime * 0.05f;
	static const float WaveAndTime = mWave*mWave*mWaveTime*mWaveTime;
	static const float CurrentWaveAndTime = ((4-(8*WaveAndTime))/(UDeltaTime+2));
	static const float PrevTime = ((UDeltaTime-2)/(UDeltaTime+2));
	for(int i = 1; i<m_terrainWidth-1; i++){
		for(int j = 1; j<m_terrainWidth-1; j++){
			int index = (m_terrainWidth * j) + i;
			if(m_heightMap[index].border == false){
				float newVal = CurrentWaveAndTime*m_heightMap[index].y;
				newVal+= PrevTime*m_heightMap[index].prevY;
				float sum = ValuesAroundPoint(i,j);
				newVal+= ((2*WaveAndTime)/(UDeltaTime+2))*sum;
				m_heightMap[index].nextY = newVal;
			}
		}
	}
	UpdateWaterValues();
}
void FluidClass::AddWater(int x, int z, float height){
	m_heightMap[(m_terrainWidth * x) + z].y+=(height/mScale);
}
bool FluidClass::CheckBorder(int x, int z){
	int numOfCircle = 3;
	float CircleX[3] = {64,84,31};
	float CircleZ[3] = {64,28,90};
	float CircleRadiusSq[3] = {40,20,30};
	bool bound = true;
	for(int i = 0; i < numOfCircle; i++){
		if(((CircleX[i]-x)*(CircleX[i]-x))+((CircleZ[i]-z)*(CircleZ[i]-z))< CircleRadiusSq[i]*CircleRadiusSq[i]){
			bound = false;
		}
	}
	return bound;
}
void FluidClass::ResetWater(){
	int index;
	float height = 0.0;
	// Initialize the data in the height map (flat).
	for(int j=0; j<m_terrainHeight; j++){
		for(int i=0; i<m_terrainWidth; i++){			
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = ((float)i)/mScale-(float(m_terrainWidth)/mScale);
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = ((float)j)/10;
			m_heightMap[index].nextY = 0.0f;
			m_heightMap[index].prevY = 0.0f;
			m_heightMap[index].border = CheckBorder(i,j);
			/*
			if(i<(m_terrainWidth/4) || i>3*(m_terrainWidth/4) || j<(m_terrainWidth/4) || j>3*(m_terrainWidth/4)){
				m_heightMap[index].border = true;
			}else{
				m_heightMap[index].border = false;
			}*/
		}
	}
	for(int j=m_terrainHeight/2-2; j<m_terrainHeight/2+2; j++){
		for(int i=m_terrainWidth/2-2; i<m_terrainWidth/2+2; i++){			
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].y = 2.0f/mScale;

		}
	}
	for(int j=m_terrainHeight/2-10; j<m_terrainHeight/2-5; j++){
		for(int i=m_terrainWidth/2-10; i<m_terrainWidth/2-5; i++){			
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].y = -5.0f/mScale;

		}
	}
	for(int j=m_terrainHeight/2+20; j<m_terrainHeight/2+25; j++){
		for(int i=m_terrainWidth/2+20; i<m_terrainWidth/2+25; i++){			
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].y = 1.5f/mScale;

		}
	}
}