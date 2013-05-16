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
#include "textureclass.h"

#define TEXTURE_REPEAT 8
////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass
{
private:
	struct VertexType{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};
	struct VectorType { 
		float x, y, z;
	};

public:
	struct HeightMapType { 
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
	TerrainClass();
	TerrainClass(const TerrainClass&);
	~TerrainClass();

	bool Initialize(ID3D11Device*, char*);
	bool InitializeTerrain(ID3D11Device* device, int terrainWidth, int terrainHeight, WCHAR* textureFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	//generate height map
	bool GenerateHeightMap(ID3D11Device* device);
	//getters
	inline int  GetIndexCount() const {return m_indexCount;}
	inline int getHeightMapSize() const {return m_terrainWidth;}
	inline HeightMapType* getHeightMap() {return m_heightMap;}
	inline ID3D11ShaderResourceView* GetTexture() {return m_Texture->GetTexture();}
private:
	bool LoadHeightMap(char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();
	//buffer functions
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	//Particle Deposition based on code from http://www.lighthouse3d.com/opengl/appstools/tg/
	void ParticleDeposition(int numIt, float height);
	void Deposit( int x, int z, float value);
	//Mid-Point Displacement based upon http://gameprogrammer.com/fractal.html
	void MidPointDisplacement (float heightScale, float h);
	float AvgDiamondVals (int i, int j, int stride, int size, int subSize);
	float AvgSquareVals (int i, int j, int stride, int size);
	//Faulting based upon code found at http://www.lighthouse3d.com/opengl/terrain/index.php?impdetails
	void Faulting(int passes, float displacement);
	void Smooth(int passes);
	//random height maps
	void GenerateRandomHeightMap();
	void GenerateSinCos(int index);
	//texture functions 
	void ReleaseTexture();
	bool LoadTexture(ID3D11Device* device, WCHAR* filename);
	void CalculateTextureCoordinates();
private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	HeightMapType* m_heightMap;
	TextureClass* m_Texture;
};

#endif