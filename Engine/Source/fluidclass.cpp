////////////////////////////////////////////////////////////////////////////////
// Filename: fluidclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fluidclass.h"
#include <cmath>

FluidClass::FluidClass(): m_vertexBuffer(0), m_indexBuffer(0), 
	m_heightMap(0), mScale(0.5f), mWave(1.0f)
{
	mWaveTime = 1/(2 * sqrt(2.0f));
}
FluidClass::FluidClass(const FluidClass& other): m_vertexBuffer(0), m_indexBuffer(0), 
	m_heightMap(0), mScale(0.5f), mWave(1.0f)
{
	mWaveTime = 1/(2 * sqrt(2.0f));
}
FluidClass::~FluidClass()
{
}
bool FluidClass::InitializeFluid(ID3D11Device* device, int terrainWidth, int terrainHeight)
{
	bool result;
	// Save the dimensions of the terrain.
	m_fluidWidth = terrainWidth;
	m_fluidHeight = terrainHeight;

	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_fluidWidth * m_fluidHeight];
	if(!m_heightMap){
		return false;
	}
	//set the borders to be the edges of the array only
	ResetBorders();
	//set all values to be zero
	ResetWater();
	// Calculate the normals for the terrain data.
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
void FluidClass::Shutdown(){
	// Release the vertex and index buffer.
	ReleaseBuffers();

	// Release the height map data.
	ShutdownHeightMap();

	return;
}
void FluidClass::Render(ID3D11DeviceContext* deviceContext){
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}
bool FluidClass::GenerateHeightMap( ID3D11Device* device ){
	bool result;
	//release the buffers so the new vertices's can be added to the buffer
	ReleaseBuffers();
	//calculate the new vertices's
	DiminishWater();
	//calculate new normals
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
bool FluidClass::CalculateNormals(){
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	VectorType* normals;

	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new VectorType[(m_fluidHeight-1) * (m_fluidWidth-1)];
	if(!normals){
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for(j=0; j<(m_fluidHeight-1); j++){
		for(i=0; i<(m_fluidWidth-1); i++){
			index1 = (j * m_fluidHeight) + i;
			index2 = (j * m_fluidHeight) + (i+1);
			index3 = ((j+1) * m_fluidHeight) + i;

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

			index = (j * (m_fluidHeight-1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices's and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for(j=0; j<m_fluidHeight; j++){
		for(i=0; i<m_fluidWidth; i++){
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if(((i-1) >= 0) && ((j-1) >= 0)){
				index = ((j-1) * (m_fluidHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if((i < (m_fluidWidth-1)) && ((j-1) >= 0)){
				index = ((j-1) * (m_fluidHeight-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if(((i-1) >= 0) && (j < (m_fluidHeight-1))){
				index = (j * (m_fluidHeight-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if((i < (m_fluidWidth-1)) && (j < (m_fluidHeight-1))){
				index = (j * (m_fluidHeight-1)) + i;

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
			index = (j * m_fluidHeight) + i;

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
void FluidClass::ShutdownHeightMap(){
	if(m_heightMap){
		delete [] m_heightMap;
		m_heightMap = 0;
	}

	return;
}
/*
* Create new vertices's, that use the quilt method for arrangement rather than square
*/
bool FluidClass::InitVertex(){
	int index;
	int index1, index2, index3, index4;
	// Create the vertex array.
	mVertices = new VertexType[m_vertexCount];
	if(!mVertices){
		return false;
	}

	// Create the index array.
	mIndices = new unsigned long[m_indexCount];
	if(!mIndices){
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;
	// Load the vertex and index array with the terrain data.
	for(int j=0; j<(m_fluidHeight-1); j++){
		for(int i=0; i<(m_fluidWidth-1); i++){
			index1 = (m_fluidHeight * j) + i;          // Bottom left.
			index2 = (m_fluidHeight * j) + (i+1);      // Bottom right.
			index3 = (m_fluidHeight * (j+1)) + i;      // Upper left.
			index4 = (m_fluidHeight * (j+1)) + (i+1);  // Upper right.

			if((i%2 !=0 && j%2 ==0) || (i%2 ==0 && j%2 != 0)){
				// Upper left.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				mIndices[index] = index;
				index++;

				// Upper right.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				mIndices[index] = index;
				index++;

				// Bottom right.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				mIndices[index] = index;
				index++;

				// Bottom right.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				mIndices[index] = index;
				index++;

				// Bottom left.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				mIndices[index] = index;
				index++;

				// Upper left.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				mIndices[index] = index;
				index++;

			}else{
				// Upper left.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
				mIndices[index] = index;
				index++;

				// Upper right.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				mIndices[index] = index;
				index++;

				// Bottom left.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				mIndices[index] = index;
				index++;

				// Bottom left.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
				mIndices[index] = index;
				index++;

				// Upper right.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
				mIndices[index] = index;
				index++;

				// Bottom right.
				mVertices[index].position = D3DXVECTOR3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
				mVertices[index].normal = D3DXVECTOR3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
				mIndices[index] = index;
				index++;
			}
		}
	}
	return true;
}
bool FluidClass::InitializeBuffers(ID3D11Device* device){
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Calculate the number of vertices's in the terrain mesh.
	m_vertexCount = (m_fluidWidth - 1) * (m_fluidHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;
	//create the verticies
	InitVertex();

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = mVertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	
	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result)){
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
    indexData.pSysMem = mIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result)){
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete [] mVertices;
	mVertices = 0;

	delete [] mIndices;
	mIndices = 0;

	return true;
}
void FluidClass::ReleaseBuffers(){
	// Release the index buffer.
	if(m_indexBuffer)	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}
	// Release the vertex buffer.
	if(m_vertexBuffer){
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
	return;
}
void FluidClass::RenderBuffers(ID3D11DeviceContext* deviceContext){
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
/*
* A basic water algorithm that doesnt dimish over time.
*/
void FluidClass::Water(){
	//constant multiplication of water and time
	static const float WaveAndTime = mWave*mWave*mWaveTime*mWaveTime;
	//constant multiplication for the current wave and time
	static const float CurrentWaveAndTime = 2-(4*WaveAndTime);
	//goes through array updating the nexty value based upon current values and previous values
	//of the current element and the elements above, below, left and right of it
	for(int i = 1; i<m_fluidWidth-1; i++){
		for(int j = 1; j<m_fluidWidth-1; j++){
			//the current element in the array
			int index = (m_fluidWidth * j) + i;
			//the first calculation to get based upon a multiplication of the current element
			float newVal = CurrentWaveAndTime*m_heightMap[index].y;
			//next minus previous value
			newVal-= m_heightMap[index].prevY;
			//then add a multiplication of the values around the element
			float sum = ValuesAroundPoint(i,j);
			newVal+= WaveAndTime*sum;
			m_heightMap[index].nextY = newVal;
		}
	}
	//update the array values so that the next is now current and the current is now old
	UpdateWaterValues();
}
void FluidClass::UpdateWaterValues(){
	for(int i = 0; i <m_fluidWidth; i++){
		for(int j = 0; j <m_fluidWidth; j++){
			int index = (m_fluidWidth * j) + i;
			//set previous equal to current value
			m_heightMap[index].prevY = m_heightMap[index].y;
			//set current value equal to next value
			m_heightMap[index].y = m_heightMap[index].nextY;
		}
	}
}
float FluidClass::ValuesAroundPoint(int x, int z){
	//add together the values from around the array
	float sum = m_heightMap[(m_fluidWidth * (z-1) ) + x].y;
	sum += m_heightMap[(m_fluidWidth * (z+1) ) + x].y;
	sum += m_heightMap[(m_fluidWidth * z) + x-1].y;
	sum += m_heightMap[(m_fluidWidth * z) + x+1].y;
	return sum;
}
/*
* uses a coefficent to diminish water over time
*/
void FluidClass::DiminishWater(){
	//0.05f is the coefficient that diminishes the water
	static const float UDeltaTime = mWaveTime * 0.05f;
	//constant multipliers
	static const float WaveAndTime = mWave*mWave*mWaveTime*mWaveTime;
	static const float CurrentWaveAndTime = ((4-(8*WaveAndTime))/(UDeltaTime+2));
	static const float PrevTime = ((UDeltaTime-2)/(UDeltaTime+2));
	//iterate through the array
	for(int i = 1; i<m_fluidWidth-1; i++){
		for(int j = 1; j<m_fluidWidth-1; j++){
			int index = (m_fluidWidth * j) + i;
			//if not a border then do calculations
			if(m_heightMap[index].border == false){
				//workout first part of algorithm based upon current element and a multiplier
				float newVal = CurrentWaveAndTime*m_heightMap[index].y;
				//add previous values times a multiplier
				newVal+= PrevTime*m_heightMap[index].prevY;
				//now add the values around the element
				float sum = ValuesAroundPoint(i,j);
				newVal+= ((2*WaveAndTime)/(UDeltaTime+2))*sum;
				m_heightMap[index].nextY = newVal;
			}
		}
	}
	UpdateWaterValues();
}
/*
*	Randomly add water drops to the array at various heights based upon scale.
*/
void FluidClass::AddWater(int x, int z){
	if(m_heightMap[(m_fluidWidth * x) + z].border == false){
		m_heightMap[(m_fluidWidth * x) + z].y+=(float(rand()%20)/10/mScale);
	}
}
/*
* Add a circular border to the water.
* Not used in example
*/
bool FluidClass::AddBorder(int x, int z){
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
/*
*	Reset the borders to be false unless the edges of the array
*/
void FluidClass::ResetBorders(){
	int index = 0;
	for(int j = 0; j < m_fluidWidth; j++){
		for(int i = 0; i < m_fluidWidth; i++){
			index = (m_fluidHeight * j) + i;
			//check to see if edges of the array
			if(i == 0 || i == m_fluidWidth-1 || j == 0 || j == m_fluidWidth-1){
				m_heightMap[index].border = true;;
			}else{
				m_heightMap[index].border = false;
			}
		}
	}
}
void FluidClass::ResetWater(){
	int index;
	float height = 0.0;
	// Initialize the data in the height map (flat).
	for(int j=0; j<m_fluidHeight; j++){
		for(int i=0; i<m_fluidWidth; i++){			
			index = (m_fluidHeight * j) + i;

			m_heightMap[index].x = ((float)i)/mScale-(float(m_fluidWidth)/2);
			m_heightMap[index].y = (float)height/mScale;
			m_heightMap[index].z = ((float)j)/mScale-(float(m_fluidWidth)/2);
			m_heightMap[index].nextY = 0.0f;
			m_heightMap[index].prevY = 0.0f;
		}
	}
}
/*
* Set init water values to a random height and location
*/
void FluidClass::InitValues(){
	int index;
	for(int j=m_fluidHeight/2-2; j<m_fluidHeight/2+2; j++){
		for(int i=m_fluidWidth/2-2; i<m_fluidWidth/2+2; i++){			
			index = (m_fluidHeight * j) + i;
			if(m_heightMap[index].border == false){
				m_heightMap[index].y = 2.0f/mScale;
			}
		}
	}
	for(int j=m_fluidHeight/2-10; j<m_fluidHeight/2-5; j++){
		for(int i=m_fluidWidth/2-10; i<m_fluidWidth/2-5; i++){			
			index = (m_fluidHeight * j) + i;
			if(m_heightMap[index].border == false){
				m_heightMap[index].y = -5.0f/mScale;
			}
		}
	}
	for(int j=m_fluidHeight/2+20; j<m_fluidHeight/2+25; j++){
		for(int i=m_fluidWidth/2+20; i<m_fluidWidth/2+25; i++){			
			index = (m_fluidHeight * j) + i;
			if(m_heightMap[index].border == false){
			m_heightMap[index].y = 1.5f/mScale;
			}
		}
	}
}
/*
* Takes in the Terrain HeightMap via a pointer and works out if the height is above zero
* set a border in the water array at the same location
*/
void FluidClass::SetBorders(TerrainClass::HeightMapType * terrainArray,int size){
	int index = 0;
	for(int i = 0; i < size; i++){
		for(int j = 0; j<size;j++){
			index = (m_fluidHeight * i) + j;
			if(terrainArray[index].y > 0){
				m_heightMap[index].border = true;
			}
		}
	}
}