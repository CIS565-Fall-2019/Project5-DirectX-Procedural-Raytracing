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
	commandList->SetComputeRootConstantBufferView(
		GlobalRootSignature::Slot::AABBattributeBuffer, 
		m_aabbPrimitiveAttributeBuffer.GpuVirtualAddress(frameIndex)
	);

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

	// We do indicies because verticies are just one register over? So we get register 1 + register 2
	// In CreateRootSignatures() we bound the data to m_raytracingGlobalRootSignature. That buffer
	// has the rootSignature for each index/vertex
	commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

	// TODO-2.8: Bind the OutputView (basically m_raytracingOutputResourceUAVGpuDescriptor). Very similar to the Index/Vertex buffer.
	m_raytracingOutputResourceUAVGpuDescriptor; // Its a descriptor handle, so probably root descriptor table?
	commandList->SetComputeRootDescriptorTable(
		GlobalRootSignature::Slot::OutputView,
		m_raytracingOutputResourceUAVGpuDescriptor);

	// This will define a `DispatchRays` function that takes in a command list, a pipeline state, and a descriptor
	// This will set the hooks using the shader tables built before and call DispatchRays on the command list
	auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject, auto* dispatchDesc)
	{
		// You will fill in a D3D12_DISPATCH_RAYS_DESC (which is dispatchDesc).
		// TODO-2.8: fill in dispatchDesc->HitGroupTable. Look up the struct D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE
		D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE hgt;
		dispatchDesc->HitGroupTable.StartAddress  = m_hitGroupShaderTable->GetGPUVirtualAddress(); // Want to start here, makes sense.
		dispatchDesc->HitGroupTable.SizeInBytes   = m_hitGroupShaderTable->GetDesc().Width; // Similar to before
		dispatchDesc->HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes; // Makes sense...

		// TODO-2.8: now fill in dispatchDesc->MissShaderTable
		// Similar to above? They're all very simialr structs
		dispatchDesc->MissShaderTable.StartAddress  = m_missShaderTable->GetGPUVirtualAddress();
		dispatchDesc->MissShaderTable.SizeInBytes   = m_missShaderTable->GetDesc().Width;
		dispatchDesc->MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;

		// TODO-2.8: now fill in dispatchDesc->RayGenerationShaderRecord
		// This one is different, It doesn't have a stride
		// Which makes sense. There is only one object represented here.
		// No other object to stride to
		dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
		dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;

		// We do this for you. This will define how many threads will be dispatched. Basically like a blockDims in CUDA!
		// 2d!
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