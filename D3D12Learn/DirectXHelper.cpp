#pragma once

#include "DirectXHelper.h"

int gAdapterIndex = 0;
int gCurrRTIndex = 0;
UINT gRTVDescriptorSize = 0;
UINT gDSVDescriptorSize = 0;
UINT gFenceValue = 0;

ID3D12Device* gDevice = nullptr;
ID3D12CommandQueue* gQueue = nullptr;
ID3D12CommandAllocator* gAllocator = nullptr;
ID3D12GraphicsCommandList* gCommandList = nullptr;
IDXGISwapChain3* gSwapChain = nullptr;
ID3D12Resource* gDSRT = nullptr;
ID3D12Resource* gColorRTs[2];
ID3D12DescriptorHeap* gSwapChainRTVHeap = nullptr;
ID3D12DescriptorHeap* gSwapChainDSVHeap = nullptr;
ID3D12Fence* gFence = nullptr;

HANDLE gFenceEvent = nullptr;

ID3D12GraphicsCommandList* GetCommandList()
{
	return gCommandList;
}

ID3D12CommandAllocator* GetCommandAllocator()
{
	return gAllocator;
}

void SwapD3D12Buffer()
{
	gSwapChain->Present(0, 0);
}

ID3D12RootSignature* InitRootSignature()
{
	D3D12_ROOT_PARAMETER RootParameters[2];

	RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	RootParameters[0].Constants.RegisterSpace = 0; 
	RootParameters[0].Constants.ShaderRegister = 0; // ->b0
	RootParameters[0].Constants.Num32BitValues = 4;

	RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	RootParameters[1].Descriptor.RegisterSpace = 0;
	RootParameters[1].Descriptor.ShaderRegister = 1; // ->b1

	D3D12_ROOT_SIGNATURE_DESC RoogSignatureDesc{};
	RoogSignatureDesc.NumParameters = _countof(RootParameters);
	RoogSignatureDesc.pParameters = RootParameters;
	RoogSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Store 64 DWORD at most -> 128 WORD -> 16 bit
	ID3DBlob* Signature = nullptr;
	ID3DBlob* ErrorMsg = nullptr;
	HRESULT hResult = D3D12SerializeRootSignature(&RoogSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &ErrorMsg);
	if (FAILED(hResult))
	{
		return nullptr;
	}

	ID3D12RootSignature* RootSignature = nullptr;
	hResult = gDevice->CreateRootSignature(gAdapterIndex, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature));
	if (FAILED(hResult))
	{
		return nullptr;
	}

	return RootSignature;
}

bool CreateShaderFromFile(LPCTSTR InShaderFilePath, const char* InMainFUncName, const char* InTarget, D3D12_SHADER_BYTECODE* InShader)
{
	ID3DBlob* ShaderBuffer = nullptr;
	ID3DBlob* ErrorMsg = nullptr;
	HRESULT hResult = D3DCompileFromFile(InShaderFilePath, nullptr, nullptr, 
		InMainFUncName, InTarget, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &ShaderBuffer, &ErrorMsg);
	if (FAILED(hResult))
	{
		printf("CompileShaderFromFile error: [%s][%s]:[%s]\n", InMainFUncName, InTarget, (char*)ErrorMsg->GetBufferPointer());
		ErrorMsg->Release();
		return false;
	}

	InShader->pShaderBytecode = ShaderBuffer->GetBufferPointer();
	InShader->BytecodeLength = ShaderBuffer->GetBufferSize();

	return true;
}

void UpdateConstantBuffer(ID3D12Resource* InCB, void* InData, int InDataLen)
{
	D3D12_RANGE Range{0};
	unsigned char* pBuffer = nullptr;
	InCB->Map(0, &Range, reinterpret_cast<void**>(&pBuffer));
	memcpy(pBuffer, InData, InDataLen);
	InCB->Unmap(0, nullptr);
}

