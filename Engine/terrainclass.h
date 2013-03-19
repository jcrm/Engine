////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dx10math.h>
#include <stdio.h>

class Generation;

////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass
{
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR3 normal;
	};


	struct VectorType 
	{ 
		float x, y, z;
	};

public:
	struct HeightMapType 
	{ 
		float x, y, z;
		float nx, ny, nz;
		float prevY, nextY;
	};
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device*, char*);
	bool InitializeTerrain(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool GenerateHeightMap(ID3D11Device* device, bool keydown);
	void GenerateRandomHeightMap();
	int  GetIndexCount();
	void MidPoint(float);
	void Particle(int index);
	void Water();
private:
	bool LoadHeightMap(char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	void MPD();
	int terrainIterateParticleDeposition(int numIt, bool up);
	void depositPlus(int x, int z);
	void smooth(float k);
	void depositMinus( int x, int z);
	void GenerateSinCos(int index);
	float ValuesAroundPoint(int x, int z);
	void UpdateWaterValues();
	inline int createIndex(int size, int i, int j){ return (size * i) + j;}
	inline signed char scrand(signed char r = 4) {return (-r + 2 * (rand() % r)); }
	signed char** mdp(signed char** base, unsigned base_n, signed char r);
private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	HeightMapType* m_heightMap;
	float mWave;
	float mWaveTime;
};

#endif