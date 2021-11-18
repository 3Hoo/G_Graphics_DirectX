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

	// 상수 버퍼는 256 바이트 배수로 만들어야 한다
	// 0 256 512 768
	_elementSize = (size + 255) & ~255;	// ~255 = ... 1111 1111 0000 0000
										// 즉 256배수가 되지 않는 하위 부분을 비트곱해서 날려버린다
										// 단, 반올림을 하기 위해 먼저 +255을 한다
	_elementCount = count;

	CreateBuffer();
	CreateView(); // -------------			
	//				             |
	/*                           v
	ConstantBuffer			Desc(CBV) Heap				  Desc. Heap(Shader Visible) b0~b4 총 5개 각각의 레지스터에 전달할 내용
	     [0] <------------------ [v0]				      [ view0, view1, view2, view3, view4 ]
	     [1] <------------------ [v1]	                  [ view0, view1, view2, view3, view4 ]
	     [2] <------------------ [v2] ------------------> [ view0, view1, view2, view3, view4 ] vi의 내용을 copy해서 table 형태로 여러개 저장한다
	     [3] <------------------ [v3]	                  [ view0, view1, view2, view3, view4 ] 여러개로 저장하는 이유는 동기화 문제를 해결하기 위해
	     ... <------------------ ....	                  [ view0, view1, view2, view3, view4 ] (cmdQueue는 즉시 데이터를 보내지 않으므로)
				주소를 저장한다       (CopyDescriptors()로 즉시 매핑)
									  (매핑 결과는 [ v0, v1, v2, view3, view4 ] 이런 형태)
				                      (이 부분은 Mesh에서 TableDescriptorHeap에서 정의한 SetCBV, SetRBV 를 이용해서 구현)
									  => Mesh에서 Constantbuffer를 불러와 pushdata를 할 때 함께 하도록 수정

	나중에 Desc.Heap(Shader Visible)가 ( [ v0, v1, v2, view3, view4 ] )
	Game->Updage->Engine->RenderBegin->CommandQueue의 RenderBegin-> """SetDescriptorHeaps"""
	Game->Update->Mesh->Render->TableDescriptor의 CommitTable -> """SetGraphicsRootDescriptorTable()"""
    일련의 과정을 통해 b0~b4 레지스터로 전달된다
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
	cbvDesc.NumDescriptors = _elementCount;	// constantbuffer의 갯수만큼 동일하게
	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&_cbvHeap));

	_cpuHandleBegin = _cbvHeap->GetCPUDescriptorHandleForHeapStart();
	_handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (uint32 i = 0; i < _elementCount; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		// v0는 constant buffer 0의 주소를 가지고 있어야 한다
		// vi는 constant buffer 0의 주소 + (i * size)의 주소를 가지고 있어야 한다
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

	// buffer의 해당 위치에 실제로 데이터 넣기
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

	// buffer의 해당 위치에 실제로 데이터 넣기
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
	// _cpuHandleBegin이란, Desc(CBV) Heap의 첫 시작 주소를 의미한다

	// return ptr(_cpuHandleBegin의 주소) += index * desciptorIncrementSize
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(_cpuHandleBegin, index * _handleIncrementSize);
}