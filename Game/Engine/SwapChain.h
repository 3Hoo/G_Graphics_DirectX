#pragma once


// 교환 사슬
// [ 외주 과정 ]
// - 현재 게임 세상에 있는 상황을 묘사
// - 막연히 외주를 주는 것이 아니라, 어떤 공식으로 어떻게 계산할지 던져줌
// - GPU가 열심히 계산 (외주)
// - 그리고 그 결과를 받아서 화면에 그려준다

// [ 외주 결과물 ]을 어디거 받지?
// - 어떤 종이(Buffer)에 그려서 건내달라고 부탁해보자
// - 특수 종이를 만들어서 -> 처음에 건내주고 -> 결과물을 해당 종이에 받는다
// - 우리 화면에 특수 종이(외주 결과물)을 출력해준다

// [?]
// - 그런데 화면에서 혅 ㅐ결과를 출력하는 와중에 다음 화면도 외주를 맡겨야 함
// - 현재 화면 결과물은 이미 화면 출력에 사용중
// - 특수 종이를 2개 만들어서 하나는 현재 화면을 그려주고, 하나는 외주 맡겨놓고... 반복
// - Double Buffering!!

// [1] [2]
// - 현재 화면 [1]  <->  GPU 작업중 [2] (BackBuffer)
// - 현재 화면 [2]  <->  GPU 작업중 [1] (BackBuffer)
// - 현재 화면 [1]  <->  GPU 작업중 [2] (BackBuffer)
// - ...

class SwapChain
{
public:
	void Init(const WindowInfo& info, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);
	void Present();
	void SwapIndex();

	ComPtr<IDXGISwapChain> GetSwapChain() { return _swapChain; }
	uint8 GetBackBufferIndex() { return _backBufferIndex; }

	/* Texture 클래스로 이동
	ComPtr<ID3D12Resource> GetRenderTarget(int32 index) { return _rtvBuffer[index]; }

	ComPtr<ID3D12Resource> GetBackRTVBuffer() { return _rtvBuffer[_backBufferIndex]; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetBackRTV() { return _rtvHandle[_backBufferIndex]; }
	*/

private:
	void CreateSwapChain(const WindowInfo& info, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);
	
	// Texture 클래스로 이동
	// void CreateRTV(ComPtr<ID3D12Device> device);

private:
	ComPtr<IDXGISwapChain>	_swapChain;

	/*	Texture 클래스로 이동
	ComPtr<ID3D12Resource>				_rtvBuffer[SWAP_CHAIN_BUFFER_COUNT];	// EningePch를 보면 이 값은 2 (총 2개의 버퍼를 사용하므로)
	// DescriptorHeap [ [ ] [ ] ]   ---> [ Resource ] 를 가리킨다(설명한다)
	ComPtr<ID3D12DescriptorHeap>		_rtvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			_rtvHandle[SWAP_CHAIN_BUFFER_COUNT];
	*/

	uint32								_backBufferIndex = 0;	// 0 -> 1 -> 0 -> 1 -> ...
};

