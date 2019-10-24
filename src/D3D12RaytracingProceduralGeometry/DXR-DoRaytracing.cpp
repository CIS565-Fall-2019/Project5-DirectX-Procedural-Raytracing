#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-2.8, TODO-2.8: The final nail in the coffin. This will finalize everything you've done so far.
// This part will tell the GPU all about the nice shader tables you built, the triangle buffers you made and uploaded,
// as well as the acceleration structure you made - and actually tells the GPU to do the raytracing!
void DXProceduralProject::DoRaytracing()
{
	auto commandList = m_deviceResources->GetCommandList();
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

	// Copy dynamic buffers to GPU. The dynamic buffers in question are SceneConstant and AABBattributeBuffer.
	// (1) copy the data to the GPU
	// (2) tell the commandList to SetComputeRootShaderResourceView() or SetComputeRootConstantBufferView()
	m_sceneCB.CopyStagingToGpu(frameIndex);
	commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));

	// TODO-2.8: do a very similar operation for the m_aabbPrimitiveAttributeBuffer
	m_aabbPrimitiveAttributeBuffer.CopyStagingToGpu(frameIndex);
	commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AABBattributeBuffer, m_aabbPrimitiveAttributeBuffer.GpuVirtualAddress(frameIndex));

	// Bind the descriptor heaps.
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		m_fallbackCommandList.Get()->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());

	}
	else // DirectX Raytracing
	{
		commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	}

	// Bind the acceleration structure.
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		m_fallbackCommandList->SetTopLevelAccelerationStructure(GlobalRootSignature::Slot::AccelerationStructure, m_fallbackTopLevelAccelerationStructurePointer);
	}
	else // DirectX Raytracing
	{
		commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS->GetGPUVirtualAddress());
	}

	// TODO-2.8: Bind the Index/Vertex buffer (basically m_indexBuffer. Think about why this isn't m_vertexBuffer too. Hint: CreateRootSignatures() in DXR-Pipeline.cpp.)
	// This should be done by telling the commandList to SetComputeRoot*(). You just have to figure out what * is.
	// Example: in the case of GlobalRootSignature::Slot::SceneConstant above, we used SetComputeRootConstantBufferView()
	// Hint: look at CreateRootSignatures() in DXR-Pipeline.cpp.
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::VertexBuffers, m_indexBuffer.gpuDescriptorHandle);

	// TODO-2.8: Bind the OutputView (basically m_raytracingOutputResourceUAVGpuDescriptor). Very similar to the Index/Vertex buffer.
	commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutputResourceUAVGpuDescriptor);

	// This will define a `DispatchRays` function that takes in a command list, a pipeline state, and a descriptor
	// This will set the hooks using the shader tables built before and call DispatchRays on the command list
	auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject, auto* dispatchDesc)
	{
		// You will fill in a D3D12_DISPATCH_RAYS_DESC (which is dispatchDesc).
		// TODO-2.8: fill in dispatchDesc->HitGroupTable. Look up the struct D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE 
		auto& hitgroup = dispatchDesc->HitGroupTable;
		hitgroup = {};
		hitgroup.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
		hitgroup.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
		hitgroup.StrideInBytes = m_hitGroupShaderTableStrideInBytes;

		// TODO-2.8: now fill in dispatchDesc->MissShaderTable
		auto& miss = dispatchDesc->MissShaderTable;
		miss = {};
		miss.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
		miss.SizeInBytes = m_missShaderTable->GetDesc().Width;
		miss.StrideInBytes = m_missShaderTableStrideInBytes;

		// TODO-2.8: now fill in dispatchDesc->RayGenerationShaderRecord
		auto& raygen = dispatchDesc->RayGenerationShaderRecord;
		raygen = {};
		raygen.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
		raygen.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;;

		// We do this for you. This will define how many threads will be dispatched. Basically like a blockDims in CUDA!
		dispatchDesc->Width = m_width;
		dispatchDesc->Height = m_height;
		dispatchDesc->Depth = 1;

		// This will tell the raytracing command list that you created an RTPSO that binds all of this stuff together.
		raytracingCommandList->SetPipelineState1(stateObject);

		// Kick off raytracing
		m_gpuTimers[GpuTimers::Raytracing].Start(commandList);
		raytracingCommandList->DispatchRays(dispatchDesc);
		m_gpuTimers[GpuTimers::Raytracing].Stop(commandList);
	};

	// Use the DispatchRays() function you created.
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		DispatchRays(m_fallbackCommandList.Get(), m_fallbackStateObject.Get(), &dispatchDesc);
	}
	else // DirectX Raytracing
	{
		DispatchRays(m_dxrCommandList.Get(), m_dxrStateObject.Get(), &dispatchDesc);
	}
}