#pragma once

// LOOKAT-1.6: a header containing definitions for various types of buffers/structs needed
// for CPU-GPU communication. We recommend reading this file at least once to get an overview of what
// type of data you'll be dealing with.

// LOOKAT-1.6: Struct defining an Acceleration Structure (Bottom Level or Top Leve)
struct AccelerationStructureBuffers
{
	ComPtr<ID3D12Resource> scratch;						// temp data needed by the GPU to build the AS
	ComPtr<ID3D12Resource> accelerationStructure;		// actual AS data
	ComPtr<ID3D12Resource> instanceDesc;				// used only for top-level AS
	UINT64                 ResultDataMaxSizeInBytes;	// this can be acquired from the prebuild info
};

// LOOKAT-1.6: Think of this as data that needs to be used by a shader. e.g: vertex data (position, color, normals)
struct D3DBuffer
{
	ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle; // this is a pointer to the CPU descriptor
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle; // this is a pointer to the GPU descriptor
};

// LOOKAT-1.6: A resource that needs to be written to the GPU physical memory.
// See ConstantBuffer, StructuredBuffer, and ShaderTable below for how the GpuUploadBuffer methods are used.
class GpuUploadBuffer
{
public:
	ComPtr<ID3D12Resource> GetResource() { return m_resource; }
	virtual void Release() { m_resource.Reset(); }
protected:
	ComPtr<ID3D12Resource> m_resource;

	GpuUploadBuffer() {}
	~GpuUploadBuffer()
	{
		if (m_resource.Get())
		{
			m_resource->Unmap(0, nullptr);
		}
	}

	// LOOKAT-1.6: tells the D3D12 device to create a Committed Resource usable by the GPU.
	void Allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		ThrowIfFailed(device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource)));
		m_resource->SetName(resourceName);
	}

	// LOOKAT: Maps the GpuUploadBuffer data onto the CPU for writing only (!)
	uint8_t* MapCpuWriteOnly()
	{
		uint8_t* mappedData;
		// We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
		return mappedData;
	}
};

// LOOKAT-1.6: Create and update a constant buffer with proper constant buffer alignments.
// Usage:
//	  Creation + Allocation:
//		ConstantBuffer<...> cb; // declaration
//		cb.Create(...);	// allocation + cpu mapping
//		cb.staging.var = ... ; | cb->var = ... ;
//	  Uploading to the GPU:
//		cb.CopyStagingToGPU(...);
//    Execution:
//		Set...View(..., cb.GputVirtualAddress());
template <class T>
class ConstantBuffer : public GpuUploadBuffer
{
	uint8_t* m_mappedConstantData;
	UINT m_alignedInstanceSize;
	UINT m_numInstances;

public:
	ConstantBuffer() : m_alignedInstanceSize(0), m_numInstances(0), m_mappedConstantData(nullptr) {}

	void Create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
	{
		m_numInstances = numInstances;
		m_alignedInstanceSize = Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		UINT bufferSize = numInstances * m_alignedInstanceSize;
		Allocate(device, bufferSize, resourceName);
		m_mappedConstantData = MapCpuWriteOnly();
	}

	void CopyStagingToGpu(UINT instanceIndex = 0)
	{
		memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
	}

	// Accessors
	T staging;
	T* operator->() { return &staging; }
	UINT NumInstances() { return m_numInstances; }
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
	{
		return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
	}
};

// LOOKAT-1.6: create and update a structured buffer.
// Usage: 
//    StructuredBuffer<...> sb;
//    sb.Create(...);
//    sb[index].var = ... ; 
//    sb.CopyStagingToGPU(...);
//    Set...View(..., sb.GputVirtualAddress());
template <class T>
class StructuredBuffer : public GpuUploadBuffer
{
	T* m_mappedBuffers;
	std::vector<T> m_staging;
	UINT m_numInstances;

public:
	// Performance tip: Align structures on sizeof(float4) boundary.
	static_assert(sizeof(T) % 16 == 0, L"Align structure buffers on 16 byte boundary for performance reasons.");

