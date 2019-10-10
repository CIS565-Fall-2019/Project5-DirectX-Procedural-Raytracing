#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-2.3, LOOKAT-2.7: This will define all shader entry points.
// You will notice that every name here corresponds to an actual shader defined in 
// a *.hlsl file.
// Example: MyRaygenShader --> void MyRaygenShader() in Raytracing.hlsl
//
// This is a bit different for hitgroups. They just represent bundles of 
// closest hit, miss, and intersection shaders. These are on the CPU side only.
//

// LOOKAT-2.3, LOOKAT-2.7: Shader entry points.
const wchar_t* DXProceduralProject::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* DXProceduralProject::c_intersectionShaderNames[] =
{
	L"MyIntersectionShader_AnalyticPrimitive",
	L"MyIntersectionShader_VolumetricPrimitive",
};
const wchar_t* DXProceduralProject::c_closestHitShaderNames[] =
{
	L"MyClosestHitShader_Triangle",
	L"MyClosestHitShader_AABB",
};
const wchar_t* DXProceduralProject::c_missShaderNames[] =
{
	L"MyMissShader",
	L"MyMissShader_ShadowRay"
};

// LOOKAT-2.3, LOOKAT-2.7: Hit groups. For triangles, we have 1 for radiance rays, and another for shadow rays.
const wchar_t* DXProceduralProject::c_hitGroupNames_TriangleGeometry[] =
{
	L"MyHitGroup_Triangle",
	L"MyHitGroup_Triangle_ShadowRay"
};
const wchar_t* DXProceduralProject::c_hitGroupNames_AABBGeometry[][RayType::Count] =
{
	{ L"MyHitGroup_AABB_AnalyticPrimitive", L"MyHitGroup_AABB_AnalyticPrimitive_ShadowRay" },
	{ L"MyHitGroup_AABB_VolumetricPrimitive", L"MyHitGroup_AABB_VolumetricPrimitive_ShadowRay" },
};