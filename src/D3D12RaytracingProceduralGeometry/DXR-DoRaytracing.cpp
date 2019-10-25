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
    // should we use CBV or SRV for the primitive attribute? I think it is just material, so using CBV is fine
    m_aabbPrimitiveAttributeBuffer.CopyStagingToGpu(frameIndex);
    commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::AABBattributeBuffer, m_aabbPrimitiveAttributeBuffer.GpuVirtualAddress(frameIndex));

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

	// TODO-2.8: Bind the Index/Vertex buffer (basically m_indexBuffer. Think about why this isn't m_vertexBuffer too. Hint: CreateRootSignatures() in DXR-RootSignature.cpp.)
        //because the vertex buffer is a global signature which every shader can access to
	// This should be done by telling the commandList to SetComputeRoot*(). You just have to figure out what * is.
	// Example: in the case of GlobalRootSignature::Slot::SceneConstant above, we used SetComputeRootConstantBufferView()
	// Hint: look at CreateRootSignatures() in DXR-RootSignature.cpp.
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::VertexBuffers, m_indexBuffer.gpuDescriptorHandle); //we use the index buffer to bind vertex buffer?


	// TODO-2.8: Bind the OutputView (basically m_raytracingOutputResourceUAVGpuDescriptor). Very similar to the Index/Vertex buffer.
    // outputView doesn't have a buffer
    commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutputResourceUAVGpuDescriptor);

	// This will define a `DispatchRays` function that takes in a command list, a pipeline state, and a descriptor
	// This will set the hooks using the shader tables built before and call DispatchRays on the command list
	auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject, auto* dispatchDesc)
	{
		// You will fill in a D3D12_DISPATCH_RAYS_DESC (which is dispatchDesc).
		// TODO-2.8: fill in dispatchDesc->HitGroupTable. Look up the struct D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE 
        // how do we utilize the stateObject to populate hitGroup, missShader and RayGenerationShader?
        //D3D12_Shader_Identifier_size_in_bytes + maxrootargumentsSize
        //everyone group should have a hitgroup shader -- triangles * rayType count + primitiveType * rayType count
        dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable.Get()->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + LocalRootSignature::MaxRootArgumentsSize();
        dispatchDesc->HitGroupTable.SizeInBytes = dispatchDesc->HitGroupTable.StrideInBytes * (RayType::Count + IntersectionShaderType::Count * RayType::Count);
		// TODO-2.8: now fill in dispatchDesc->MissShaderTable
		//m_missShaderTable
        dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable.Get()->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        dispatchDesc->MissShaderTable.SizeInBytes = dispatchDesc->HitGroupTable.StrideInBytes * 2;
		// TODO-2.8: now fill in dispatchDesc->RayGenerationShaderRecord
		//m_RayGenerationShaderTable
		//only start address and size
        dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable.Get()->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		

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