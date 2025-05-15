#pragma once

#include <d3d12.h>
#include <unordered_map>
#include <string>

using namespace std;

struct StaticMeshComponentVertexData
{
	float mPosition[4];
	float mTexcoord[4];
	float mNormal[4];
	float mTangent[4];
};

struct SubMesh
{	
	int mIndexCount;
	ID3D12Resource* mIBO;
	D3D12_INDEX_BUFFER_VIEW mIBV;
};

class StaticMeshComponent
{
public:

	void InitFromFile(ID3D12GraphicsCommandList* InCommandList, const char* InFilePath);
	void SetVertexCount(int InVertexCount);
	void SetVertexPosition(int InIndex, float InX, float InY, float InZ, float InW);
	void SetVertexTexcoord(int InIndex, float InX, float InY, float InZ, float InW);
	void SetVertexNormal(int InIndex, float InX, float InY, float InZ, float InW);
	void SetVertexTangent(int InIndex, float InX, float InY, float InZ, float InW);

	int mVertexCount;
	ID3D12Resource* mVBO;
	D3D12_VERTEX_BUFFER_VIEW mVBV;
	StaticMeshComponentVertexData* mVertexData;

	unordered_map<string, SubMesh*> mSubMeshes;
};