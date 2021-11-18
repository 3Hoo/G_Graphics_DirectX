#include "pch.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "Engine.h"

GraphicsCommandQueue::~GraphicsCommandQueue()
{
	::CloseHandle(_fenceEvent);
}

void GraphicsCommandQueue::Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain)
{
	_swapChain = swapChain;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// device�� ���� (� ������� ���� ������, ��� ���� ������(IID_PPV~) ) �� ����
	// queue�� ���� ������ queueDesc��, ť�� ���� ������ _cmdQueue��
	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_cmdQueue));

	// - D3D12_COMMAND_LIST_TYPE_DIRECT : GPU�� ���� �����ϴ� ��� ���
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAlloc));

	// GPU�� �ϳ��� �ý��ۿ����� 0����
	// DIRECT or BUNDLE
	// Allocator
	// �ʱ� ���� (�׸��� ����� nullptr�� ����)
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	// CommandList�� Close/Open ���°� �ִµ�
	// Open ���¿��� Command�� �ִٰ� Close�� ���� �����ϴ� ����
	_cmdList->Close();

	// Resource�� ���õ� cmdList �����
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_resCmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _resCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_resCmdList));

	// CreateFence
	// - CPU�� GPU�� ����ȭ �������� ���
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	// ::CreateEvent ��ü�� DX Ư���� ����� �ƴ�
	// ����ȭ�� ���� ����ϴ� �Լ�
	// event�� ������ ��ȣ�� ���� - � ������ �����ϸ� �Ķ���/������ ���ֱ�
}

void GraphicsCommandQueue::WaitSync()
{
	// �ϰ��� �Ű��� ��ȣ (0���� 1�� ����)
	_fenceValue++;

	// CommandQueue�� ���ο� fencepoint�� set
	// ������ GPU�� timeline�� �ֱ� ������, new fence�� �� Signal�̶�� �Լ� ������
	// �۾����̴� ��� command�� ������ set �� ���̴�
	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	// GPU�� �� fence point���� command�� ���� ������ ��ٸ���
	if (_fence->GetCompletedValue() < _fenceValue)
	{
		// GPU�� fenceValue ��ȣ���� �������� _fenceEvent ȣ��
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);

		// cpu�� gpu�� _fenceEvent ��� �̺�Ʈ�� ȣ���� ������ wait 
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void GraphicsCommandQueue::RenderBegin()
{
	// capacity�� �����ϸ鼭 �ʱ�ȭ
	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	// SwapChain�� �ϱ� ���� ��û�� ������ ���·� ����� �ִ�
	// ���ҽ� �踮���
	// �ش� ���ҽ��� � ���°� �Ǳ� ������ �� ���ҽ��� ������ �� ������ �����ش�
	// - ��� ���ҽ��� ��� ������ �ϳ��� STATE�� �������� �Ǿ��ְ�
	//   � �׷��� ����� �� ���ҽ��� ����� ������ �Ǳ� ���ؼ���
	//   �� ��ɿ� �ʿ��� ���·� ���ҽ��� ���°� �ٲ�� �־�� �Ѵ�
	uint8 backIndex = _swapChain->GetBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),		// 
		D3D12_RESOURCE_STATE_PRESENT,			// ���� ȭ�� ��µǴ� �κ�	(before)
		D3D12_RESOURCE_STATE_RENDER_TARGET);	// ���� �����	(after)

	// RootSignature (��༭, ����) ��������
	_cmdList->SetGraphicsRootSignature(GRAPHICS_ROOT_SIGNATURE.Get());

	// ConstantBuffer �ʱ�ȭ
	// (RenderEnd �� �� ConstantBuffer�� �ִ� ��� ���۵��� �������ͷ� �������Ƿ�
	//  �ٽ� ó������ �����͸� �׾ư�����)
	GEngine->GetConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM)->Clear();
	GEngine->GetConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL)->Clear();
	// TableDescriptorHeap �� �ʱ�ȭ
	GEngine->GetGraphicsDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap = GEngine->GetGraphicsDescHeap()->GetDescriptorHeap().Get();
	// ���� � heap�� ����� ������ ������ ��,
	// TableDescriptorHeap�� CommitTable �Լ��� �÷�������
	// �׷��� SetDescriptorHeaps�� �������� �ſ� ���� �Լ��̹Ƿ� �� ������ �� �� ���� �����϶�� �Ѵ�
	_cmdList->SetDescriptorHeaps(1, &descHeap);

	// �׸��� ������ ���� ����� _cmdList�� �־��ش�
	_cmdList->ResourceBarrier(1, &barrier);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	// 	   ���� rendertargetgroup���� ���� ��
	// _cmdList->RSSetViewports(1, vp);
	// _cmdList->RSSetScissorRects(1, rect);


	/* Texture Ŭ������ �̵�
	// � ���ۿ� �׸��� �׸� �� ������ش�
	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = _swapChain->GetBackRTV();	// ����۸� ������ ��
	_cmdList->ClearRenderTargetView(backBufferView, Colors::Black, 0, nullptr);	// �⺻ ������ lightsteelblue
	
	// depth stencil view�� ���� �غ� ���ش�
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = GEngine->GetDepthStencilBuffer()->GetDSVCpuHandle();
	
	// view���� GPU���� �˷��ش�
	_cmdList->OMSetRenderTargets(1, &backBufferView, FALSE, &depthStencilView);

	// depth buffer �ʱ�ȭ
	_cmdList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	*/
}

void GraphicsCommandQueue::RenderEnd()
{
	int8 backIndex = _swapChain->GetBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // ���� �����
		D3D12_RESOURCE_STATE_PRESENT); // ȭ�� ���
	// RenderBegin�� ������ �ݴ��!

	_cmdList->ResourceBarrier(1, &barrier);
	_cmdList->Close();

	// Ŀ�ǵ� ����Ʈ ����
	// Begin���� _cmdQueue�� �־����� ���� ���� ��û������!
	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	_swapChain->Present();

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	WaitSync();

	// ���� backbufferindex�� �ٲ�� �ȴ�
	_swapChain->SwapIndex();
}

// ���ҽ� gpu�� �ε��ϴ� �Լ�
void GraphicsCommandQueue::FlushResourceCommandQueue()
{
	_resCmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _resCmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	_resCmdAlloc->Reset();
	_resCmdList->Reset(_resCmdAlloc.Get(), nullptr);
}


// ************************
// ComputeCommandQueue
// ************************
ComputeCommandQueue::~ComputeCommandQueue()
{
	::CloseHandle(_fenceEvent);
}

void ComputeCommandQueue::Init(ComPtr<ID3D12Device> device)
{
	D3D12_COMMAND_QUEUE_DESC computeQueueDesc = {};
	computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&_cmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	// CreateFence
	// - CPU�� GPU�� ����ȭ �������� ���δ�
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void ComputeCommandQueue::WaitSync()
{
	_fenceValue++;

	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	if (_fence->GetCompletedValue() < _fenceValue)
	{
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void ComputeCommandQueue::FlushComputeCommandQueue()
{
	_cmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	auto t = _countof(cmdListArr);
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	COMPUTE_CMD_LIST->SetComputeRootSignature(COMPUTE_ROOT_SIGNATURE.Get());
}