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

		// TODO-2.2: In range index 1 (the second range), initialize 2 SRV resources at register 1: indices and vertices of triangle data.
                // This will effectively put the indices at register 1, and the vertices at register 2.
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);

		// TODO-2.2: Initialize all the parameters of the GlobalRootSignature in their appropriate slots.
		//		* See GlobalRootSignature in RaytracingSceneDefines.h to understand what they are.
		// - The OutputView should correspond to the UAV range descriptor above (descriptor table), bound to register 0 of the UAV registers.
                // - The Index/Vertex Buffer should correspond to the SRV range (descriptor table) above, bound to registers 1 and 2 of the SRV registers.
                //      Note that since we initialize these as a range of size 2, then you should bind the entire range to register 1.
                //      This will automatically fill in registers 1 and 2.
		// - The AccelerationStructure should be init as SRV bound to register 0 of the SRV registers.
                // - The SceneConstant should be init as a ConstantBufferView (CBV) bound to register 0 of the CBV registers.
                // - The AABBAttributeBuffer should be init as SRV bound to register 3 of the SRV registers.
		// - Look up InitAsDescriptorTable(), InitAsShaderResourceView(), and InitAsConstantBuffer() in the DirectX documentation
		// to understand what to do.
                // - If you're ever unsure if the register mapping is correct, look at the top of Raytracing.hlsl.
                //      u registers --> UAV
                //      t registers --> SRV
                //      b registers --> CBV
		CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];
		rootParameters[GlobalRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[GlobalRootSignature::Slot::VertexBuffers].InitAsDescriptorTable(1, &ranges[1]);
		rootParameters[GlobalRootSignature::Slot::AccelerationStructure].InitAsShaderResourceView(0);
		rootParameters[GlobalRootSignature::Slot::SceneConstant].InitAsConstantBufferView(0);
		rootParameters[GlobalRootSignature::Slot::AABBattributeBuffer].InitAsShaderResourceView(3);

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

		// TODO-2.2: AABB geometry. Inspire yourself from the triangle local signature above to create an AABB local signature
		// - Remember that the AABB holds 1 slot for Material Constants, and another 1 for the geometry instance.
		// - See the AABB Definition in RaytracingSceneDefines.h to understand what this means.
		// - Use registers 1 and 2 of the CBVs for the AABB. Yes, althought the triangle MaterialConstant *also* maps
                //      to register 1, this overlap is allowed since we are talking about *local* root signatures 
		//      --> the values they hold will depend on the shader function the local signature is bound to!
		{
			namespace RootSignatureSlots = LocalRootSignature::AABB::Slot;
			CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
			rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 1);
			rootParameters[RootSignatureSlots::GeometryIndex].InitAsConstants(SizeOfInUint32(PrimitiveConstantBuffer), 2);

			CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
			localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
			SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::AABB]);
		}
	}
}
