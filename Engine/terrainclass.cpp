////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "terrainclass.h"
#include <cmath>
#include "Generation.h"


TerrainClass::TerrainClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_heightMap = 0;
	m_terrainGeneratedToggle = false;
}


TerrainClass::TerrainClass(const TerrainClass& other)
{
}


TerrainClass::~TerrainClass()
{
}

bool TerrainClass::InitializeTerrain(ID3D11Device* device, int terrainWidth, int terrainHeight)
{
	int index;
	float height = 0.0;
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

	// Initialize the data in the height map (flat).
	for(int j=0; j<m_terrainHeight; j++)
	{
		for(int i=0; i<m_terrainWidth; i++)
		{			
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

		}
	}


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
bool TerrainClass::Initialize(ID3D11Device* device, char* heightMapFilename)
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


void TerrainClass::Shutdown()
{
	// Release the vertex and index buffer.
	ShutdownBuffers();

	// Release the height map data.
	ShutdownHeightMap();

	return;
}


void TerrainClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


int TerrainClass::GetIndexCount()
{
	return m_indexCount;
}

bool TerrainClass::GenerateHeightMap(ID3D11Device* device, bool keydown)
{

	bool result;
	//the toggle is just a bool that I use to make sure this is only called ONCE when you press a key
	//until you release the key and start again. We don t want to be generating the terrain 500
	//times per second. 
	if(keydown&&(!m_terrainGeneratedToggle))
	{
		int index;
		float height = 0.0;
		

		
		for(int j=0; j<m_terrainHeight; j++)
		{
			for(int i=0; i<m_terrainWidth; i++)
			{			
				index = (m_terrainHeight * j) + i;

				m_heightMap[index].x = (float)i;
				m_heightMap[index].y=  0.0f;
				m_heightMap[index].z = (float)j;
			}
		}
		/*int i = 0;
		while(i++ < 15){
			Particle(8320);
		}*/
		for (int i = 0; i <15; i++){
			terrainIterateParticleDeposition(1000);
		}
		//MPD();
		//bool generate = Generation::MidPointDisplacement(m_heightMap,m_terrainWidth-1,rand()%RAND_MAX,5.0f,0.1f);
		/*
		//GenerateRandomHeightMap();

		//loop through the terrain and set the heights how we want. This is where we generate the terrain
		//in this case I will run a sin-wave through the terrain in one axis.
		float sinValue = (rand()%12)+1;
		float cosValue = (((float(rand()%200))/10)-10);
		float sinMulti = (((float(rand()%100))/10)-5);
		float cosMulti = (((float(rand()%50))/10)-2.5);
		if(cosValue == 0)	cosValue = 1;
 		for(int j=0; j<m_terrainHeight; j++)
		{
			for(int i=0; i<m_terrainWidth; i++)
			{			
				index = (m_terrainHeight * j) + i;

				m_heightMap[index].x = (float)i;
				m_heightMap[index].y+= (float)((sin((float)i/(m_terrainWidth/sinValue))*sinMulti) + (cos((float)j/cosValue)*cosMulti)); //magic numbers ahoy, just to ramp up the height of the sin function so its visible.
				m_heightMap[index].z = (float)j;
			}
		}
		
		/*
		for(int i=0; i<m_terrainWidth; i++)
		{	
			cosValue = (((float(rand()%200))/10)-10);
			for(int j=0; j<m_terrainHeight; j++)
			{	
				index = (m_terrainWidth * i) + j;

				m_heightMap[index].x = (float)j;
				m_heightMap[index].y+= (cos((float)j/cosValue)*cosMulti); //magic numbers ahoy, just to ramp up the height of the sin function so its visible.
				m_heightMap[index].z = (float)i;
			}
		}*/

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

		m_terrainGeneratedToggle = true;
	}
	else
	{
		m_terrainGeneratedToggle = false;
	}

	


	return true;
}
bool TerrainClass::LoadHeightMap(char* filename)
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


void TerrainClass::NormalizeHeightMap()
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


bool TerrainClass::CalculateNormals()
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
	for(j=0; j<(m_terrainHeight-1); j++)
	{
		for(i=0; i<(m_terrainWidth-1); i++)
		{
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


void TerrainClass::ShutdownHeightMap()
{
	if(m_heightMap)
	{
		delete [] m_heightMap;
		m_heightMap = 0;
	}

	return;
}


bool TerrainClass::InitializeBuffers(ID3D11Device* device)
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


void TerrainClass::ShutdownBuffers()
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


void TerrainClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
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
void TerrainClass::GenerateRandomHeightMap()
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
float RandF(float min, float max){
	float randFloat  = min + float(rand()/ (float(RAND_MAX) / (max-min)));
	return randFloat;
}
static float randnum (float min, float max)
{
	int r;
	float	x;

	r = rand();
	x = (float)(r & 0x7fff) /
		(float)0x7fff;
	return (x * (max - min) + min);
} 
void TerrainClass::MidPoint(float ranMax){
	float h = 5.0f;
	//side length is distance of a single square side
	//or distance of diagonal in diamond
	for(int sideLength = m_terrainWidth-1; sideLength >= 2; sideLength /=2, h/= 2.0){
		//half the length of the side of a square
		//or distance from diamond center to one corner
		//(just to make calcs below a little clearer)
		int halfSide = sideLength/2;

		//generate the new square values
		for(int x=0;x<m_terrainWidth-1;x+=sideLength){
			for(int y=0;y<m_terrainWidth-1;y+=sideLength){
				//x, y is upper left corner of square
				//calculate average of existing corners
				int index[4] = {(m_terrainHeight * y) + x,								//top left
								(m_terrainHeight * y) + x + sideLength,					//top right
								(m_terrainHeight * (y+sideLength)) + x,					//lower left
								(m_terrainHeight * (y+sideLength)) + x+sideLength};		//lower right

				float avg = m_heightMap[index[0]].y + m_heightMap[index[1]].y +
							m_heightMap[index[2]].y + m_heightMap[index[3]].y;
				avg /= 4.0;

				m_heightMap[(m_terrainHeight * (y+halfSide)) + x+halfSide].y = avg + (randnum(0,0.4)) -h;
				//center is average plus random offset
			}
		}

		//generate the diamond values
		//since the diamonds are staggered we only move x
		//by half side
		//NOTE: if the data shouldn't wrap then x < DATA_SIZE
		//to generate the far edge values
		for(int x=0;x<m_terrainWidth-1;x+=halfSide){
			//and y is x offset by half a side, but moved by
			//the full side length
			//NOTE: if the data shouldn't wrap then y < DATA_SIZE
			//to generate the far edge values
			for(int y=(x+halfSide)%sideLength;y<m_terrainWidth-1;y+=sideLength){
				//x, y is center of diamond
				//note we must use mod  and add DATA_SIZE for subtraction 
				//so that we can wrap around the array to find the corners

				int index[4] = {(m_terrainHeight * y) +((x -halfSide + m_terrainHeight-1)%(m_terrainHeight-1)),		//left of centre
								(m_terrainHeight * y) + (x+halfSide)%(m_terrainHeight-1),							//right of centre
								(m_terrainHeight * ((y+halfSide)%(m_terrainHeight-1))) + x,							//below centre
								(m_terrainHeight * ((y -halfSide + m_terrainHeight-1)%(m_terrainHeight-1))) + x};	//above centre

				float avg = m_heightMap[index[0]].y + m_heightMap[index[1]].y +
					m_heightMap[index[2]].y + m_heightMap[index[3]].y;
				avg /= 4.0;

				//new value = average plus random offset
				//We calculate random value in range of 2h
				//and then subtract h so the end value is
				//in the range (-h, +h)
				//update value for center of diamond
				m_heightMap[(m_terrainHeight * y) + x].y = avg + (randnum(0,4)) -h;

				//wrap values on the edges, remove
				//this and adjust loop condition above
				//for non-wrapping values.
				if(x == 0) {
					m_heightMap[(m_terrainHeight * m_terrainHeight)-1 + x].y = avg;
				}
				if(y == 0){
					m_heightMap[(m_terrainHeight * y) + m_terrainHeight-1].y = avg;
				}
			}
		}
	}
}
void TerrainClass::Particle(int index){
	static float size = 10.0f;
	if(m_heightMap[index].y != 0){
		size -= 0.05f;
		switch(rand()%4){
		case 0: Particle(index+m_terrainHeight); break;
		case 1:	Particle(index-m_terrainHeight); break;
		case 2: Particle(index+1); break;
		case 3: Particle(index-1); break;
		}
	}else{
		m_heightMap[index].y += size;
	}
}
int createIndex(int size, int i, int j){
	return (size * i) + j;
}
signed char scrand(signed char r = 4) {
	return (-r + 2 * (rand() % r)); 
}
signed char** mdp(signed char** base, unsigned base_n, signed char r) {
	size_t n = (2 * base_n) - 1;

	signed char** map = new signed char*[n];
	for (unsigned i = 0; i < n; ++i) map[i] = new signed char[n];

	// Resize
	// 1 0 1
	// 0 0 0
	// 1 0 1
	for (size_t i = 0; i < n; i += 2) {
		for (size_t j = !(i % 2 == 0); j < n; j += 2) {
			map[i][j] = base[i / 2][j / 2];
		}
	}

	// Diamond algorithm
	// 0 0 0
	// 0 X 0
	// 0 0 0
	for (size_t i = 1; i < n; i += 2) {
		for (size_t j = 1; j < n; j += 2) {
			signed char& map_ij = map[i][j];

			signed char a = map[i - 1][j - 1];
			signed char b = map[i - 1][j + 1];
			signed char c = map[i + 1][j - 1];
			signed char d = map[i + 1][j + 1];
			map_ij = (a + b + c + d) / 4;

			int rv = scrand(r);
			if (map_ij + rv > 16 )
				map_ij = 16;
			else if(map_ij + rv < -16)
				map_ij = -16;
			else
				map_ij += rv;
		}
	}

	// Square algorithm
	// 0 1 0
	// 1 0 1
	// 0 1 0
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = (i % 2 == 0); j < n; j += 2) {
			signed char& map_ij = map[i][j];

			// get surrounding values
			signed char a = 0, b = a, c = a, d = a;
			if (i != 0) a = map[i - 1][j];
			if (j != 0) b = map[i][j - 1];
			if (j + 1 != n) c = map[i][j + 1];
			if (i + 1 != n) d = map[i + 1][j];

			// average calculation
			if (i == 0) map_ij = (b + c + d) / 3;
			else if (j == 0) map_ij = (a + c + d) / 3;
			else if (j + 1 == n) map_ij = (a + b + d) / 3;
			else if (i + 1 == n) map_ij = (a + b + c) / 3;
			else map_ij = (a + b + c + d) / 4;

			int rv = scrand(r);
			if (map_ij + rv > 16 )
				map_ij = 16;
			else if(map_ij + rv < -16)
				map_ij = -16;
			else
				map_ij += rv;
		}

	}

	return map;
}
void TerrainClass::MPD(){
	const unsigned n = 130;
	
	signed char** final = new signed char*[n];
	for (unsigned i = 0;i < n; ++i) {
			final[i] = new signed char[n];
			for (unsigned j = 0; j < n; ++j){
				final[i][j] = scrand();
			}
	}

	for (unsigned i = 1; i < 8; ++i){ 
		final = mdp(final, n,  16/ i);
	}
	for (size_t i = 0; i < n-1; ++i) {
		for (size_t j = 0; j < n-1; ++j) {
			m_heightMap[(i * (n-1)) + j].y = final[i][j];
		}
	}
}
void TerrainClass::deposit( int x, int z)
{
	int j,k,kk,jj,flag;
	
	flag = 0;
	for (k=-1;k<2;k++)
		for(j=-1;j<2;j++)
			if (k!=0 && j!=0 && x+k>-1 && x+k<m_terrainWidth && z+j>-1 && z+j<m_terrainHeight) 
				if (m_heightMap[(x+k) * m_terrainHeight + (z+j)].y < m_heightMap[x * m_terrainHeight + z].y) {
					flag = 1;
					kk = k;
					jj = j;
				}

				if (!flag){
						m_heightMap[x * m_terrainHeight + z].y += (rand()%40)/10 + (rand()%40)/10;
				}else{
					deposit(x+kk,z+jj);
				}
}
int TerrainClass::terrainIterateParticleDeposition(int numIt) {
	int x,z,i,dir;

	if (m_heightMap == NULL){
		return 0;
	}

	x = rand() % m_terrainWidth;
	z = rand() % m_terrainHeight;

	for (i=0; i < numIt; i++) {
		dir = rand() % 4;

		if (dir == 2) {
			x++;
			if (x >= m_terrainWidth)
				x = 0;
		}
		else if (dir == 3){
			x--;
			if (x == -1)
				x = m_terrainWidth-1;
		}

		else if (dir == 1) {
			z++;
			if (z >= m_terrainHeight)
				z = 0;
		}
		else if (dir == 0){
			z--;
			if (z == -1)
				z = m_terrainHeight - 1;
		}
		deposit(x,z);
	}
	return 1;
}
