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
	bool GenerateHeightMap(ID3D11Device* device);

	inline int  GetIndexCount() const {return m_indexCount;}
	inline int getHeightMapSize() const {return m_terrainWidth;}

	inline HeightMapType* getHeightMap() {return m_heightMap;}
	inline ID3D11ShaderResourceView* GetTexture() {return m_Texture->GetTexture();}
private:
	bool LoadHeightMap(char*);
	void NormalizeHeightMap();
	bool CalculateNormals();
	void ShutdownHeightMap();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	void GenerateRandomHeightMap();
	void GenerateSinCos(int index);

	void ParticleDeposition(int numIt, float height);
	void Deposit( int x, int z, float value);

	void MidPointDisplacement (float heightScale, float h);
	float AvgDiamondVals (int i, int j, int stride, int size, int subSize);
	float AvgSquareVals (int i, int j, int stride, int size);

	void Faulting(int passes, float displacement);
	int CheckCrossProduct(float x1, float z1, float x2, float z2);
	
	void Smooth(int passes);
	
	float ValuesAroundPoint(int x, int z);
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