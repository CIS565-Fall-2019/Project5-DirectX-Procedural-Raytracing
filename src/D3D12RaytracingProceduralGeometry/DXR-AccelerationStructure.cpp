#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-2.6, TODO-2.6: Build geometry descs for the bottom-level AS for the plane and for the procedural geometries in the scene.
// A bottom-level AS is a unique (or multiple) piece(s) of geometry, each for each type of object in your scene.
// Example: if you had 2 boxes in your scene, then you would make 1 bottom level AS for 1 box 
//			and then 2 different instances.
// In our project, we will have 2 BLAS: 1 for triangles, and 1 for procedurals.
// You will be building the GeometryDescriptor for a bottom-level AS here. A geometry descriptor describes
// how the geometry is laid out.
//	 *	For a triangle mesh, this tells us where the indices are, what's their type,
//		where the vertices are, etc...
//	 *	For a procedural geometry, this tells us the count of the AABBs, as well their size.
//		In this project, we will have 1 AABB geometry per procedural.
// Lookup the DXR documentation for D3D12_RAYTRACING_GEOMETRY_DESC to see how to fill in these structs
void DXProceduralProject::BuildGeometryDescsForBottomLevelAS(array<vector<D3D12_RAYTRACING_GEOMETRY_DESC>, BottomLevelASType::Count>& geometryDescs)
{
	D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	{
		// Triangle bottom-level AS contains a single plane geometry.
		geometryDescs[BottomLevelASType::Triangle].resize(1); // only 1 triangle-type geometry: the plane

		// TODO-2.6: Fill the triangle geometry desc.
		//  * Remember to use m_indexBuffer and m_vertexBuffer to get pointers to the data.
		//  * GPUVirtualAddresses can be accessed from a D3D12Resource using GetGPUVirtualAddress() (e.g m_vertexBuffer.resource->GetGPUVirtualAddress())
		//  * The *total size* of the buffer can be accessed from GetDesc().Width (e.g m_indexBuffer.resource->GetDesc().Width)
        //  * We filled in the format of the buffers to avoid confusion.
		auto& geometryDesc = geometryDescs[BottomLevelASType::Triangle][0];
		geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Flags = geometryFlags;
		geometryDesc.Triangles.Transform3x4 = NULL; // assuming vertices given world space positions
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.IndexCount = m_indexBuffer.resource->GetDesc().Width / sizeof(Index);
		geometryDesc.Triangles.VertexCount = m_vertexBuffer.resource->GetDesc().Width / sizeof(Vertex);
		geometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
	}

	{
		D3D12_RAYTRACING_GEOMETRY_DESC aabbDescTemplate = {};
		aabbDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
		aabbDescTemplate.AABBs.AABBCount = 1;
		aabbDescTemplate.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
		aabbDescTemplate.Flags = geometryFlags;

		// One AABB primitive per procedural geometry.
		geometryDescs[BottomLevelASType::AABB].resize(IntersectionShaderType::TotalPrimitiveCount, aabbDescTemplate);

		// TODO-2.6: Fill the AABB geometry desc for every procedural geometry.
		// Look-up what a D3D12_RAYTRACING_GEOMETRY_AABBS_DESC structs needs to be filled in correctly.
		// Remember to use m_aabbBuffer to get the AABB geometry data you previously filled in.
		// Note: Having separate geometries allows of separate shader record binding per geometry.
		//		 In this project, this lets us specify custom hit groups per AABB geometry.
		for (int i = 0; i < IntersectionShaderType::TotalPrimitiveCount; i++)
		{
			auto& desc = geometryDescs[BottomLevelASType::AABB][i];
			desc.AABBs.AABBs.StartAddress = m_aabbBuffer.resource->GetGPUVirtualAddress() + i * sizeof(D3D12_RAYTRACING_AABB);
		}
	}
}

