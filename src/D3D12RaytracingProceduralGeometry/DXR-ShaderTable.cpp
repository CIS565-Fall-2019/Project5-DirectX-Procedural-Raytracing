#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-2.7, TODO-2.7: Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void DXProceduralProject::BuildShaderTables()
{
	auto device = m_deviceResources->GetD3DDevice();

	void* rayGenShaderID;
	void* missShaderIDs[RayType::Count];
	void* hitGroupShaderIDs_TriangleGeometry[RayType::Count];
	void* hitGroupShaderIDs_AABBGeometry[IntersectionShaderType::Count][RayType::Count];

	// A shader name look-up table for shader table debug print out.
	unordered_map<void*, wstring> shaderIdToStringMap;

	// The state object can be thought of a pipeline state object that holds application shader information.
	// Remember that we filled the RTPSO in CreateRaytracingPipelineStateObject in DXR-Pipeline.cpp.
	// Given a pipeline state object, we can retrieve the properties that allows to access functions such as GetShaderIdentifier()
	// TODO-2.7: fill in this lambda function that given a stateObjectProperties, will tell you all the shader ids used.
	auto GetShaderIDs = [&](auto* stateObjectProperties)
	{
		// Ray generation shader
		rayGenShaderID = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
		shaderIdToStringMap[rayGenShaderID] = c_raygenShaderName;

		// TODO-2.7: Miss shaders.
		// Similar to the raygen shader, but now we  have 1 for each ray type (radiance, shadow)
		// Don't forget to update shaderIdToStringMap.
		missShaderIDs[0] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[RayType::Radiance]);
		shaderIdToStringMap[missShaderIDs[0]] = c_missShaderNames[RayType::Radiance];
		missShaderIDs[1] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[RayType::Shadow]);
		shaderIdToStringMap[missShaderIDs[1]] = c_missShaderNames[RayType::Shadow];

		// Hitgroup shaders for the Triangle. We have 2: one for radiance ray, and another for the shadow ray.
		for (UINT i = 0; i < RayType::Count; i++)
		{
			hitGroupShaderIDs_TriangleGeometry[i] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_TriangleGeometry[i]);
			shaderIdToStringMap[hitGroupShaderIDs_TriangleGeometry[i]] = c_hitGroupNames_TriangleGeometry[i];
		}

		// TODO-2.7: Hitgroup shaders for the AABBs. We have 2 for each AABB.
		for (UINT i = 0; i < IntersectionShaderType::Count; i++)
		{
			for (UINT j = 0; j < RayType::Count; j++)
			{
				hitGroupShaderIDs_AABBGeometry[i][j] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_AABBGeometry[i][j]);
				shaderIdToStringMap[hitGroupShaderIDs_AABBGeometry[i][j]] = c_hitGroupNames_AABBGeometry[i][j];
			}
		}
	};

	// Get shader identifiers using the lambda function defined above.
	UINT shaderIDSize;
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		GetShaderIDs(m_fallbackStateObject.Get());
		shaderIDSize = m_fallbackDevice->GetShaderIdentifierSize();
	}
	else // DirectX Raytracing
	{
		ComPtr<ID3D12StateObjectPropertiesPrototype> stateObjectProperties;
		ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
		GetShaderIDs(stateObjectProperties.Get());
		shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	}

	/*************--------- Shader table layout -------*******************
	| --------------------------------------------------------------------
	| Shader table - HitGroupShaderTable:
	| [0] : MyHitGroup_Triangle
	| [1] : MyHitGroup_Triangle_ShadowRay
	| [2] : MyHitGroup_AABB_AnalyticPrimitive
	| [3] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
	| ...
	| [6] : MyHitGroup_AABB_VolumetricPrimitive
	| [7] : MyHitGroup_AABB_VolumetricPrimitive_ShadowRay
	| ...
	| --------------------------------------------------------------------
	**********************************************************************/

	// RayGen shader table. We use the ShaderTable class in DXR-Structs, which is essentially 
	// a bundle of ShaderRecords (also defined in DXR-Structs) that are uploadable to the GPU.
	{
		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIDSize; // No root arguments

		// The RayGen shader table contains a single ShaderRecord: the one single raygen shader!
		ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
		
		// Push back the shader record, which does not need any root signatures.
		rayGenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderRecordSize, nullptr, 0));
		
		// Save the uploaded resource (remember that the uploaded resource is created when we call Allocate() on a GpuUploadBuffer
		rayGenShaderTable.DebugPrint(shaderIdToStringMap);
		m_rayGenShaderTable = rayGenShaderTable.GetResource();
	}

	// TODO-2.7: Miss shader table. Very similar to the RayGen table except now we push_back() 2 shader records
	// 1 for the radiance ray, 1 for the shadow ray. Don't forget to call DebugPrint() on the table for your sanity!
	{
		UINT numShaderRecords = 2;
		UINT shaderRecordSize = shaderIDSize; // No root arguments

		ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");

		// Push back the shader record, which does not need any root signatures.
		missShaderTable.push_back(ShaderRecord(missShaderIDs[0], shaderRecordSize, nullptr, 0));
		missShaderTable.push_back(ShaderRecord(missShaderIDs[1], shaderRecordSize, nullptr, 0));

		// Save the uploaded resource (remember that the uploaded resource is created when we call Allocate() on a GpuUploadBuffer
		missShaderTable.DebugPrint(shaderIdToStringMap);
		m_missShaderTable = missShaderTable.GetResource();
		m_missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
	}

	// Hit group shader table. This one is slightly different given that a hit group requires its own custom root signature.
	// You defined the root signatures in DXR-Pipeline.cpp, and you can refresh your memory as to what these local root signatures
	// hold by looking at RaytracingSceneDefines.h.
	{
		// # shader records = # triangle shader records + # AABB shader records = 2 + (# AABB types) * 2
		UINT numShaderRecords = RayType::Count + IntersectionShaderType::TotalPrimitiveCount * RayType::Count;

		// Note how the shader record size now also accounts for the local root signature size
		UINT shaderRecordSize = shaderIDSize + LocalRootSignature::MaxRootArgumentsSize();

		// Initialize one giant hitgroup shader table for ALL hitgroups. We will pushback shader records one by one.
		ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

		// Triangle geometry hit groups.
		{
			// We finally get to assign the root arguments needed for the triangle hitgroup: the plane material properties!
			LocalRootSignature::Triangle::RootArguments rootArgs;
			rootArgs.materialCb = m_planeMaterialCB;

			for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
			{
				hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
			}
		}

		// TODO-2.7: AABB geometry hit groups. Very similar to the triangle case.
		// The root arguments are now LocalRootSignature::AABB::RootArguments (see RaytracingSceneDefines.h)
		// Essentially, the root arguments for the AABB is a material + the primitive instance constant buffer.
		// Hint 1:	the material can be found in m_aabbMaterialCB.
		// Hint 2:	the primitive instance constant buffer is 1 instance index and 1 primitive type.
		//			the instance index is used to index into m_aabbPrimitiveAttributeBuffer. This is unique per object in the scene.
		//				this should follow the *same* order you used to build the instance buffers in UpdateAABBPrimitiveAttributes()
		//				in DXR-Pipeline.cpp. So if you did AABB, then Sphere, then Metaballs, then follow that order.
		//			the primitive type is used to tell the shader what type of procedural geometry this is.
		// Remember that hitGroupShaderIDs_AABBGeometry is a 2-array indexed like so [type of geometry][ray type]
		{
			LocalRootSignature::AABB::RootArguments rootArgs;
			UINT instanceIndex = 0;

			// Create a shader record for each primitive.
			for (UINT iShader = 0, instanceIndex = 0; iShader < IntersectionShaderType::Count; iShader++)
			{
				UINT numPrimitiveTypes = IntersectionShaderType::PerPrimitiveTypeCount(static_cast<IntersectionShaderType::Enum>(iShader));

				// Primitives for each intersection shader.
				for (UINT primitiveIndex = 0; primitiveIndex < numPrimitiveTypes; primitiveIndex++)
				{
					rootArgs.materialCb = m_aabbMaterialCB[instanceIndex];
					rootArgs.aabbCB.instanceIndex = instanceIndex;
					rootArgs.aabbCB.primitiveType = primitiveIndex;
					instanceIndex++;

					// Ray types.
					for (UINT r = 0; r < RayType::Count; r++)
					{
						auto& hitGroupShaderID = hitGroupShaderIDs_AABBGeometry[iShader][r];
						hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
					}
				}
			}
		}

		// Save the uploaded resource as well as the stride used to index into it.
		hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
		m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
		m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
	}
}
