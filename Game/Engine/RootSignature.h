#pragma once

// [계약서 / 결재]
// 
// CPU [          ]   GPU [           ]
// CPU는 램으로부터 데이터를 뽑아쓸 수 있지만,
// GPU는 램으로부터 데이터를 뽑기엔 너무 멀리 있다
// 하지만 일단 데이터를 갖고 있는 것이 필요하기는 함

// GPU의 어떤 공간에 데이터를 넘겨주는 과정이 필요함
// 이 과정은 막 일어나는 것이 아니라 어떤 버퍼를, 레지스터를, 정책을 사용할 것인지 계약서(테이블의 형태)가 필요
// -> GPU Memory와 관련있는 내용

// 이 register를 어떻게 사용할 것인지에 대해 알려준다
// -> 실제 데이터가 전달되는 것은 아님
//    실제 데이터를 옮기는 것은 완전 다른 단계
//    별도의 함수를 호출해야 한다

// 결국 register에는 buffer의 view, 즉 일종의 주소값이 전달된다
// 아래 과정은 Mesh에서 일어난다
// 1) cpu에서 gpu로 buffer를 setting (값의 복사) 한다
// 2) buffer의 view(주소)를 지정한 register에 보내준다
// 그런데 1)은 device를 통해 즉시 일어나지만, 2)는 cmdQueue에 의해 나중에 발생한다
// 즉 cmdqueue에 의해 보내지기 전에 다시 buffer가 setting된다면 이전 값들은 사라지는 동기화문제!
// => Mesh.cpp 에서 그 해결법이 있다!

/*
* rootsignature은 다음과 같은 구조로 이루어져 있다
* [ [API bind]  [ data ]  [ HLSL bind ]
* [ [API bind]  [ data ]  [ HLSL bind ]
* [ [API bind]  [ data ]  [ HLSL bind ] ...
* 
* API bind는 0부터 시작하며 그냥 일종의 index로 생각하면 된다
* HLSL bind는 어떤 레지스터를 사용할지 적어둔다. 이 레지스터 이름은 hlsl shader에서 불러서 사용 가능하다
* data에는 해당 레지스터를 어떤 용도로 사용할 것인지 적는 것으로 empty, 상수, CBV, descriptor table이 들어갈 수 있다
* 이 때 descriptor table을 사용할 경우 구조는 
""[ [API BIND] [ DESC table ]  ] "" 이 되며 desp table을 따라가면
""[ [각종 view]   [HLSL bind(Register)]  ] 의 배열이 존재한다
*/

class RootSignature
{
public:
	void Init();

	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return _graphicsRootSignature; }
	ComPtr<ID3D12RootSignature>	GetComputeRootSignature() { return _computeRootSignature; }

private:
	void CreateGraphicsRootSignature();
	void CreateComputeRootSignature();

private:
	D3D12_STATIC_SAMPLER_DESC _samplerDesc;
	ComPtr<ID3D12RootSignature>	_graphicsRootSignature;
	ComPtr<ID3D12RootSignature>	_computeRootSignature;
};

