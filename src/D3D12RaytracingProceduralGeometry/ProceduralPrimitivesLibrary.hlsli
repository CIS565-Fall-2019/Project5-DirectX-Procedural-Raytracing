#ifndef PROCEDURALPRIMITIVESLIBRARY_H
#define PROCEDURALPRIMITIVESLIBRARY_H

// LOOKAT-1.9.2: ProceduralPrimitivesLibrary.hlsli, a wrapper over all types of intersections the project support.
// The base project supports analytical primitives (AABB, Spheres), and volumetric primitives (Metaballs).
// This can be extended to add more procedurals, like signed-distance fields!
// Each intersection test will output the t at which the intersection happened as well as filled in attributes struct (normal vector)

#include "RaytracingShaderHelper.hlsli"

#include "AnalyticPrimitives.hlsli"
#include "VolumetricPrimitives.hlsli"

// LOOKAT-3.4.1: Analytic geometry intersection test.
// AABB local space dimensions: <-1,1>.
bool RayAnalyticGeometryIntersectionTest(in Ray ray, in AnalyticPrimitive::Enum analyticPrimitive, out float thit, out ProceduralPrimitiveAttributes attr)
{
    float3 aabb[2] = {
        float3(-1,-1,-1),
        float3(1,1,1)
    };

    switch (analyticPrimitive)
    {
    case AnalyticPrimitive::AABB: return RayAABBIntersectionTest(ray, aabb, thit, attr);
    case AnalyticPrimitive::Spheres: return RayMultipleSpheresIntersectionTest(ray, thit, attr);
    default: return false;
    }
}

// LOOKAT-3.4.2: Analytic geometry intersection test.
// AABB local space dimensions: <-1,1>.
bool RayVolumetricGeometryIntersectionTest(in Ray ray, in VolumetricPrimitive::Enum volumetricPrimitive, out float thit, out ProceduralPrimitiveAttributes attr, in float elapsedTime)
{
    switch (volumetricPrimitive)
    {
    case VolumetricPrimitive::Metaballs: return RayMetaballsIntersectionTest(ray, thit, attr, elapsedTime);
    default: return false;
    }
}

#endif // PROCEDURALPRIMITIVESLIBRARY_H