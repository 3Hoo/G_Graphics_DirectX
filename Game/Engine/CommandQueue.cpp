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

	// device를 통해 (어떤 방식으로 만들 것인지, 어디에 만들 것인지(IID_PPV~) ) 를 설정
	// queue에 대한 설명은 queueDesc에, 큐에 대한 정보는 _cmdQueue로
	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_cmdQueue));

	// - D3D12_COMMAND_LIST_TYPE_DIRECT : GPU가 직접 실행하는 명령 목록
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAlloc));

	// GPU가 하나인 시스템에서는 0으로
	// DIRECT or BUNDLE
	// Allocator
	// 초기 상태 (그리기 명령은 nullptr로 지정)
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	// CommandList는 Close/Open 상태가 있는데
	// Open 상태에서 Command를 넣다가 Close한 다음 제출하는 개념
	_cmdList->Close();

	// Resource와 관련된 cmdList 만들기
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_resCmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _resCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_resCmdList));

	// CreateFence
	// - CPU와 GPU의 동기화 수단으로 사용
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	// ::CreateEvent 자체는 DX 특유의 기능이 아닌
	// 동기화에 자주 사용하는 함수
	// event는 일종의 신호등 역할 - 어떤 조건을 만족하면 파란불/빨간불 켜주기
}

void GraphicsCommandQueue::WaitSync()
{
	// 일감에 매겨진 번호 (0부터 1씩 증가)
	_fenceValue++;

	// CommandQueue에 새로운 fencepoint를 set
	// 지금은 GPU의 timeline에 있기 때문에, new fence는 이 Signal이라는 함수 이전에
	// 작업중이던 모든 command가 끝나야 set 될 것이다
	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	// GPU가 이 fence point까지 command를 끝낼 때까지 기다린다
	if (_fence->GetCompletedValue() < _fenceValue)
	{
		// GPU가 fenceValue 번호까지 끝났으면 _fenceEvent 호출
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);

		// cpu는 gpu가 _fenceEvent 라는 이벤트를 호출할 때까지 wait 
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void GraphicsCommandQueue::RenderBegin()
{
	// capacity는 유지하면서 초기화
	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	// SwapChain을 하기 위한 요청을 베리어 형태로 만들고 있다
	// 리소스 배리어는
	// 해당 리소스가 어떤 상태가 되기 전까지 그 리소스를 실행할 수 없도록 막아준다
	// - 모든 리소스는 어느 순간에 하나의 STATE를 가지도록 되어있고
	//   어떤 그래픽 명령이 그 리소스를 사용해 실행이 되기 위해서는
	//   그 명령에 필요한 상태로 리소스의 상태가 바뀌어 있어야 한다
	uint8 backIndex = _swapChain->GetBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),		// 
		D3D12_RESOURCE_STATE_PRESENT,			// 현재 화면 출력되는 부분	(before)
		D3D12_RESOURCE_STATE_RENDER_TARGET);	// 외주 결과물	(after)

	// RootSignature (계약서, 서명서) 가져오기
	_cmdList->SetGraphicsRootSignature(GRAPHICS_ROOT_SIGNATURE.Get());

	// ConstantBuffer 초기화
	// (RenderEnd 할 때 ConstantBuffer에 있던 모든 버퍼들이 레지스터로 보내지므로
	//  다시 처음부터 데이터를 쌓아가야함)
	GEngine->GetConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM)->Clear();
	GEngine->GetConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL)->Clear();
	// TableDescriptorHeap 도 초기화
	GEngine->GetGraphicsDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap = GEngine->GetGraphicsDescHeap()->GetDescriptorHeap().Get();
	// 먼저 어떤 heap을 사용할 것인지 지정한 후,
	// TableDescriptorHeap의 CommitTable 함수로 올려보낸다
	// 그런데 SetDescriptorHeaps는 문서에서 매우 느린 함수이므로 한 프레임 당 한 번만 실행하라고 한다
	_cmdList->SetDescriptorHeaps(1, &descHeap);

	// 그리고 위에서 만든 베리어를 _cmdList에 넣어준다
	_cmdList->ResourceBarrier(1, &barrier);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	// 	   이제 rendertargetgroup에서 직접 함
	// _cmdList->RSSetViewports(1, vp);
	// _cmdList->RSSetScissorRects(1, rect);


	/* Texture 클래스로 이동
	// 어떤 버퍼에 그림을 그릴 지 언급해준다
	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = _swapChain->GetBackRTV();	// 백버퍼를 꺼내온 후
	_cmdList->ClearRenderTargetView(backBufferView, Colors::Black, 0, nullptr);	// 기본 배경색은 lightsteelblue
	
	// depth stencil view를 보낼 준비를 해준다
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = GEngine->GetDepthStencilBuffer()->GetDSVCpuHandle();
	
	// view들을 GPU에게 알려준다
	_cmdList->OMSetRenderTargets(1, &backBufferView, FALSE, &depthStencilView);

	// depth buffer 초기화
	_cmdList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	*/
}

void GraphicsCommandQueue::RenderEnd()
{
	int8 backIndex = _swapChain->GetBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // 외주 결과물
		D3D12_RESOURCE_STATE_PRESENT); // 화면 출력
	// RenderBegin과 순서가 반대다!

	_cmdList->ResourceBarrier(1, &barrier);
	_cmdList->Close();

	// 커맨드 리스트 수행
	// Begin에서 _cmdQueue에 넣었으니 이제 직접 요청해주자!
	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	_swapChain->Present();

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	WaitSync();

	// 이제 backbufferindex가 바뀌게 된다
	_swapChain->SwapIndex();
}

// 리소스 gpu에 로드하는 함수
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
	// - CPU와 GPU의 동기화 수단으로 쓰인다
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