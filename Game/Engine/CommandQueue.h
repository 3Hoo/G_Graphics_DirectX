#pragma once

// GPU���� ��� ���ָ� �� ��
// ���� DX ���������� �׶����� device�� ����(device����) �ϰ��� ���� ���
// �׷��� �ֱ� DX������ �� �����ϰ� �ٲ�
// CommandQueue �� �׾Ƴ��ٰ� RenderEnd �� �� ���� �����ϵ��� �Ѵ�
// ���� device�� �����ͼ� ���� ����� �ٷ� �����ϰ� �� ���� ���� 
//			(Mesh.cpp���� Device->CreateCommitedResource ���� �۾�)

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
	// CommandQueue : DX12�� ����
	// ���ָ� ��û�� ��, �ϳ��� ��û�ϸ� ��ȿ�������� �߰���
	// [���� ���]�� �ϰ��� ����ߴٰ� �� �濡 ��û�ϴ� ��!
	ComPtr<ID3D12CommandQueue>			_cmdQueue;	// �ϰ� ��ü�� �־��ֱ� ���� �޸𸮰��� �Ҵ�
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;	// �޷θ� �����ϴ� ��
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;
	// �̷��� 3���� �� �ϳ�

	// ���ҽ��� ���� cmd list
	ComPtr<ID3D12CommandAllocator>		_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_resCmdList;

	// Fence : ��Ÿ��
	// CPU / GPU�� ����ȭ�� ���� ������ ����
	// - ���� [���� ���]�� ���� ���������� ��ٷ��� �� ��, Fence�� ����Ѵ�
	// - ��, ���� 105�� ���ְ� ���� ������ ��ٸ��ڴ� �̷� ������
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