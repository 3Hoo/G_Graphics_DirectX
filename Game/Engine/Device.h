#pragma once

// 인력 사무소의 개념
// - GPU와 함께 일할 곳
// - 각종 객체의 생성

class Device
{
public:
	void Init();

	ComPtr<IDXGIFactory> GetDXGI() { return _dxgi; }
	ComPtr<ID3D12Device> GetDevice() { return _device; }

private:
	// DirectX 자체는 GPU를 제어하고 프로그래밍하는데 사용하는 저수준 API
	// 
	// [ COM = Component Object Model ]
	// - DX의 프로그래밍언어 독립성과 하위 호환성을 가능하게 하는 기술
	// - COM객체 (COM인터페이스) 를 사용, 세부사항은 비밀
	// - ComPtr은 일종의 COM 객체에 대한 스마트 포인터
	ComPtr<ID3D12Debug>			_debugController;
	ComPtr<IDXGIFactory>		_dxgi;		// 화면 관련 기능들이 포함됨. 인력사무소 에이스
	ComPtr<ID3D12Device>		_device;	// 각종 객체의 생성. 인력사무소 반장

};

