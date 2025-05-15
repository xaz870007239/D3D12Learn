#include <stdio.h>

#include "StaticMeshComponent.h"
#include "../DirectXHelper.h"

void StaticMeshComponent::InitFromFile(ID3D12GraphicsCommandList* InCommandList, const char* InFilePath)
{
	FILE* pFile = nullptr;
	errno_t Error = fopen_s(&pFile, InFilePath, "rb");
	if (Error == 0) {
		int Tmp = 0;
		fread(&Tmp, 4, 1, pFile);
		mVertexCount = Tmp;
		mVertexData = new StaticMeshComponentVertexData[mVertexCount];
		fread(mVertexData, 1, sizeof(StaticMeshComponentVertexData) * mVertexCount, pFile);
		mVBO = CreateBufferObject(
			InCommandList,
			mVertexData,
			sizeof(StaticMeshComponentVertexData) * mVertexCount,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		mVBV.BufferLocation = mVBO->GetGPUVirtualAddress();
		mVBV.SizeInBytes = sizeof(StaticMeshComponentVertexData) * mVertexCount;
		mVBV.StrideInBytes = sizeof(StaticMeshComponentVertexData);

		while (!feof(pFile)) {
			fread(&Tmp, 4, 1, pFile);
			if (feof(pFile)) 
			{
				break;
			}
			char name[256] = {0};
			fread(name, 1, Tmp, pFile);
			fread(&Tmp, 4, 1, pFile);
			SubMesh* SM = new SubMesh();
			SM->mIndexCount = Tmp;
			unsigned int *Indexes = new unsigned int[Tmp];
			fread(Indexes, 1, sizeof(unsigned int) * Tmp, pFile);
			SM->mIBO = CreateBufferObject(
				InCommandList, 
				Indexes,
				sizeof(unsigned int) * Tmp,
				D3D12_RESOURCE_STATE_INDEX_BUFFER
			);

			SM->mIBV.BufferLocation = SM->mIBO->GetGPUVirtualAddress();
			SM->mIBV.SizeInBytes = sizeof(unsigned int) * Tmp;
			SM->mIBV.Format = DXGI_FORMAT_R32_UINT;

			mSubMeshes.insert(make_pair(name,SM));
			delete[]Indexes;
		}
		fclose(pFile);
	}
}

void StaticMeshComponent::Render(ID3D12GraphicsCommandList* InCommandList)
{
	D3D12_VERTEX_BUFFER_VIEW VBVs[] =
	{
		mVBV
	};

	InCommandList->IASetVertexBuffers(0, 1, VBVs);

	if (mSubMeshes.empty())
	{
		InCommandList->DrawInstanced(mVertexCount, 1, 0, 0);
	}
	else
	{
		for (auto Iter = mSubMeshes.begin(); Iter != mSubMeshes.end(); ++Iter)
		{
			InCommandList->IASetIndexBuffer(&Iter->second->mIBV);
			InCommandList->DrawIndexedInstanced(Iter->second->mIndexCount, 1, 0, 0, 0);
		}
	}
}

void StaticMeshComponent::SetVertexCount(int InVertexCount)
{
	mVertexCount = InVertexCount;
	mVertexData = new StaticMeshComponentVertexData[mVertexCount];
	memset(mVertexData, 0, sizeof(StaticMeshComponentVertexData));
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
