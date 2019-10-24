// LOOKAT-1.9.1: RaytracingShaderHelper.hlsli
// Contains various helper functions used from the shader code in Raytracing.hlsl.

#ifndef RAYTRACINGSHADERHELPER_H
#define RAYTRACINGSHADERHELPER_H

#include "RayTracingHlslCompat.h"

#define INFINITY (1.0/0.0)

// LOOKAT-1.9.1: Ray equation: origin + t * direction
struct Ray
{
    float3 origin;
    float3 direction;
};

float length_toPow2(float2 p)
{
    return dot(p, p);
}

float length_toPow2(float3 p)
{
    return dot(p, p);
}

void swap(inout float a, inout float b)
{
	float temp = a;
	a = b;
	b = temp;
}

bool is_in_range(in float val, in float min, in float max)
{
	return (val >= min && val <= max);
}

// LOOKAT-1.9.1: Tests if a hit is culled based on specified RayFlags.
bool is_culled(in Ray ray, in float3 hitSurfaceNormal)
{
	float rayDirectionNormalDot = dot(ray.direction, hitSurfaceNormal);

	bool isCulled =
		((RayFlags() & RAY_FLAG_CULL_BACK_FACING_TRIANGLES) && (rayDirectionNormalDot > 0))
		||
		((RayFlags() & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES) && (rayDirectionNormalDot < 0));

	return isCulled;
}

// LOOKAT-1.9.1: Test if a hit is valid based on specified RayFlags and <RayTMin, RayTCurrent> range.
bool is_a_valid_hit(in Ray ray, in float thit, in float3 hitSurfaceNormal)
{
	return is_in_range(thit, RayTMin(), RayTCurrent()) && !is_culled(ray, hitSurfaceNormal);
}

// TODO-3.4.2: Return a cycling <0 -> 1 -> 0> animation interpolant
// Given the total elapsed time, and the duration of a cycle, do the following:
// (1) Find out how far in the current cycle the time is. E.g if total time is 5 seconds, a cycle is 2 seconds, then we are 50% through the current cycle.
//	   Call this valye `interpolant`
// (2) We want the interpolant to cycle from 0 to 1 to 0 ALL in one single cycle.
//	   So if we are < 50% through the cycle, we want to make sure that this interpolant hits 1 at 50%.
//		  if we are > 50% through the cycle, we want to make sure that this interpolant hits 0 at 100%.
//	   Basically: given a linear value from 0 --> 1, apply a function like so:
//				  f(0) = 0, f(0.5) = 1, f(1) = 0. And is linear between these 3 points.
// (3) Call the hlsl built-in function smoothstep() on this interpolant to smooth it out so it doesn't change abruptly.
float CalculateAnimationInterpolant(in float elapsedTime, in float cycleDuration)
{
    float interpolant = fmod(elapsedTime, cycleDuration) / cycleDuration;
    interpolant = sin(3.1415 * interpolant);
    
	return smoothstep(0, 1, interpolant);
}

// Load three 2-byte indices from a ByteAddressBuffer.
static
uint3 Load3x16BitIndices(uint offsetBytes, ByteAddressBuffer Indices)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4-byte boundary.
    // Since we need to read three 2-byte indices: { 0, 1, 2 } 
    // aligned at a 4-byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four2ByteIndices = Indices.Load2(dwordAlignedOffset);

    // Aligned: { 0 1 | 2 - } => retrieve first three 2-byte indices
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four2ByteIndices.x & 0xffff;
        indices.y = (four2ByteIndices.x >> 16) & 0xffff;
        indices.z = four2ByteIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 2-byte indices
    {
        indices.x = (four2ByteIndices.x >> 16) & 0xffff;
        indices.y = four2ByteIndices.y & 0xffff;
        indices.z = (four2ByteIndices.y >> 16) & 0xffff;
    }

    return indices;
}

// LOOKAT-1.9.1: Retrieve the intersection point in world coordinates.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// LOOKAT-1.9.1: Retrieve some attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
// e.g. vertexAttribute could be a color, a normal, etc..
float3 HitAttribute(float3 vertexAttribute[3], float2 barycentrics)
{
    return vertexAttribute[0] +
        barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// TODO-3.1: Generate a ray in world space for a camera pixel corresponding to a dispatch index (analogous to a thread index in CUDA).
// Check out https://docs.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-raytracing-hlsl-system-value-intrinsics to see interesting 
// intrinsic HLSL raytracing functions you may use.
// Remember that you are given the pixel coordinates from index. You need to convert this to normalized-device coordinates first.
// Want: bottom left corner = (-1,-1) and top right corner = (1,1). 
// Keep in mind that the pixel space in DirectX is top left = (0,0) and bottom right = (width, height)
// Once you have the normalized-device coordinates, use the projectionToWorld matrix to find a 3D location for this pixel. The depth will be wrong but
// as long as the direction of the ray is correct then the depth does not matter.
inline Ray GenerateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 projectionToWorld)
{
	Ray ray;
    ray.origin = cameraPosition;
    float ndcx = (float(index.x + 0.5f) / DispatchRaysDimensions().x) * 2.0f - 1.0f;
    float ndcy = 1.0f - (float(index.y + 0.5f) / DispatchRaysDimensions().y) * 2.0f;
    float4 castPos = mul(float4(ndcx, ndcy, 1.0, 1.0), projectionToWorld);
	castPos /= castPos.w;
	ray.direction = normalize(castPos.xyz - cameraPosition);

    return ray;
}

// TODO-3.5: Fresnel reflectance - schlick approximation.
// See https://en.wikipedia.org/wiki/Schlick%27s_approximation for formula.
// f0 is usually the albedo of the material assuming the outside environment is air.
float3 FresnelReflectanceSchlick(in float3 I, in float3 N, in float3 f0)
{
	float3 cosTheta = abs(dot(N, I));
    float3 approx = f0 + (1 - f0) * pow((1 - cosTheta), 5);
    
    return approx;
}

#endif // RAYTRACINGSHADERHELPER_H