// TODO-2.6: Given the geometry and the geometry descriptors, build the bottom-level acceleration structures.
AccelerationStructureBuffers DXProceduralProject::BuildBottomLevelAS(const vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();
	ComPtr<ID3D12Resource> scratch; // temporary AS data
	ComPtr<ID3D12Resource> bottomLevelAS; // actual bottom-level AS resource

	// This is what we need to fill in.
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};

	// The bottom-level inputs are the geometries you build in BuildGeometryDescsForBottomLevelAS()
	// Again, these tell the AS where the actual geometry data is and how it is laid out.
	// TODO-2.6: fill the bottom-level inputs. Consider using D3D12_ELEMENTS_LAYOUT_ARRAY as the DescsLayout.
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.Flags = buildFlags;
	bottomLevelInputs.NumDescs = geometryDescs.size();
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.pGeometryDescs = geometryDescs.data();

	// Query the driver for resource requirements to build an acceleration structure. We've done this for you.
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	}
	else // DirectX Raytracing
	{
		m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	}
	ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	// Create a scratch buffer as a UAV. This is like a temporary buffer that the driver needs to build the acceleration structure.
	AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	// Allocate resources for acceleration structures as a UAV --> this will prepare bottomLevelAS for us.
	// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
	// Default heap is OK since the application doesn�t need CPU read/write access to them. 
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
	//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
	//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
	{
		D3D12_RESOURCE_STATES initialResourceState;
		if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
		{
			initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
		}
		else // DirectX Raytracing
		{
			initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		}
		AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
	}

	// TODO-2.6: Now that you have the scratch and actual bottom-level AS desc, pass their GPU addresses to the bottomLevelBuildDesc.
	// Consider reading about D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC.
	// This should be as easy as passing the GPU addresses to the struct using GetGPUVirtualAddress() calls.
	bottomLevelBuildDesc.DestAccelerationStructureData = bottomLevelAS->GetGPUVirtualAddress();
	//bottomLevelBuildDesc.SourceAccelerationStructureData = bottomLevelAS->GetGPUVirtualAddress();
	bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();

	// Fill up the command list with a command that tells the GPU how to build the bottom-level AS.
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
		ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
		m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
		m_fallbackCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	}
	else // DirectX Raytracing
	{
		m_dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	}

	// TODO-2.6: After we finished building the bottom-level AS, save all the info in
	// the AccelerationStructureBuffers struct so the top-level AS can use it! 
	// Don't forget that this is the return value.
	// Consider looking into the AccelerationStructureBuffers struct in DXR-Structs.h
	AccelerationStructureBuffers asBuffer = {};
	asBuffer.accelerationStructure = bottomLevelAS;
	asBuffer.instanceDesc = nullptr; //not top level AS
	asBuffer.ResultDataMaxSizeInBytes = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
	asBuffer.scratch = scratch;

	return asBuffer;
}

// TODO-2.6: Build the instance descriptor for each bottom-level AS you built before.
// An instance descriptor will contain information on how these bottom-level ASes will be instanciated in the scene.
// Among other things, we will pass the world transformation matrix in the instance descriptor.
// InstanceDescType will either be D3D12_RAYTRACING_INSTANCE_DESC for real DXR or D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC for fallback layer.
// BLASPtrType will either be D3D12_GPU_VIRTUAL_ADDRESS for real DXR or WRAPPED_GPU_POINTER for fallback layer.
template <class InstanceDescType, class BLASPtrType>
void DXProceduralProject::BuildBottomLevelASInstanceDescs(BLASPtrType *bottomLevelASaddresses, ComPtr<ID3D12Resource>* instanceDescsResource)
{
	auto device = m_deviceResources->GetD3DDevice();

	vector<InstanceDescType> instanceDescs;
	instanceDescs.resize(NUM_BLAS);

	// Bottom-level AS for the plane instance.
	{
		// Make the plane a little larger than the actual number of primitives in each dimension.
		const XMUINT3 NUM_AABB = XMUINT3(700, 1, 700);
		const XMFLOAT3 fWidth = XMFLOAT3(
			NUM_AABB.x * c_aabbWidth + (NUM_AABB.x - 1) * c_aabbDistance,
			NUM_AABB.y * c_aabbWidth + (NUM_AABB.y - 1) * c_aabbDistance,
			NUM_AABB.z * c_aabbWidth + (NUM_AABB.z - 1) * c_aabbDistance);
		const XMVECTOR vWidth = XMLoadFloat3(&fWidth);

		auto& instanceDesc = instanceDescs[BottomLevelASType::Triangle];
		instanceDesc = {};
		instanceDesc.InstanceMask = 1;
		instanceDesc.InstanceContributionToHitGroupIndex = 0;
		instanceDesc.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::Triangle];

		// Calculate transformation matrix.
		// We multiply the width by -0.5 in the x,z plane because we want the middle of the plane
		// (which is currently expanded in the positive x,z plane) to be centered.
		const XMVECTOR vBasePosition = vWidth * XMLoadFloat3(&XMFLOAT3(-0.5f, 0.0f, -0.5f));

		// Scale in XZ dimensions.
		XMMATRIX mScale = XMMatrixScaling(fWidth.x, fWidth.y, fWidth.z);
		XMMATRIX mTranslation = XMMatrixTranslationFromVector(vBasePosition);
		XMMATRIX mTransform = mScale * mTranslation;

		// Store the transform in the instanceDesc.
		XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), mTransform);
	}

	// TODO-2.6: Create instanced bottom-level AS with procedural geometry AABBs.
	// * Make sure to set InstanceContributionToHitGroupIndex to beyond the shader records for the triangle AABB.
	//		For triangles, we have 1 shader record for radiance rays, and another for shadow rays.
	//		Where do you think procedural shader records would start then? Hint: right after.
	// * Make each instance hover above the ground by ~ half its width
	{
		auto& instanceDesc = instanceDescs[BottomLevelASType::AABB];
		instanceDesc = {};
		instanceDesc.InstanceMask = 1;
		instanceDesc.InstanceContributionToHitGroupIndex = 2;
		instanceDesc.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::AABB];

		const XMVECTOR hoverPos = XMLoadFloat3(&XMFLOAT3(0.0f, c_aabbWidth * 0.5f, 0.0f));
		XMMATRIX mTranslation = XMMatrixTranslationFromVector(hoverPos);
		// Store the transform in the instanceDesc.
		XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), mTranslation);
	}

	// Upload all these instances to the GPU, and make sure the resouce is set to instanceDescsResource.
	UINT64 bufferSize = static_cast<UINT64>(instanceDescs.size() * sizeof(instanceDescs[0]));
	AllocateUploadBuffer(device, instanceDescs.data(), bufferSize, &(*instanceDescsResource), L"InstanceDescs");
};