ID3D12PipelineState* CreatePSO(ID3D12RootSignature* InRootSignature, D3D12_SHADER_BYTECODE InVertexShader, D3D12_SHADER_BYTECODE InPixelShader)
{
	D3D12_INPUT_ELEMENT_DESC VertexDataElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} ,
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 4,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} ,
		{"NORMAL",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 8,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} ,
		{"TANGENT",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} ,
	};

	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{};
	InputLayoutDesc.NumElements = 4;
	InputLayoutDesc.pInputElementDescs = VertexDataElementDesc;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc{};
	PSODesc.pRootSignature = InRootSignature;
	PSODesc.InputLayout = InputLayoutDesc;
	PSODesc.VS = InVertexShader;
	PSODesc.PS = InPixelShader;
	PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSODesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	PSODesc.SampleDesc.Count = 1;
	PSODesc.SampleDesc.Quality = 0;
	PSODesc.SampleMask = 0xffffffff;
	PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PSODesc.NodeMask = gAdapterIndex;

	PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	PSODesc.RasterizerState.DepthBiasClamp = TRUE;
	//Option
	PSODesc.RasterizerState.FrontCounterClockwise = false;
	PSODesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	PSODesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	PSODesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	PSODesc.RasterizerState.MultisampleEnable = false;
	PSODesc.RasterizerState.AntialiasedLineEnable = false;
	PSODesc.RasterizerState.ForcedSampleCount = 0;
	PSODesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	PSODesc.DepthStencilState.DepthEnable = true;
	PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	PSODesc.BlendState = { 0 };

	D3D12_RENDER_TARGET_BLEND_DESC RTBlendDesc{
		FALSE, FALSE, 

		D3D12_BLEND_ONE, 
		D3D12_BLEND_ZERO, 
		D3D12_BLEND_OP_ADD,

		D3D12_BLEND_ONE, 
		D3D12_BLEND_ZERO, 
		D3D12_BLEND_OP_ADD,	

		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL,
	};

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		PSODesc.BlendState.RenderTarget[i] = RTBlendDesc;
	}

	PSODesc.NumRenderTargets = 1;

	ID3D12PipelineState* PSO = nullptr;
	HRESULT hResult = gDevice->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO));
	if (FAILED(hResult))
	{
		return nullptr;
	}

	return PSO;
}

ID3D12Resource* CreateBufferObject(ID3D12GraphicsCommandList* InCommandList, void* InData, int InDataLen, D3D12_RESOURCE_STATES InFinalResState)
{
	D3D12_HEAP_PROPERTIES HeapProp{};
	HeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC ResDesc{};
	ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResDesc.Alignment = 0;
	ResDesc.Width = InDataLen;
	ResDesc.Height = 1;
	ResDesc.DepthOrArraySize = 1;
	ResDesc.MipLevels = 1;
	ResDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResDesc.SampleDesc.Count = 1;
	ResDesc.SampleDesc.Quality = 0;
	ResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* BufferObject = nullptr;

	HRESULT hResult = gDevice->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&ResDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&BufferObject)
	);

	if (FAILED(hResult))
	{ 
		return nullptr;
	}

	ResDesc = BufferObject->GetDesc();
	UINT64 MemSizeUsed = 0;
	UINT64 RowSizeInBytes = 0;
	UINT RowUsed = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT SubresourceFootPrint{};
	gDevice->GetCopyableFootprints(&ResDesc, 0, 1, 0, &SubresourceFootPrint, &RowUsed, &RowSizeInBytes, &MemSizeUsed);

	ID3D12Resource* TmpBufferObject = nullptr;
	HeapProp = {};
	HeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hResult = gDevice->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&ResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&TmpBufferObject)
	);

	if (FAILED(hResult))
	{ 
		return nullptr;
	}

	// GPU fetch 32bytes once, notice aligment
	BYTE* pData = nullptr;
	TmpBufferObject->Map(0, nullptr, reinterpret_cast<void**>(&pData));
	BYTE* pDestTmpBuffer = reinterpret_cast<BYTE*>(pData + SubresourceFootPrint.Offset);

	const BYTE* pSrcData = reinterpret_cast<BYTE*>(InData);
	for (UINT i = 0; i < RowUsed; ++i)
	{
		memcpy(pDestTmpBuffer + SubresourceFootPrint.Footprint.RowPitch * i, pSrcData + RowSizeInBytes * i, RowSizeInBytes);
	}
	TmpBufferObject->Unmap(0, nullptr);

	InCommandList->CopyBufferRegion(BufferObject, 0, TmpBufferObject, 0, SubresourceFootPrint.Footprint.Width);
	D3D12_RESOURCE_BARRIER Barrier = InitResourceBarrier(
		BufferObject,
		D3D12_RESOURCE_STATE_COPY_DEST,
		InFinalResState
	);
	InCommandList->ResourceBarrier(1, &Barrier);

	return BufferObject;
}

