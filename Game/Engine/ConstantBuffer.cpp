#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine.h"


ConstantBuffer::ConstantBuffer()
{
}


ConstantBuffer::~ConstantBuffer()
{
	if (_cbvBuffer)
	{
		if (_cbvBuffer != nullptr)
			_cbvBuffer->Unmap(0, nullptr);

		_cbvBuffer = nullptr;
	}
}



void ConstantBuffer::Init(CBV_REGISTER reg, uint32 size, uint32 count)
{
	_reg = reg;

	// ��� ���۴� 256 ����Ʈ ����� ������ �Ѵ�
	// 0 256 512 768
	_elementSize = (size + 255) & ~255;	// ~255 = ... 1111 1111 0000 0000
										// �� 256����� ���� �ʴ� ���� �κ��� ��Ʈ���ؼ� ����������
										// ��, �ݿø��� �ϱ� ���� ���� +255�� �Ѵ�
	_elementCount = count;

	CreateBuffer();
	CreateView(); // -------------			
	//				             |
	/*                           v
	ConstantBuffer			Desc(CBV) Heap				  Desc. Heap(Shader Visible) b0~b4 �� 5�� ������ �������Ϳ� ������ ����
	     [0] <------------------ [v0]				      [ view0, view1, view2, view3, view4 ]
	     [1] <------------------ [v1]	                  [ view0, view1, view2, view3, view4 ]
	     [2] <------------------ [v2] ------------------> [ view0, view1, view2, view3, view4 ] vi�� ������ copy�ؼ� table ���·� ������ �����Ѵ�
	     [3] <------------------ [v3]	                  [ view0, view1, view2, view3, view4 ] �������� �����ϴ� ������ ����ȭ ������ �ذ��ϱ� ����
	     ... <------------------ ....	                  [ view0, view1, view2, view3, view4 ] (cmdQueue�� ��� �����͸� ������ �����Ƿ�)
				�ּҸ� �����Ѵ�       (CopyDescriptors()�� ��� ����)
									  (���� ����� [ v0, v1, v2, view3, view4 ] �̷� ����)
				                      (�� �κ��� Mesh���� TableDescriptorHeap���� ������ SetCBV, SetRBV �� �̿��ؼ� ����)
									  => Mesh���� Constantbuffer�� �ҷ��� pushdata�� �� �� �Բ� �ϵ��� ����

	���߿� Desc.Heap(Shader Visible)�� ( [ v0, v1, v2, view3, view4 ] )
	Game->Updage->Engine->RenderBegin->CommandQueue�� RenderBegin-> """SetDescriptorHeaps"""
	Game->Update->Mesh->Render->TableDescriptor�� CommitTable -> """SetGraphicsRootDescriptorTable()"""
    �Ϸ��� ������ ���� b0~b4 �������ͷ� ���޵ȴ�
	*/
}

void ConstantBuffer::CreateBuffer()
{
	uint32 bufferSize = _elementSize * _elementCount;
	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	DEVICE->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_cbvBuffer));

	_cbvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&_mappedBuffer));
	// We do not need to unmap until we are done with the resource.  However, we must not write to
	// the resource while it is in use by the GPU (so we must use synchronization techniques).
}

void ConstantBuffer::CreateView()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
	cbvDesc.NumDescriptors = _elementCount;	// constantbuffer�� ������ŭ �����ϰ�
	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&_cbvHeap));

	_cpuHandleBegin = _cbvHeap->GetCPUDescriptorHandleForHeapStart();
	_handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (uint32 i = 0; i < _elementCount; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		// v0�� constant buffer 0�� �ּҸ� ������ �־�� �Ѵ�
		// vi�� constant buffer 0�� �ּ� + (i * size)�� �ּҸ� ������ �־�� �Ѵ�
		cbvDesc.BufferLocation = _cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(_elementSize) * i;
		cbvDesc.SizeInBytes = _elementSize;   // CB size is required to be 256-byte aligned.

		DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
	}
}

void ConstantBuffer::Clear()
{
	_currentIndex = 0;
}

void ConstantBuffer::PushGraphicsData(void* buffer, uint32 size)
{
	assert(_currentIndex < _elementCount);
	assert(_elementSize == ((size + 255) & ~255));

	// buffer�� �ش� ��ġ�� ������ ������ �ֱ�
	::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);

	//D3D12_GPU_VIRTUAL_ADDRESS address = GetGpuVirtualAddress(_currentIndex);
	//CMD_LIST->SetGraphicsRootConstantBufferView(rootParamIndex, address);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);
	GEngine->GetGraphicsDescHeap()->SetCBV(cpuHandle, _reg);

	_currentIndex++;

	//return cpuHandle;
}

void ConstantBuffer::PushComputeData(void* buffer, uint32 size)
{
	assert(_currentIndex < _elementCount);
	assert(_elementSize == ((size + 255) & ~255));

	::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);
	GEngine->GetComputeDescHeap()->SetCBV(cpuHandle, _reg);

	_currentIndex++;
}

void ConstantBuffer::SetGraphicsGlobalData(void* buffer, uint32 size)
{
	assert(_elementSize == ((size + 255) & ~255));

	// buffer�� �ش� ��ġ�� ������ ������ �ֱ�
	::memcpy(&_mappedBuffer[0], buffer, size);

	GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(0, GetGpuVirtualAddress(0));
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGpuVirtualAddress(uint32 index)
{
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = _cbvBuffer->GetGPUVirtualAddress();
	objCBAddress += index * _elementSize;
	return objCBAddress;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetCpuHandle(uint32 index)
{
	// _cpuHandleBegin�̶�, Desc(CBV) Heap�� ù ���� �ּҸ� �ǹ��Ѵ�

	// return ptr(_cpuHandleBegin�� �ּ�) += index * desciptorIncrementSize
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(_cpuHandleBegin, index * _handleIncrementSize);
}