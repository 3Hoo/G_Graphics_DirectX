#pragma once

// [��༭ / ����]
// 
// CPU [          ]   GPU [           ]
// CPU�� �����κ��� �����͸� �̾ƾ� �� ������,
// GPU�� �����κ��� �����͸� �̱⿣ �ʹ� �ָ� �ִ�
// ������ �ϴ� �����͸� ���� �ִ� ���� �ʿ��ϱ�� ��

// GPU�� � ������ �����͸� �Ѱ��ִ� ������ �ʿ���
// �� ������ �� �Ͼ�� ���� �ƴ϶� � ���۸�, �������͸�, ��å�� ����� ������ ��༭(���̺��� ����)�� �ʿ�
// -> GPU Memory�� �����ִ� ����

// �� register�� ��� ����� �������� ���� �˷��ش�
// -> ���� �����Ͱ� ���޵Ǵ� ���� �ƴ�
//    ���� �����͸� �ű�� ���� ���� �ٸ� �ܰ�
//    ������ �Լ��� ȣ���ؾ� �Ѵ�

// �ᱹ register���� buffer�� view, �� ������ �ּҰ��� ���޵ȴ�
// �Ʒ� ������ Mesh���� �Ͼ��
// 1) cpu���� gpu�� buffer�� setting (���� ����) �Ѵ�
// 2) buffer�� view(�ּ�)�� ������ register�� �����ش�
// �׷��� 1)�� device�� ���� ��� �Ͼ����, 2)�� cmdQueue�� ���� ���߿� �߻��Ѵ�
// �� cmdqueue�� ���� �������� ���� �ٽ� buffer�� setting�ȴٸ� ���� ������ ������� ����ȭ����!
// => Mesh.cpp ���� �� �ذ���� �ִ�!

/*
* rootsignature�� ������ ���� ������ �̷���� �ִ�
* [ [API bind]  [ data ]  [ HLSL bind ]
* [ [API bind]  [ data ]  [ HLSL bind ]
* [ [API bind]  [ data ]  [ HLSL bind ] ...
* 
* API bind�� 0���� �����ϸ� �׳� ������ index�� �����ϸ� �ȴ�
* HLSL bind�� � �������͸� ������� ����д�. �� �������� �̸��� hlsl shader���� �ҷ��� ��� �����ϴ�
* data���� �ش� �������͸� � �뵵�� ����� ������ ���� ������ empty, ���, CBV, descriptor table�� �� �� �ִ�
* �� �� descriptor table�� ����� ��� ������ 
""[ [API BIND] [ DESC table ]  ] "" �� �Ǹ� desp table�� ���󰡸�
""[ [���� view]   [HLSL bind(Register)]  ] �� �迭�� �����Ѵ�
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

