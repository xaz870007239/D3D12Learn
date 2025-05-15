#include <Windows.h>

#include "Component/StaticMeshComponent.h"
#include "DirectXHelper.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;

LPCTSTR gWindowClassName = L"D3D12 App";

LRESULT CALLBACK WndProc(HWND InHWND, UINT InMSG, WPARAM InWParam, LPARAM InLParam)
{
	switch (InMSG)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(InHWND, InMSG, InWParam, InLParam);
}

int WINAPI WinMain(HINSTANCE HInstance, HINSTANCE HPrevInstance, LPSTR LpCmdLine, int InShowCmd)
{
	WNDCLASSEX WinClassEx{};
	WinClassEx.cbSize = sizeof(WNDCLASSEX);
	WinClassEx.style = CS_HREDRAW | CS_VREDRAW;
	WinClassEx.cbClsExtra = NULL;
	WinClassEx.cbWndExtra = NULL;
	WinClassEx.hInstance = HInstance;
	WinClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WinClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	//WinClassEx.hCursor = LoadIcon(NULL, IDC_ARROW);
	WinClassEx.hbrBackground = NULL;
	WinClassEx.lpszMenuName = NULL;
	WinClassEx.lpszClassName = gWindowClassName;
	WinClassEx.lpfnWndProc = WndProc;
	
	if (!RegisterClassEx(&WinClassEx))
	{
		MessageBox(NULL, L"Register Class Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	int ViewWidth = 1600;
	int ViewHeight = 900;
	RECT ViewRect;
	ViewRect.left = 0;
	ViewRect.top = 0;
	ViewRect.right = ViewWidth;
	ViewRect.bottom = ViewHeight;

	AdjustWindowRect(&ViewRect, WS_OVERLAPPEDWINDOW, FALSE);

	int ScreenWidth = ViewRect.right - ViewRect.left;
	int ScreenHeight = ViewRect.bottom - ViewRect.top;
	HWND Hwnd = CreateWindowEx(NULL, gWindowClassName, L"D3D12 App", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, NULL, NULL, HInstance, NULL);

	if (!Hwnd)
	{
		MessageBox(NULL, L"Create WIndow Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	InitD3D12(Hwnd, 1600, 900);

	ID3D12GraphicsCommandList* CommandList = GetCommandList();

	StaticMeshComponent StaticMeshComp;
	StaticMeshComp.InitFromFile(CommandList, "Res/Model/Sphere.lhsm");

	ID3D12RootSignature* RootSignature = InitRootSignature();
	D3D12_SHADER_BYTECODE VS;
	D3D12_SHADER_BYTECODE PS;
	CreateShaderFromFile(L"Res/Shader/ndctriangle.hlsl", "VS_Main", "vs_5_0", &VS);
	CreateShaderFromFile(L"Res/Shader/ndctriangle.hlsl", "PS_Main", "ps_5_0", &PS);
	ID3D12PipelineState* PSO = CreatePSO(RootSignature, VS, PS);
	ID3D12Resource* CB = CreateConstantBufferObject(65535);


	DirectX::XMFLOAT4X4 TmpMatrix;
	float Matrices[16 * 4];

	DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(45.0f),
		(float)ViewWidth / (float)ViewHeight,
		0.1f,
		100.0f
	);
	DirectX::XMStoreFloat4x4(&TmpMatrix, ProjectionMatrix);
	memcpy(Matrices + 16 * 0, &TmpMatrix, sizeof(float) * 16);

	DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4(&TmpMatrix, ViewMatrix);
	memcpy(Matrices + 16 * 1, &TmpMatrix, sizeof(float) * 16);

	DirectX::XMMATRIX ModelMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 5.0f);
	ModelMatrix *= DirectX::XMMatrixRotationZ(90.0f * 3.1415926f / 180.0f);
	DirectX::XMStoreFloat4x4(&TmpMatrix, ModelMatrix);
	memcpy(Matrices + 16 * 2, &TmpMatrix, sizeof(float) * 16);

	DirectX::XMVECTOR Determinant;
	DirectX::XMMATRIX InverseModelMatrix = DirectX::XMMatrixInverse(&Determinant, ModelMatrix);
	if (DirectX::XMVectorGetX(Determinant) != 0.0f)
	{
		DirectX::XMMATRIX NormalMatrix = DirectX::XMMatrixTranspose(InverseModelMatrix);

		DirectX::XMStoreFloat4x4(&TmpMatrix, ModelMatrix);
		memcpy(Matrices + 16 * 3, &TmpMatrix, sizeof(float) * 16);
	}

	UpdateConstantBuffer(CB, Matrices, sizeof(float) * 16 * 4);
	EndCommandList();
	WaitForComplectionOfCommandList();

	ShowWindow(Hwnd, InShowCmd);
	UpdateWindow(Hwnd);

	float color[] = { 0.5f, 0.5f, 0.5f, 1.0f };

	MSG msg;
	while (true) 
	{
#if defined(_WIN32) || defined(_WIN64)
		ZeroMemory(&msg, sizeof(MSG));
#elif
		memset(msg, 0, sizeof(MSG);
#endif

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//Render
			WaitForComplectionOfCommandList();
			BeginCommandList();
			BeginRenderToSwapChain(CommandList);

			//...
			CommandList->SetPipelineState(PSO);
			CommandList->SetGraphicsRootSignature(RootSignature);
			CommandList->SetGraphicsRoot32BitConstants(0, 4, color, 0);
			CommandList->SetGraphicsRootConstantBufferView(1, CB->GetGPUVirtualAddress());
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			StaticMeshComp.Render(CommandList);

			EndRenderToSwapChain(CommandList);
			EndCommandList();

			SwapD3D12Buffer();
		}
	}

	return 0;
}