#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

// LOOKAT-2.2: Read about root signatures here https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signatures-overview

// TODO-2.2: Create parts of the shader root signature.
void DXProceduralProject::CreateRootSignatures()
{
	auto device = m_deviceResources->GetD3DDevice();

	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	// Think of them as global function parameters.
	{
		// A descriptor range is like an array of descriptors of the same type (UAV, SRV, CBV).
		// The range has the following data: type of resource accessed, the number of descriptors, and the base register the shader
		// should look at to access these resources.
		CD3DX12_DESCRIPTOR_RANGE ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture

		// TODO-2.2: In range index 1 (the second range), initialize 2 SRV resources at register 1.
		// We will have 1 SRV resource for the vertices, and another for the indices.

		// TODO-2.2: Initialize all the parameters of the GlobalRootSignature in their appropriate slot.
		//		* These are the actual global root parameters. 
		//		* See GlobalRootSignature in RaytracingSceneDefines.h to understand what they are.
		// - The OutputView and the VertexBuffers should correspond to the UAV (UniformAccessView) descriptor above and 
		//   the SRV (ShaderResourceView) descriptors above respectively.
		// - As for the AccelerationStructure, SceneConstant, and AABBAttributeBuffer, they should be:
		//	 SRV, ConstantBufferView (CBV), and SRV respectively.
		// - Look up InitAsDescriptorTable(), InitAsShaderResourceView(), and InitAsConstantBuffer() in the DirectX documentation
		// to understand what to do.
		CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];

		// Finally, we bundle up all the descriptors you filled up and tell the device to create this global root signature!
		CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	// Think of them as local function parameters.
	{
		// Triangle geometry
		{
			namespace RootSignatureSlots = LocalRootSignature::Triangle::Slot;
			CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
			rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 1);

			CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
			localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
			SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle]);
		}

		// TODO-2.2: AABB geometry. Inspire yourself from the triangle code local signature above to create an AABB local signature
		// Remember that the AABB holds 1 slot for Material Constants, and another for the geometry instance.
		// See the AABB Definition in RaytracingSceneDefines.h to understand what this means.
		// Use registers 1 and 2 for the AABB.
		{
			
		}
	}
}