// TODO-2.6: Build the top-level acceleration structure.
// The top-level acceleration structure is a set of bottom-level *instances*. It is basically = scene.
// This should be very similar to BuildBottomLevelAS() except now we have to add in the instances!
AccelerationStructureBuffers DXProceduralProject::BuildTopLevelAS(AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count], D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();
	ComPtr<ID3D12Resource> scratch;
	ComPtr<ID3D12Resource> topLevelAS;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};

	// TODO-2.6: fill in the topLevelInputs, read about D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS.
	// Consider using D3D12_ELEMENTS_LAYOUT_ARRAY as a DescsLayout since we are using an array of bottom-level AS.
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = 2;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	//topLevelInputs.InstanceDescs = ; DO DOWN BELOW

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	}
	else // DirectX Raytracing
	{
		m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
	}
	ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	// TODO-2.6: Allocate a UAV buffer for the scracth/temporary top-level AS data.
	AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	// Allocate space for the top-level AS.
	{
		D3D12_RESOURCE_STATES initialResourceState;
		if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
		{
			initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
		}
		else // DirectX Raytracing
		{
			initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		}

		// TODO-2.6: Allocate a UAV buffer for the actual top-level AS.
		AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
	}

	// Note on Emulated GPU pointers (AKA Wrapped pointers) requirement in Fallback Layer:
	// The primary point of divergence between the DXR API and the compute-based Fallback layer is the handling of GPU pointers. 
	// DXR fundamentally requires that GPUs be able to dynamically read from arbitrary addresses in GPU memory. 
	// The existing Direct Compute API today is more rigid than DXR and requires apps to explicitly inform the GPU what 
	// blocks of memory it will access with SRVs/UAVs.
	// In order to handle the requirements of DXR, the Fallback Layer uses the concept of Emulated GPU pointers, 
	// which requires apps to create views around all memory they will access for raytracing, 
	// but retains the DXR-like flexibility of only needing to bind the top level acceleration structure at DispatchRays.
	//
	// The Fallback Layer interface uses WRAPPED_GPU_POINTER to encapsulate the underlying pointer
	// which will either be an emulated GPU pointer for the compute - based path or a GPU_VIRTUAL_ADDRESS for the DXR path.

	// Create instance descs for the bottom-level acceleration structures.
	// This should be as easy as calling BuildBottomLevelASInstanceDescs you completed above.
	ComPtr<ID3D12Resource> instanceDescsResource;
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDescs[BottomLevelASType::Count] = {};
		WRAPPED_GPU_POINTER bottomLevelASaddresses[BottomLevelASType::Count] =
		{
			CreateFallbackWrappedPointer(bottomLevelAS[0].accelerationStructure.Get(), static_cast<UINT>(bottomLevelAS[0].ResultDataMaxSizeInBytes) / sizeof(UINT32)),
			CreateFallbackWrappedPointer(bottomLevelAS[1].accelerationStructure.Get(), static_cast<UINT>(bottomLevelAS[1].ResultDataMaxSizeInBytes) / sizeof(UINT32))
		};

		// TODO-2.6: Call the fallback-templated version of BuildBottomLevelASInstanceDescs() you completed above.
		BuildBottomLevelASInstanceDescs<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC, WRAPPED_GPU_POINTER>(bottomLevelASaddresses, &instanceDescsResource);
	}
	else // DirectX Raytracing
	{
		D3D12_RAYTRACING_INSTANCE_DESC instanceDescs[BottomLevelASType::Count] = {};
		D3D12_GPU_VIRTUAL_ADDRESS bottomLevelASaddresses[BottomLevelASType::Count] =
		{
			bottomLevelAS[0].accelerationStructure->GetGPUVirtualAddress(),
			bottomLevelAS[1].accelerationStructure->GetGPUVirtualAddress()
		};

		// TODO-2.6: Call the DXR-templated version of BuildBottomLevelASInstanceDescs() you completed above.
		BuildBottomLevelASInstanceDescs<D3D12_RAYTRACING_INSTANCE_DESC, D3D12_GPU_VIRTUAL_ADDRESS>(bottomLevelASaddresses, &instanceDescsResource);
	}

	// Create a wrapped pointer to the acceleration structure.
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		UINT numBufferElements = static_cast<UINT>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
		m_fallbackTopLevelAccelerationStructurePointer = CreateFallbackWrappedPointer(topLevelAS.Get(), numBufferElements);
	}

	// TODO-2.6: fill in the topLevelBuildDesc. Read about D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC.
	// This should be as easy as passing the GPU addresses to the struct using GetGPUVirtualAddress() calls.
	topLevelInputs.InstanceDescs = instanceDescsResource->GetGPUVirtualAddress();
	topLevelBuildDesc.DestAccelerationStructureData = topLevelAS->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
	//topLevelBuildDesc.SourceAccelerationStructureData = topLevelAS->GetGPUVirtualAddress();

	// Build acceleration structure.
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		// Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
		ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
		m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
		m_fallbackCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	}
	else // DirectX Raytracing
	{
		m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	}

	// TODO-2.6: After we finished building the top-level AS, save all the info in the AccelerationStructureBuffers struct.
	// Very similar to how you did this in BuildBottomLevelAS() except now you have to worry about topLevelASBuffers.instanceDesc.
	// Consider looking into the AccelerationStructureBuffers struct in DXR-Structs.h.
	// Make sure to return the topLevelASBuffers before you exit the function.
	AccelerationStructureBuffers asBuffer = {};
	asBuffer.accelerationStructure = topLevelAS;
	asBuffer.instanceDesc = instanceDescsResource;
	asBuffer.ResultDataMaxSizeInBytes = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
	asBuffer.scratch = scratch;

	return asBuffer;
}

