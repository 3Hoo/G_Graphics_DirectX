#include "pch.h"
#include "RootSignature.h"
#include "Engine.h"
#include <fstream>

void RootSignature::Init()
{
	CreateGraphicsRootSignature();
	CreateComputeRootSignature();
}

void OutputErrorMessage(ComPtr<ID3DBlob> errorMessage, HWND hwnd)
{
	// Get a pointer to the error message text buffer.
	char* compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	unsigned long long bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	ofstream fout;
	fout.open("blob-error_RootSignature.txt");

	// Write out the error message.
	for (int index = 0; index < bufferSize; ++index)
	{
		fout << compileErrors[index];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = nullptr;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	::MessageBoxA(hwnd, "Error compiling shader. Check shader-error.txt for message.", nullptr, MB_OK);

	return;
}

void RootSignature::CreateGraphicsRootSignature()
{
	_samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_DESCRIPTOR_RANGE ranges[] =
	{
		// constant buffer�� ���õ� �������ʹ� b�� ������(b0~b4)
		// (������ �μ��� 0�� b0�� �ǹ�) b0���� CBV_REGISTER_COUNT ������ŭ ����ϰڴ� (b0~b4 ���)
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT - 1, 1), // b1~b4

		// �ؽ�ó�� ���� ��������(t0~t4) ����
		// (������ �μ��� 0�� t0�� �ǹ�) t0���� SRV_REGISTER_COUNT ������ŭ ����ϰڴ� (t0~t4 ���)
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0), // t0~t4
	};

	/*
	CD3DX12_ROOT_PARAMETER param[2];
	param[0].InitAsConstantBufferView(0);	// 0�� API Bind(�ε������ ���� ����?)�� b0�� ���ε�! (�Ű������� 0�� b0�� �ǹ���)
	param[1].InitAsConstantBufferView(1);	// 1�� API Bind -> b1 register
	���ڵ鿡 Shader_Visibility�� �ִµ� �� �ɼ��� �ش� ������ ����/��� ����� �� ������ ������ (�⺻�� All)
	*/

	CD3DX12_ROOT_PARAMETER param[2];
	param[0].InitAsConstantBufferView(static_cast<uint32>(CBV_REGISTER::b0));	// �������� b0�� Ȱ���� ����
	param[1].InitAsDescriptorTable(_countof(ranges), ranges);
	// 0�� API Bind, Desc Table 
	//---->  [ [0] [CBV b0] ] , 
	//       [ [1] [SRV t0] ]

	//D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(D3D12_DEFAULT); // �⺻ ������ ���༭ �ۼ�
	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, 1, &_samplerDesc);
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // �Է� ������ �ܰ�

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_graphicsRootSignature));
	// OutputErrorMessage(blobError, nullptr);
}

void RootSignature::CreateComputeRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE ranges[] =
	{
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT, 0), // b0~b4
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0), // t0~t9
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UAV_REGISTER_COUNT, 0), // u0~u4
	};

	CD3DX12_ROOT_PARAMETER param[1];
	param[0].InitAsDescriptorTable(_countof(ranges), ranges);

	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param);
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_computeRootSignature));

	COMPUTE_CMD_LIST->SetComputeRootSignature(_computeRootSignature.Get());
}