	StructuredBuffer() : m_mappedBuffers(nullptr), m_numInstances(0) {}

	void Create(ID3D12Device* device, UINT numElements, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
	{
		m_staging.resize(numElements);
		UINT bufferSize = numInstances * numElements * sizeof(T);
		Allocate(device, bufferSize, resourceName);
		m_mappedBuffers = reinterpret_cast<T*>(MapCpuWriteOnly());
	}

	void CopyStagingToGpu(UINT instanceIndex = 0)
	{
		memcpy(m_mappedBuffers + instanceIndex * NumElementsPerInstance(), &m_staging[0], InstanceSize());
	}

	// Accessors
	T& operator[](UINT elementIndex) { return m_staging[elementIndex]; }
	size_t NumElementsPerInstance() { return m_staging.size(); }
	UINT NumInstances() { return m_staging.size(); }
	size_t InstanceSize() { return NumElementsPerInstance() * sizeof(T); }
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
	{
		return m_resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize();
	}
};

// LOOKAT-1.6: Shader record = {{Shader ID}, {RootArguments}}
class ShaderRecord
{
public:
	ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
		shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
	{
	}

	ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
		shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
		localRootArguments(pLocalRootArguments, localRootArgumentsSize)
	{
	}

	void CopyTo(void* dest) const
	{
		uint8_t* byteDest = static_cast<uint8_t*>(dest);
		memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
		if (localRootArguments.ptr)
		{
			memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
		}
	}

	struct PointerWithSize {
		void *ptr;
		UINT size;

		PointerWithSize() : ptr(nullptr), size(0) {}
		PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
	};
	PointerWithSize shaderIdentifier;
	PointerWithSize localRootArguments;
};

// LOOKAT-1.6: Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}
class ShaderTable : public GpuUploadBuffer
{
	uint8_t* m_mappedShaderRecords;
	UINT m_shaderRecordSize;

	// Debug support
	std::wstring m_name;
	std::vector<ShaderRecord> m_shaderRecords;

	ShaderTable() {}
public:
	ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr)
		: m_name(resourceName)
	{
		m_shaderRecordSize = Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		m_shaderRecords.reserve(numShaderRecords);
		UINT bufferSize = numShaderRecords * m_shaderRecordSize;
		Allocate(device, bufferSize, resourceName);
		m_mappedShaderRecords = MapCpuWriteOnly();
	}

	void push_back(const ShaderRecord& shaderRecord)
	{
		ThrowIfFalse(m_shaderRecords.size() < m_shaderRecords.capacity());
		m_shaderRecords.push_back(shaderRecord);
		shaderRecord.CopyTo(m_mappedShaderRecords);
		m_mappedShaderRecords += m_shaderRecordSize;
	}

	UINT GetShaderRecordSize() { return m_shaderRecordSize; }

	// Pretty-print the shader records.
	void DebugPrint(std::unordered_map<void*, std::wstring> shaderIdToStringMap)
	{
		std::wstringstream wstr;
		wstr << L"|--------------------------------------------------------------------\n";
		wstr << L"|Shader table - " << m_name.c_str() << L": "
			<< m_shaderRecordSize << L" | "
			<< m_shaderRecords.size() * m_shaderRecordSize << L" bytes\n";

		for (UINT i = 0; i < m_shaderRecords.size(); i++)
		{
			wstr << L"| [" << i << L"]: ";
			wstr << shaderIdToStringMap[m_shaderRecords[i].shaderIdentifier.ptr] << L", ";
			wstr << m_shaderRecords[i].shaderIdentifier.size << L" + " << m_shaderRecords[i].localRootArguments.size << L" bytes \n";
		}
		wstr << L"|--------------------------------------------------------------------\n";
		wstr << L"\n";
		OutputDebugStringW(wstr.str().c_str());
	}
};