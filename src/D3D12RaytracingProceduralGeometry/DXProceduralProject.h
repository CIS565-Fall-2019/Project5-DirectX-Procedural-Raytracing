#pragma once

//	LOOKAT-1.7: !!!!Read this entire file at least once. You are essentially implementing this class!!!
//	NOTE: The implementation of this class is scattered in files that are named DXR-*.cpp
//	* Class that builds the rendering pipeline and executes it.
//	* Subclasses from DXProject. Read DXProject.h before venturing here.
//	* This where most of the CPU-side code is declared.
//	* Used by the main application in Win32Application.cpp (see the WindowProc()) method) to update the app state.

#include "DXProject.h"
#include "StepTimer.h"
#include "RaytracingSceneDefines.h"
#include "DirectXRaytracingHelper.h"
#include "PerformanceTimers.h"

// Fallback Layer uses DirectX Raytracing if a driver and OS supports it. 
// Otherwise, it falls back to compute pipeline to emulate raytracing.
class DXProceduralProject : public DXProject
{
    enum class RaytracingAPI {
        FallbackLayer,
        DirectXRaytracing,
    };

public:
    DXProceduralProject(UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // DXR-AppLifeCycle.cpp: available to use from Win32Application.
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnDestroy();

    virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

private:
    static const UINT FrameCount = 3;

	// LOOKAT-1.7: Carefully read ALL the private variables of this class to understand 
	// the state it is responsible for.

    // Scene constants
    const UINT NUM_BLAS = 2;          // BLAS = Bottom-Level Acceleration Structure. We have 2: Triangle + AABB (AABB = Axis-Aligned Bounding Box)
    const float c_aabbWidth = 2;      // AABB width.
    const float c_aabbDistance = 2;   // Distance between AABBs.

    // Raytracing Fallback Layer (FL) attributes
	ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice; // 1 device per project
    ComPtr<ID3D12RaytracingFallbackCommandList> m_fallbackCommandList;	// >= command lists per project
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackStateObject; // holds various state attributes for the pipeline

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;
    ComPtr<ID3D12StateObject> m_dxrStateObject;
    bool m_isDxrSupported;

    // Root/Global signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature[LocalRootSignature::Type::Count];

    // Descriptors. We use 1 descriptor heap for everything for simplicity
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated;
    UINT m_descriptorSize;

    // Global root, scene arguments
    ConstantBuffer<SceneConstantBuffer> m_sceneCB; // holds multiple scene states per frame (camera is an example)
    StructuredBuffer<PrimitiveInstancePerFrameBuffer> m_aabbPrimitiveAttributeBuffer; // holds multiple primitives
    std::vector<D3D12_RAYTRACING_AABB> m_aabbs; // holds 1 AABB per procedural object (e.g 1 for metaballs, 1 for a sphere, etc..)

    // Local root constant buffers
    PrimitiveConstantBuffer m_planeMaterialCB;
    PrimitiveConstantBuffer m_aabbMaterialCB[IntersectionShaderType::TotalPrimitiveCount];

    // Geometry
    D3DBuffer m_indexBuffer;
    D3DBuffer m_vertexBuffer;
    D3DBuffer m_aabbBuffer;

    // Acceleration structure
    ComPtr<ID3D12Resource> m_bottomLevelAS[BottomLevelASType::Count];
    ComPtr<ID3D12Resource> m_topLevelAS; // if DXR
	WRAPPED_GPU_POINTER m_fallbackTopLevelAccelerationStructurePointer; // else, we use this one for the FL

    // Raytracing output (i.e a frame buffer)
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    // Shader names
    static const wchar_t* c_hitGroupNames_TriangleGeometry[RayType::Count];
    static const wchar_t* c_hitGroupNames_AABBGeometry[IntersectionShaderType::Count][RayType::Count];
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_intersectionShaderNames[IntersectionShaderType::Count];
    static const wchar_t* c_closestHitShaderNames[GeometryType::Count];
    static const wchar_t* c_missShaderNames[RayType::Count];
	
	// Shader tables
    ComPtr<ID3D12Resource> m_missShaderTable; // points to the miss shader table resource
    UINT m_missShaderTableStrideInBytes;
    ComPtr<ID3D12Resource> m_hitGroupShaderTable; // points to the hit group shader table resource
    UINT m_hitGroupShaderTableStrideInBytes;
    ComPtr<ID3D12Resource> m_rayGenShaderTable; // points to the raygen shader table resource

    // Application state
    DX::GPUTimer m_gpuTimers[GpuTimers::Count];
	StepTimer m_timer;
    RaytracingAPI m_raytracingAPI;
    bool m_forceComputeFallback;
    float m_animateGeometryTime; // Animation
    bool m_animateGeometry;
    bool m_animateCamera;
    bool m_animateLight;
    XMVECTOR m_eye; // Camera
    XMVECTOR m_at;
    XMVECTOR m_up;

private:
	// DXR-DescriptorHeap.cpp
	void CreateDescriptorHeap();
    
    // DXR-DynamicBuffers.cpp
	void InitializeScene();
	void CreateConstantBuffers();
	void CreateAABBPrimitiveAttributesBuffers();
	void UpdateCameraMatrices();
	void UpdateAABBPrimitiveAttributes(float animationTime);

	// DXR-RootSignature.cpp
	void CreateRootSignatures();

	// DXR-HitGroup.cpp
	void CreateHitGroupSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);
	void CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);

	// DXR-Pipeline.cpp
	void CreateRaytracingPipelineStateObject();

	// DXR-Geometry.cpp
	void BuildPlaneGeometry();
	void BuildProceduralGeometryAABBs();
	void BuildGeometry();

	// DXR-AccelerationStructure.cpp
	void BuildGeometryDescsForBottomLevelAS(std::array<std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>, BottomLevelASType::Count>& geometryDescs);
	AccelerationStructureBuffers BuildBottomLevelAS(const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
	template <class InstanceDescType, class BLASPtrType>
	void BuildBottomLevelASInstanceDescs(BLASPtrType *bottomLevelASaddresses, ComPtr<ID3D12Resource>* instanceDescsResource);
	AccelerationStructureBuffers BuildTopLevelAS(AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count], D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
	void BuildAccelerationStructures();

	// DXR-ShaderTable.cpp
	void BuildShaderTables();

	// DXR-DoRaytracing.cpp
	void DoRaytracing();

	// DXR-Common.cpp
	void AllocateUploadBuffer(ID3D12Device* pDevice, void *pData, UINT64 datasize, ID3D12Resource **ppResource,
		const wchar_t* resourceName = nullptr);
	void AllocateUAVBuffer(ID3D12Device* pDevice, UINT64 bufferSize, ID3D12Resource **ppResource,
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr);
	UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
	UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
	WRAPPED_GPU_POINTER CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements);
    
	// DXR-Other.cpp
	void CreateRaytracingInterfaces();
	void CreateDeviceDependentResources();
	void CreateAuxilaryDeviceResources();
	void CreateWindowSizeDependentResources();
	void CreateDxilLibrarySubobject(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);
	void CreateRaytracingOutputResource();
	void RecreateD3D();
	void ReleaseDeviceDependentResources();
	void ReleaseWindowSizeDependentResources();
	void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
	void SelectRaytracingAPI(RaytracingAPI type);
	void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CalculateFrameStats();
	void EnableDirectXRaytracing(IDXGIAdapter1* adapter);
	void ParseCommandLineArgs(WCHAR* argv[], int argc);
};
