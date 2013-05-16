////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
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
// Class name: TerrainClass
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

	bool Initialize(ID3D11Device*, char*);
	bool InitializeFluid(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device* device);
	void GenerateRandomHeightMap();
	void Water();

	inline int  GetIndexCount() {return m_indexCount;}
	void AddWater(int x, int z);
	void ResetWater();
	void SetBorders(TerrainClass::HeightMapType * terrain_array,int size);
	void InitValues();
private:
	bool LoadHeightMap(char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();
	bool InitVertex();
	bool InitializeBuffers(ID3D11Device*);
	void ReleaseBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	float ValuesAroundPoint(int x, int z);
	void UpdateWaterValues();
	void DiminishWater();
	bool CheckBorder(int x, int z);
	void ResetBorders();
private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	HeightMapType* m_heightMap;
	float mWave;
	float mWaveTime;
	float mScale;
	VertexType* mVertices;
	unsigned long* mIndices;
};

#endif