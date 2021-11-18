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
		// constant buffer와 관련된 레지스터는 b로 시작함(b0~b4)
		// (마지막 인수인 0이 b0를 의미) b0부터 CBV_REGISTER_COUNT 개수만큼 사용하겠다 (b0~b4 사용)
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT - 1, 1), // b1~b4

		// 텍스처를 위한 레지스터(t0~t4) 예약
		// (마지막 인수인 0이 t0를 의미) t0부터 SRV_REGISTER_COUNT 개수만큼 사용하겠다 (t0~t4 사용)
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0), // t0~t4
	};

	/*
	CD3DX12_ROOT_PARAMETER param[2];
	param[0].InitAsConstantBufferView(0);	// 0번 API Bind(인덱스라고 했죵 내가?)을 b0에 바인딩! (매개변수의 0이 b0을 의미함)
	param[1].InitAsConstantBufferView(1);	// 1번 API Bind -> b1 register
	인자들에 Shader_Visibility가 있는데 이 옵션은 해당 내용을 언제/어디서 사용할 수 있을지 설정함 (기본은 All)
	*/

	CD3DX12_ROOT_PARAMETER param[2];
	param[0].InitAsConstantBufferView(static_cast<uint32>(CBV_REGISTER::b0));	// 전역으로 b0를 활용할 것임
	param[1].InitAsDescriptorTable(_countof(ranges), ranges);
	// 0번 API Bind, Desc Table 
	//---->  [ [0] [CBV b0] ] , 
	//       [ [1] [SRV t0] ]

	//D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(D3D12_DEFAULT); // 기본 상태의 빈계약서 작성
	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, 1, &_samplerDesc);
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // 입력 조립기 단계

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
