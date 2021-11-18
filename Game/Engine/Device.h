#pragma once

// �η� �繫���� ����
// - GPU�� �Բ� ���� ��
// - ���� ��ü�� ����

class Device
{
public:
	void Init();

	ComPtr<IDXGIFactory> GetDXGI() { return _dxgi; }
	ComPtr<ID3D12Device> GetDevice() { return _device; }

private:
	// DirectX ��ü�� GPU�� �����ϰ� ���α׷����ϴµ� ����ϴ� ������ API
	// 
	// [ COM = Component Object Model ]
	// - DX�� ���α׷��־�� �������� ���� ȣȯ���� �����ϰ� �ϴ� ���
	// - COM��ü (COM�������̽�) �� ���, ���λ����� ���
	// - ComPtr�� ������ COM ��ü�� ���� ����Ʈ ������
	ComPtr<ID3D12Debug>			_debugController;
	ComPtr<IDXGIFactory>		_dxgi;		// ȭ�� ���� ��ɵ��� ���Ե�. �η»繫�� ���̽�
	ComPtr<ID3D12Device>		_device;	// ���� ��ü�� ����. �η»繫�� ����

};

