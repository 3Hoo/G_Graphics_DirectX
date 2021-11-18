#pragma once


class DepthStencilBuffer
{
public:
	void Init(const WindowInfo& window, DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT);
	// 현재는 이 버퍼를 depth buffer 용도로만 사용 => 전체 32bit를 depth 값으로 사용
	// 만약 stencil까지 사용하고자 한다면, dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT 로 8비트를 stencil로 사용한다

	D3D12_CPU_DESCRIPTOR_HANDLE	GetDSVCpuHandle() { return _dsvHandle; }
	DXGI_FORMAT GetDSVFormat() { return _dsvFormat; }

private:
	// Depth Stencil View
	ComPtr<ID3D12Resource>				_dsvBuffer;
	ComPtr<ID3D12DescriptorHeap>		_dsvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			_dsvHandle = {};
	DXGI_FORMAT							_dsvFormat = {};
};


