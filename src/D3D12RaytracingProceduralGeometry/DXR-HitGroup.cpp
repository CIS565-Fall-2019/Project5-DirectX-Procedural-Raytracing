#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

// LOOKAT-2.3, TODO-2.3: Create the hitgroup pipeline subobject.
// A hitgroup specifies closest hit (mandatory), any hit (optional) and intersection shaders (mandatory but not for triangles).
// They are executed when a ray intersects the geometry.
// See how we do it for 1 triangle, and apply that logic to an AABB.
void DXProceduralProject::CreateHitGroupSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
	// LOOKAT-2.3: Triangle geometry hit groups.
	// The triangle hitgroup only contains a closest hit shader. The intersection shader is automatic, it is implented
	// in the DirectX Raytracing driver.
	{
		for (UINT rayType = 0; rayType < RayType::Count; rayType++)
		{
			auto hitGroup = raytracingPipeline->CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
			if (rayType == RayType::Radiance)
			{
				// We import the closest hit shader name
				hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[GeometryType::Triangle]);
			}

			// We tell the hitgroup that it should export into the correct shader hit group name, with the correct type
			hitGroup->SetHitGroupExport(c_hitGroupNames_TriangleGeometry[rayType]);
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
		}
	}

	// TODO-2.3: AABB geometry hit groups. Very similar to triangles, except now you have to *also* loop over the primitive types.
	{
		//how do we loop over primitive types? -- how do we determine which primitive of this belonging to? or should we just go through them all?
        //analytic
        for (UINT IntersectionShaderTypeCount = 0; IntersectionShaderTypeCount < IntersectionShaderType::Count; ++IntersectionShaderTypeCount)
        {
            for (UINT rayType = 0; rayType < RayType::Count; rayType++)
            {
                auto hitGroup = raytracingPipeline->CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();

                if (rayType == RayType::Radiance)
                {
                    // We import the closest hit shader name
                    hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[GeometryType::AABB]);
                }

                //setting up intersection shader for AABB
                hitGroup->SetIntersectionShaderImport(c_intersectionShaderNames[IntersectionShaderTypeCount]);
                // We tell the hitgroup that it should export into the correct shader hit group name, with the correct type
                // for analytics, we have 2 intersection shaders, will that affect?
                hitGroup->SetHitGroupExport(c_hitGroupNames_AABBGeometry[IntersectionShaderTypeCount][rayType]); //we have the exact index of hitgroup element from IntersectionShaderTypeCount
                hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
            }
        }
	}
}

// TODO-2.3: Local root signature and shader association (linking)
void DXProceduralProject::CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
	// Ray gen and miss shaders in this project are not using a local root signature and thus one is not associated with them.
	
	// Triangle geometry
	{
		auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();

		// This is the triangle local root signature you already filled in before.
		localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle].Get());

		// Shader association
		auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExports(c_hitGroupNames_TriangleGeometry);
	}

	// TODO-2.3: AABB geometry hitgroup/local root signature association.
	// Very similar to triangles, except now one for each primitive type.
	{
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();

        // This is the AABB root signature
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::AABB].Get());

        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        //here we should export two hitgroups
        //c_hitGroupNames_AABBGeometry[IntersectionShaderTypeCount]
        for (UINT primitiveType = 0; primitiveType < IntersectionShaderType::Count; primitiveType++)
        {
            rootSignatureAssociation->AddExports(c_hitGroupNames_AABBGeometry[primitiveType]);
        }
        //rootSignatureAssociation->AddExports(c_hitGroupNames_AABBGeometry[IntersectionShaderType::AnalyticPrimitive]);
        //rootSignatureAssociation->AddExports(c_hitGroupNames_AABBGeometry[IntersectionShaderType::VolumetricPrimitive]);
	}
}