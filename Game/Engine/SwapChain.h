#pragma once


// ��ȯ �罽
// [ ���� ���� ]
// - ���� ���� ���� �ִ� ��Ȳ�� ����
// - ������ ���ָ� �ִ� ���� �ƴ϶�, � �������� ��� ������� ������
// - GPU�� ������ ��� (����)
// - �׸��� �� ����� �޾Ƽ� ȭ�鿡 �׷��ش�

// [ ���� ����� ]�� ���� ����?
// - � ����(Buffer)�� �׷��� �ǳ��޶�� ��Ź�غ���
// - Ư�� ���̸� ���� -> ó���� �ǳ��ְ� -> ������� �ش� ���̿� �޴´�
// - �츮 ȭ�鿡 Ư�� ����(���� �����)�� ������ش�

// [?]
// - �׷��� ȭ�鿡�� �p ������� ����ϴ� ���߿� ���� ȭ�鵵 ���ָ� �ðܾ� ��
// - ���� ȭ�� ������� �̹� ȭ�� ��¿� �����
// - Ư�� ���̸� 2�� ���� �ϳ��� ���� ȭ���� �׷��ְ�, �ϳ��� ���� �ðܳ���... �ݺ�
// - Double Buffering!!

// [1] [2]
// - ���� ȭ�� [1]  <->  GPU �۾��� [2] (BackBuffer)
// - ���� ȭ�� [2]  <->  GPU �۾��� [1] (BackBuffer)
// - ���� ȭ�� [1]  <->  GPU �۾��� [2] (BackBuffer)
// - ...

class SwapChain
{
public:
	void Init(const WindowInfo& info, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);
	void Present();
	void SwapIndex();

	ComPtr<IDXGISwapChain> GetSwapChain() { return _swapChain; }
	uint8 GetBackBufferIndex() { return _backBufferIndex; }

	/* Texture Ŭ������ �̵�
	ComPtr<ID3D12Resource> GetRenderTarget(int32 index) { return _rtvBuffer[index]; }

	ComPtr<ID3D12Resource> GetBackRTVBuffer() { return _rtvBuffer[_backBufferIndex]; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetBackRTV() { return _rtvHandle[_backBufferIndex]; }
	*/

private:
	void CreateSwapChain(const WindowInfo& info, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);
	
	// Texture Ŭ������ �̵�
	// void CreateRTV(ComPtr<ID3D12Device> device);

private:
	ComPtr<IDXGISwapChain>	_swapChain;

	/*	Texture Ŭ������ �̵�
	ComPtr<ID3D12Resource>				_rtvBuffer[SWAP_CHAIN_BUFFER_COUNT];	// EningePch�� ���� �� ���� 2 (�� 2���� ���۸� ����ϹǷ�)
	// DescriptorHeap [ [ ] [ ] ]   ---> [ Resource ] �� ����Ų��(�����Ѵ�)
	ComPtr<ID3D12DescriptorHeap>		_rtvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			_rtvHandle[SWAP_CHAIN_BUFFER_COUNT];
	*/

	uint32								_backBufferIndex = 0;	// 0 -> 1 -> 0 -> 1 -> ...
};

