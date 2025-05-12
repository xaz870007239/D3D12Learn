#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;

LPCTSTR gWindowClassName = L"D3D12 App";
int gAdapterIndex = 0;
UINT gRTVDescriptorSize = 0;
UINT gDSVDescriptorSize = 0;
UINT gFenceValue = 0;

ID3D12Device* gDevice = nullptr;
ID3D12CommandQueue* gQueue = nullptr;
ID3D12CommandAllocator* gAllocator = nullptr;
ID3D12CommandList* gList = nullptr;
IDXGISwapChain3* gSwapChain = nullptr;
ID3D12Resource* gDSRT = nullptr;
ID3D12Resource* gColorRTs[2];
ID3D12DescriptorHeap* gSwapChainRTVHeap = nullptr;
ID3D12DescriptorHeap* gSwapChainDSVHeap = nullptr;
ID3D12Fence* gFence = nullptr;

HANDLE gFenceEvent = nullptr;

LRESULT CALLBACK WndProc(HWND inHWND, UINT inMSG, WPARAM inWParam, LPARAM inLParam)
{
	switch (inMSG)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(inHWND, inMSG, inWParam, inLParam);
}

bool InitD3D12(HWND inHWND, int inWidth, int inHeight)
{
	HRESULT hResult;
	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	{
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	IDXGIFactory4* dxgiFactory = nullptr;
	hResult = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

	if (FAILED(hResult))
	{
		return false;
	}

	IDXGIAdapter1* adapter;
	bool adapterFound = false;
	while (dxgiFactory->EnumAdapters1(gAdapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) && (desc.DedicatedSystemMemory > 1024 * 1024))
		{
			continue;
		}

		hResult = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hResult))
		{
			adapterFound = true;
			break;
		}

		gAdapterIndex++;
	}

	if (!adapterFound)
	{
		return false;
	}

	hResult = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&gDevice));
	if (FAILED(hResult))
	{
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc{};
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.NodeMask = gAdapterIndex;
	CommandQueueDesc.Priority = 0;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = gDevice->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&gQueue));
	if (FAILED(hResult))
	{
		return false;
	}

	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	DXGI_MODE_DESC BufferDesc{};
	BufferDesc.Width = inWidth;
	BufferDesc.Height = inHeight;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc = BufferDesc;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = inHWND;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IDXGISwapChain* TmpSwapChain = nullptr;
	hResult = dxgiFactory->CreateSwapChain(gQueue, &SwapChainDesc, &TmpSwapChain);
	if (FAILED(hResult))
	{
		return false;
	}
	gSwapChain = static_cast<IDXGISwapChain3*>(TmpSwapChain);

	D3D12_HEAP_PROPERTIES HeapProp{};
	HeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	
	D3D12_RESOURCE_DESC ResDesc{};
	ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResDesc.Alignment = 0;
	ResDesc.Width = inWidth;
	ResDesc.Height = inHeight;
	ResDesc.DepthOrArraySize = 1;
	ResDesc.MipLevels = 0;
	ResDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ResDesc.SampleDesc.Count = 1;
	ResDesc.SampleDesc.Quality = 0;
	ResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE DSClearValue{};
	DSClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSClearValue.DepthStencil.Depth = 1.0f;
	DSClearValue.DepthStencil.Stencil = 0;

	hResult = gDevice->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&ResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&DSClearValue,
		IID_PPV_ARGS(&gDSRT)
	);

	if (FAILED(hResult))
	{
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc{};
	RTVHeapDesc.NumDescriptors = 2;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	hResult = gDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&gSwapChainRTVHeap));
	gRTVDescriptorSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	if (FAILED(hResult))
	{
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc{};
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = gDevice->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&gSwapChainDSVHeap));
	gDSVDescriptorSize = gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	if (FAILED(hResult))
	{
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RTVHeapStart = gSwapChainRTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < 2; ++i)
	{
		gSwapChain->GetBuffer(i, IID_PPV_ARGS(&gColorRTs[i]));
		D3D12_CPU_DESCRIPTOR_HANDLE RTVPointer;
		RTVPointer.ptr = RTVHeapStart.ptr + i * gRTVDescriptorSize;
		gDevice->CreateRenderTargetView(gColorRTs[i], nullptr, RTVPointer);
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
	DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	gDevice->CreateDepthStencilView(gDSRT, &DSVDesc, gSwapChainDSVHeap->GetCPUDescriptorHandleForHeapStart());

	hResult = gDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gAllocator));
	hResult = gDevice->CreateCommandList(gAdapterIndex, D3D12_COMMAND_LIST_TYPE_DIRECT, gAllocator, nullptr, IID_PPV_ARGS(&gList));
	hResult = gDevice->CreateFence(gAdapterIndex, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gFence));
	if (FAILED(hResult))
	{
		return false;
	}
	
	gFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return true;
}

void WaitForComplectionOfCommandList()
{
	if (gFence->GetCompletedValue() < gFenceValue)
	{
		gFence->SetEventOnCompletion(gFenceValue, gFenceEvent);
		WaitForSingleObject(gFenceEvent, INFINITE);
	}
}

void EndCommandList()
{
	gFenceValue++;
	gQueue->Signal(gFence, gFenceValue);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int inShowCmd)
{
	WNDCLASSEX WinClassEx{};
	WinClassEx.cbSize = sizeof(WNDCLASSEX);
	WinClassEx.style = CS_HREDRAW | CS_VREDRAW;
	WinClassEx.cbClsExtra = NULL;
	WinClassEx.cbWndExtra = NULL;
	WinClassEx.hInstance = hInstance;
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
		CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, NULL, NULL, hInstance, NULL);

	if (!Hwnd)
	{
		MessageBox(NULL, L"Create WIndow Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	ShowWindow(Hwnd, inShowCmd);
	UpdateWindow(Hwnd);
	InitD3D12(Hwnd, 1600, 900);

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
		}
	}

	return 0;
}