ID3D12Resource* CreateConstantBufferObject(int InDataLen)
{
	D3D12_HEAP_PROPERTIES HeapProp{};
	HeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC ResDesc{};
	ResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResDesc.Alignment = 0;
	ResDesc.Width = InDataLen;
	ResDesc.Height = 1;
	ResDesc.DepthOrArraySize = 1;
	ResDesc.MipLevels = 1;
	ResDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResDesc.SampleDesc.Count = 1;
	ResDesc.SampleDesc.Quality = 0;
	ResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* BufferObject = nullptr;

	HRESULT hResult = gDevice->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&ResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&BufferObject)
	);

	if (FAILED(hResult))
	{ 
		return nullptr;
	}

	return BufferObject;
}

D3D12_RESOURCE_BARRIER InitResourceBarrier(ID3D12Resource* InResource, D3D12_RESOURCE_STATES InPreState, D3D12_RESOURCE_STATES InNextState)
{
	D3D12_RESOURCE_BARRIER ResourceBarrier{};
	memset(&ResourceBarrier, 0, sizeof(D3D12_RESOURCE_BARRIER));
	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Transition.pResource = InResource;
	ResourceBarrier.Transition.StateBefore = InPreState;
	ResourceBarrier.Transition.StateAfter = InNextState;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	return ResourceBarrier;
}

bool InitD3D12(HWND InHWND, int InWidth, int InHeight)
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
		else 
		{
			MessageBox(nullptr, L"D3D12 Debug Layer is missing!", L"Error", MB_OK);
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
	BufferDesc.Width = InWidth;
	BufferDesc.Height = InHeight;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc = BufferDesc;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = InHWND;
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
	ResDesc.Width = InWidth;
	ResDesc.Height = InHeight;
	ResDesc.DepthOrArraySize = 1;
	ResDesc.MipLevels = 1;
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
	hResult = gDevice->CreateCommandList(gAdapterIndex, D3D12_COMMAND_LIST_TYPE_DIRECT, gAllocator, nullptr, IID_PPV_ARGS(&gCommandList));
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

void BeginCommandList()
{
	GetCommandAllocator()->Reset();
	GetCommandList()->Reset(gAllocator, nullptr);
}

void EndCommandList()
{
	gCommandList->Close();

	ID3D12CommandList* ppCommandList[] = { gCommandList };
	gQueue->ExecuteCommandLists(1, ppCommandList);

	gFenceValue++;
	gQueue->Signal(gFence, gFenceValue);
}

void BeginRenderToSwapChain(ID3D12GraphicsCommandList* InCommandList)
{
	gCurrRTIndex = gSwapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER Barrier = InitResourceBarrier(
		gColorRTs[gCurrRTIndex],
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	InCommandList->ResourceBarrier(1, &Barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE ColorRT;
	D3D12_CPU_DESCRIPTOR_HANDLE DSRT;
	DSRT.ptr = gSwapChainDSVHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	ColorRT.ptr = gSwapChainRTVHeap->GetCPUDescriptorHandleForHeapStart().ptr + gCurrRTIndex * gRTVDescriptorSize;
	InCommandList->OMSetRenderTargets(1, &ColorRT, FALSE, &DSRT);

	D3D12_VIEWPORT ViewProt = { 0.0f, 0.0f, 1600.0f, 900.0f };
	D3D12_RECT ScissorRect = { 0, 0, 1600, 900 };

	InCommandList->RSSetViewports(1, &ViewProt);
	InCommandList->RSSetScissorRects(1, &ScissorRect);

	const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	InCommandList->ClearRenderTargetView(ColorRT, ClearColor, 0, nullptr);
	InCommandList->ClearDepthStencilView(DSRT, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void EndRenderToSwapChain(ID3D12GraphicsCommandList* InCommandList)
{
	D3D12_RESOURCE_BARRIER Barrier = InitResourceBarrier(
		gColorRTs[gCurrRTIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	InCommandList->ResourceBarrier(1, &Barrier);
}
