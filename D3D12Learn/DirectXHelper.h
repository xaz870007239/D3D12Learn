#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>

ID3D12Resource* CreateBufferObject(ID3D12GraphicsCommandList* InCommandList, void* InData, int InDataLen, D3D12_RESOURCE_STATES InFinalResState);
