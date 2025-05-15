#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>

ID3D12GraphicsCommandList* GetCommandList();
ID3D12CommandAllocator* GetCommandAllocator();
void SwapD3D12Buffer();

ID3D12RootSignature* InitRootSignature();
bool CreateShaderFromFile(LPCTSTR InShaderFilePath, const char* InMainFUncName, const char* InTarget, D3D12_SHADER_BYTECODE* InShader);
void UpdateConstantBuffer(ID3D12Resource* InCB, void* InData, int InDataLen);
ID3D12PipelineState* CreatePSO(ID3D12RootSignature* InRootSignature, D3D12_SHADER_BYTECODE InVertexShader, D3D12_SHADER_BYTECODE InPixelShader);
ID3D12Resource* CreateBufferObject(ID3D12GraphicsCommandList* InCommandList, void* InData, int InDataLen, D3D12_RESOURCE_STATES InFinalResState);
ID3D12Resource* CreateConstantBufferObject(int InDataLen);
D3D12_RESOURCE_BARRIER InitResourceBarrier(ID3D12Resource* InResource, D3D12_RESOURCE_STATES InPreState, D3D12_RESOURCE_STATES InNextState);

bool InitD3D12(HWND InHWND, int InWidth, int InHeight);

void WaitForComplectionOfCommandList();
void BeginCommandList();
void EndCommandList();
void BeginRenderToSwapChain(ID3D12GraphicsCommandList* InCommandList);
void EndRenderToSwapChain(ID3D12GraphicsCommandList* InCommandList);

