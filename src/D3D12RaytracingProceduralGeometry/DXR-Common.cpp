#include "stdafx.h"
#include "DXProceduralProject.h"

using namespace std;
using namespace DX;

// LOOKAT-1.8.1: Allocates an upload buffer. This will take care of transferring data from the CPU to the GPU, and is typically used
// for data that doesn't change over time. Best use cases are for triangle data (indices + vertices)
void DXProceduralProject::AllocateUploadBuffer(ID3D12Device* pDevice, void *pData, UINT64 datasize, 
												ID3D12Resource **ppResource, 
												const wchar_t* resourceName)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
	void *pMappedData;
	(*ppResource)->Map(0, nullptr, &pMappedData);
	memcpy(pMappedData, pData, datasize);
	(*ppResource)->Unmap(0, nullptr);
}

// LOOKAT-1.8.1: Allocates space for a UniformAccessView (UAV) that will be used by shaders.
// Typically this is used for textures or acceleration structures in DXR.
void DXProceduralProject::AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize,
												ID3D12Resource **ppResource, 
												D3D12_RESOURCE_STATES initialResourceState, 
												const wchar_t* resourceName)
{
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(ppResource)));
	if (resourceName)
	{
		(*ppResource)->SetName(resourceName);
	}
}

// LOOKAT-1.8.1: Allocate a descriptor and return its index.
// We set the application to support some max number of allocated descriptors, but we can dynamically grow this number:
// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
UINT DXProceduralProject::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
	auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
	{
		ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
		descriptorIndexToUse = m_descriptorsAllocated++;
	}
	*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
	return descriptorIndexToUse;
}

// LOOKAT-1.8.1: Create a ShaderResourceView (SRV) for a buffer. An SRV is best used for data like indices and vertices.
UINT DXProceduralProject::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
{
	auto device = m_deviceResources->GetD3DDevice();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	if (elementSize == 0)
	{
		// Raw byte data
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else
	{
		// A structured buffer with a fixed byte stride
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
	// Tell the device where to find the data, how to use it (descriptor), where it lives on the CPU.
	device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
	
	// Give back a GPU pointer/handle for this descriptor.
	buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
	return descriptorIndex;
}

// Create a wrapped pointer for the Fallback Layer path.
WRAPPED_GPU_POINTER DXProceduralProject::CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements)
{
	auto device = m_deviceResources->GetD3DDevice();

	D3D12_UNORDERED_ACCESS_VIEW_DESC rawBufferUavDesc = {};
	rawBufferUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	rawBufferUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	rawBufferUavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	rawBufferUavDesc.Buffer.NumElements = bufferNumElements;

	D3D12_CPU_DESCRIPTOR_HANDLE bottomLevelDescriptor;

	// Only compute fallback requires a valid descriptor index when creating a wrapped pointer.
	UINT descriptorHeapIndex = 0;
	if (!m_fallbackDevice->UsingRaytracingDriver())
	{
		descriptorHeapIndex = AllocateDescriptor(&bottomLevelDescriptor);
		device->CreateUnorderedAccessView(resource, nullptr, &rawBufferUavDesc, bottomLevelDescriptor);
	}
	return m_fallbackDevice->GetWrappedPointerSimple(descriptorHeapIndex, resource->GetGPUVirtualAddress());
}
