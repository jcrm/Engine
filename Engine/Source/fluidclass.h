////////////////////////////////////////////////////////////////////////////////
// Filename: fluidclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FLUIDCLASS_H_
#define _FLUIDCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>
#include "terrainclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: FluidClass
////////////////////////////////////////////////////////////////////////////////
class FluidClass
{
private:
	struct VertexType{
		D3DXVECTOR3 position;
	    D3DXVECTOR3 normal;
	};
	struct VectorType { 
		float x, y, z;
	};
public:
	struct HeightMapType { 
		float x, y, z;
		float nx, ny, nz;
		float prevY, nextY;
		bool border; 
	};
	FluidClass();
	FluidClass(const FluidClass&);
	~FluidClass();
	inline int  GetIndexCount() {return m_indexCount;}

	bool InitializeFluid(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device* device);

	void AddWater(int x, int z);
	void ResetWater();
	void SetBorders(TerrainClass::HeightMapType * terrain_array,int size);
	void InitValues();
private:
	//two water algorithms first the water will continue moving forever
	void Water();
	//the below function algorithm will diminish over time.
	void DiminishWater();
	//calculate normals
	bool CalculateNormals();
	//shutdown map array
	void ShutdownHeightMap();
	//create vertices's for the buffers
	bool InitVertex();
	//init and release buffers
	bool InitializeBuffers(ID3D11Device*);
	void ReleaseBuffers();
	//render the buffer
	void RenderBuffers(ID3D11DeviceContext*);
	//calculate the total of the four points around the point passed in
	float ValuesAroundPoint(int x, int z);
	//set old positions to be what the current positions is, and set the current position to be what the next position is
	void UpdateWaterValues();
	//check the vertex is not a border for the water
	bool AddBorder(int x, int z);
	//set the borders to be false except for the edge of the array
	void ResetBorders();
private:
	//width and height of the array
	int m_fluidWidth, m_fluidHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	HeightMapType* m_heightMap;
	//variables used in water algorithm: wave is used as difference between x and y in the water algorithms
	//wave time is the time step between iterations
	float mWave;
	float mWaveTime;
	//scales the water algorithms so it can meet the condition value
	float mScale;
	VertexType* mVertices;
	unsigned long* mIndices;
};

#endif