#pragma once

// GPU에게 계산 외주를 줄 때
// 옛날 DX 버전에서는 그때마다 device를 통해(device에게) 일감을 직접 줬다
// 그런데 최근 DX에서는 좀 복잡하게 바뀜
// CommandQueue 에 쌓아놨다가 RenderEnd 때 한 번에 수행하도록 한다
// 물론 device를 가져와서 직접 명령을 바로 수행하게 할 수도 있음 
//			(Mesh.cpp에서 Device->CreateCommitedResource 같은 작업)

class SwapChain;
class DescriptorHeap;

//***************************
//GraphicsCommandQueue
//***************************
class GraphicsCommandQueue
{
public:
	~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain);
	void WaitSync();

	void RenderBegin();
	void RenderEnd();

	void FlushResourceCommandQueue();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList() { return	_cmdList; }
	ComPtr<ID3D12GraphicsCommandList> GetResourceCmdList() { return	_resCmdList; }

private:
	// CommandQueue : DX12에 등장
	// 외주를 요청할 때, 하나씩 요청하면 비효율적임을 발견함
	// [외주 목록]에 일감을 기록했다가 한 방에 요청하는 것!
	ComPtr<ID3D12CommandQueue>			_cmdQueue;	// 일감 자체를 넣어주기 위한 메모리공간 할당
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;	// 메로리 관리하는 넘
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;
	// 이렇게 3개가 모여 하나

	// 리소스를 위한 cmd list
	ComPtr<ID3D12CommandAllocator>		_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_resCmdList;

	// Fence : 울타리
	// CPU / GPU의 동기화를 위한 간단한 도구
	// - 위의 [외주 목록]의 일이 끝날때까지 기다려야 할 때, Fence를 사용한다
	// - 즉, 만약 105번 외주가 끝날 때까지 기다리겠다 이런 식으로
	ComPtr<ID3D12Fence>					_fence;
	uint32								_fenceValue = 0;
	HANDLE								_fenceEvent = INVALID_HANDLE_VALUE;

	shared_ptr<class SwapChain>		_swapChain;
};


// ************************
// ComputeCommandQueue
// ************************
class ComputeCommandQueue
{
public:
	~ComputeCommandQueue();

	void Init(ComPtr<ID3D12Device> device);
	void WaitSync();
	void FlushComputeCommandQueue();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetComputeCmdList() { return _cmdList; }

private:
	ComPtr<ID3D12CommandQueue>			_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;

	ComPtr<ID3D12Fence>					_fence;
	uint32								_fenceValue = 0;
	HANDLE								_fenceEvent = INVALID_HANDLE_VALUE;
};