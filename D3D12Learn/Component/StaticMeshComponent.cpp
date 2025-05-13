#include "StaticMeshComponent.h"

void StaticMeshComponent::InitFromFile(ID3D12GraphicsCommandList* InCommandList, const char* InFilePath)
{
	mVBOView.BufferLocation = mVBO->GetGPUVirtualAddress();
	mVBOView.SizeInBytes = sizeof(StaticMeshComponnentVertexData) * 3;
	mVBOView.StrideInBytes = sizeof(StaticMeshComponnentVertexData);
}

void StaticMeshComponent::SetVertexCount(int InVertexCount)
{
	mVertexCount = InVertexCount;
	mVertexData = new StaticMeshComponnentVertexData[mVertexCount];
	memset(mVertexData, 0, sizeof(StaticMeshComponnentVertexData));
}

void StaticMeshComponent::SetVertexPosition(int InIndex, float InX, float InY, float InZ, float InW)
{
	if (InIndex < 0 || InIndex > mVertexCount)
		return;

	mVertexData[InIndex].mPosition[0] = InX;
	mVertexData[InIndex].mPosition[1] = InY;
	mVertexData[InIndex].mPosition[2] = InZ;
	mVertexData[InIndex].mPosition[3] = InW;
}

void StaticMeshComponent::SetVertexTexcoord(int InIndex, float InX, float InY, float InZ, float InW)
{
	if (InIndex < 0 || InIndex > mVertexCount)
		return;

	mVertexData[InIndex].mTexcoord[0] = InX;
	mVertexData[InIndex].mTexcoord[1] = InY;
	mVertexData[InIndex].mTexcoord[2] = InZ;
	mVertexData[InIndex].mTexcoord[3] = InW;
}

void StaticMeshComponent::SetVertexNormal(int InIndex, float InX, float InY, float InZ, float InW)
{
	if (InIndex < 0 || InIndex > mVertexCount)
		return;

	mVertexData[InIndex].mNormal[0] = InX;
	mVertexData[InIndex].mNormal[1] = InY;
	mVertexData[InIndex].mNormal[2] = InZ;
	mVertexData[InIndex].mNormal[3] = InW;
}

void StaticMeshComponent::SetVertexTangent(int InIndex, float InX, float InY, float InZ, float InW)
{
	if (InIndex < 0 || InIndex > mVertexCount)
		return;

	mVertexData[InIndex].mTangent[0] = InX;
	mVertexData[InIndex].mTangent[1] = InY;
	mVertexData[InIndex].mTangent[2] = InZ;
	mVertexData[InIndex].mTangent[3] = InW;
}
