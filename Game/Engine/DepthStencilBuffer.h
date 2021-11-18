#pragma once


class DepthStencilBuffer
{
public:
	void Init(const WindowInfo& window, DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT);
	// ����� �� ���۸� depth buffer �뵵�θ� ��� => ��ü 32bit�� depth ������ ���
	// ���� stencil���� ����ϰ��� �Ѵٸ�, dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT �� 8��Ʈ�� stencil�� ����Ѵ�

	D3D12_CPU_DESCRIPTOR_HANDLE	GetDSVCpuHandle() { return _dsvHandle; }
	DXGI_FORMAT GetDSVFormat() { return _dsvFormat; }

private:
	// Depth Stencil View
	ComPtr<ID3D12Resource>				_dsvBuffer;
	ComPtr<ID3D12DescriptorHeap>		_dsvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			_dsvHandle = {};
	DXGI_FORMAT							_dsvFormat = {};
};


