#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

// LOOKAT-2.4: Fill in the Raytracing Pipeline State Object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call, with all configuration options resolved, 
// such as local signatures and other state.
void DXProceduralProject::CreateRaytracingPipelineStateObject()
{
	// Create 18 subobjects that combine into 1 RTPSO:
	// 1 - DXIL library
	// 8 - Hit group types - 4 geometries (1 triangle, 3 aabb) x 2 ray types (ray, shadowRay)
	// 1 - Shader config
	// 6 - 3 x Local root signature and their association
	// 1 - Global root signature
	// 1 - Pipeline config
	CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	// DXIL library
	CreateDxilLibrarySubobject(&raytracingPipeline);

	// LOOKAT-2.4: Hit groups. Call the function you filled in in DXR-HitGroup.cpp. 
	CreateHitGroupSubobjects(&raytracingPipeline);

	// Shader config: defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = max(sizeof(RayPayload), sizeof(ShadowRayPayload));
	UINT attributeSize = sizeof(struct ProceduralPrimitiveAttributes);
	shaderConfig->Config(payloadSize, attributeSize);

	// LOOKAT-2.4: Local root signature and shader association. Call the other function you did in in DXR-HitGroup.cpp.
	CreateLocalRootSignatureSubobjects(&raytracingPipeline);

	// Global root signature
	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3D12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

	// Pipeline config: defines the maximum TraceRay() recursion depth.
	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
	pipelineConfig->Config(maxRecursionDepth);

	// This is for debugging purposes. Will nicely print out the pipeline state for you.
	PrintStateObjectDesc(raytracingPipeline);

	// Finally, create the RTPSO. We store it in m_fallbackStateObject for the fallback layer, or in m_dxrStateObject if using actual DXR.
	if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
	{
		ThrowIfFailed(m_fallbackDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_fallbackStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
	}
	else // DirectX Raytracing
	{
		ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
	}
}