// TODO-2.6: This will wrap building the Acceleration Structure! This is what we will call when building our scene.
void DXProceduralProject::BuildAccelerationStructures()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();
	auto commandQueue = m_deviceResources->GetCommandQueue();
	auto commandAllocator = m_deviceResources->GetCommandAllocator();

	// Reset the command list for the acceleration structure construction.
	commandList->Reset(commandAllocator, nullptr);

	// TODO-2.6: Build the geometry descriptors. Hint: you filled in a function that does this.
	array<vector<D3D12_RAYTRACING_GEOMETRY_DESC>, BottomLevelASType::Count> geometryDescs;
	BuildGeometryDescsForBottomLevelAS(geometryDescs);

	// TODO-2.6: For each bottom-level object (triangle, procedural), build a bottom-level AS.
	// Hint: you filled in a function that does this.
	AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count];
	bottomLevelAS[0] = BuildBottomLevelAS(geometryDescs[0]);
	bottomLevelAS[1] = BuildBottomLevelAS(geometryDescs[1]);

	// Batch all resource barriers for bottom-level AS builds.
	// This will Notifies the driver that it needs to synchronize multiple accesses to resources.
	D3D12_RESOURCE_BARRIER resourceBarriers[BottomLevelASType::Count];
	for (UINT i = 0; i < BottomLevelASType::Count; i++)
	{
		resourceBarriers[i] = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS[i].accelerationStructure.Get());
	}
	commandList->ResourceBarrier(BottomLevelASType::Count, resourceBarriers);

	// TODO-2.6: Build top-level AS. Hint, you already made a function that does this.
	AccelerationStructureBuffers topLevelAS;
	topLevelAS = BuildTopLevelAS(bottomLevelAS);

	// Kick off acceleration structure construction.
	m_deviceResources->ExecuteCommandList();

	// Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
	m_deviceResources->WaitForGpu();

	// TODO-2.6: Store the AS buffers. The rest of the buffers will be released once we exit the function.
	// Do this for both the bottom-level and the top-level AS. Consider re-reading the DXProceduralProject class
	// to find what member variables should be set.
	m_bottomLevelAS[0] = bottomLevelAS[0].accelerationStructure;
	m_bottomLevelAS[1] = bottomLevelAS[1].accelerationStructure;
	m_topLevelAS = topLevelAS.accelerationStructure;
}