#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

// LOOKAT-1.9.0: RaytracingHLSLCompat.h, a header with shared definitions for C++ and HLSL source files. 
// This will contain all the defines and structs that BOTH the CPU code and the GPU code must know about.
// Example: Vertex struct. The CPU code needs to know about this so it can allocate memory properly, 
//			while the GPU shader code must know about this so it can read Vertex structs.
//	You should probably read this entire file at least once to know what types of shared
//			structs are used.

#ifdef HLSL
	typedef float2 XMFLOAT2;
	typedef float3 XMFLOAT3;
	typedef float4 XMFLOAT4;
	typedef float4 XMVECTOR;
	typedef float4x4 XMMATRIX;
	typedef uint UINT;
#else

using namespace DirectX;

// Shader will use byte encoding to access vertex indices.
typedef UINT16 Index;
#endif

// Number of metaballs to use within an AABB. Limit to 3 unless you are attempting the dynamic looping extra-credit.
#define N_METABALLS 3

#define N_FRACTAL_ITERATIONS 5      // = <1,...>

#define MAX_RAY_RECURSION_DEPTH 3    // ~ primary rays + reflections + shadow rays from reflected geometry.

/**************** Scene *****************/
static const XMFLOAT4 ChromiumReflectance = XMFLOAT4(0.549f, 0.556f, 0.554f, 1.0f);
static const XMFLOAT4 BackgroundColor = XMFLOAT4(0.8f, 0.9f, 1.0f, 1.0f);
static const float InShadowRadiance = 0.35f;

// Ray types traced in this project.
namespace RayType {
	enum Enum {
		Radiance = 0,   // ~ Primary, reflected camera/view rays calculating color for each hit.
		Shadow,         // ~ Shadow/visibility rays, only testing for occlusion
		Count
	};
}

struct RayPayload
{
	XMFLOAT4 color;
	UINT   recursionDepth;
};

struct ShadowRayPayload
{
	bool hit;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

// Really this isn't a `constant` buffer, as in the contents may change but the buffer itself isn't dynamic.
struct SceneConstantBuffer
{
	XMMATRIX projectionToWorld;
	XMVECTOR cameraPosition;
	XMVECTOR lightPosition;
	XMVECTOR lightAmbientColor;
	XMVECTOR lightDiffuseColor;
	float    reflectance;
	float    elapsedTime;                 // Elapsed application time.
};

// Use this namespace to fill your TraceRay() functions.
namespace TraceRayParameters
{
	static const UINT InstanceMask = ~0;   // Everything is visible.
	namespace HitGroup {
		static const UINT Offset[RayType::Count] =
		{
			0, // Radiance ray
			1  // Shadow ray
		};
		static const UINT GeometryStride = RayType::Count;
	}
	namespace MissShader {
		static const UINT Offset[RayType::Count] =
		{
			0, // Radiance ray
			1  // Shadow ray
		};
	}
}

/**************** Primitives *****************/
// AABB or Sphere
namespace AnalyticPrimitive {
	enum Enum {
		AABB = 0,
		Spheres,
		Count
	};
}

// Metaballs
namespace VolumetricPrimitive {
	enum Enum {
		Metaballs = 0,
		Count
	};
}

// Attributes to be interpolated per primitive. This is just the normal.
struct ProceduralPrimitiveAttributes
{
    XMFLOAT3 normal;
};

// Constant buffer per primitive. A primitive is considered static.
struct PrimitiveConstantBuffer
{
    XMFLOAT4 albedo;
    float reflectanceCoef;
    float diffuseCoef;
    float specularCoef;
    float specularPower;
};

// Attributes per primitive instance. An instance primitive actually exists in the scene and may be dynamic.
struct PrimitiveInstanceConstantBuffer
{
    UINT instanceIndex;  
    UINT primitiveType; // Procedural primitive type
};

// Dynamic attributes per primitive instance.
struct PrimitiveInstancePerFrameBuffer
{
    XMMATRIX localSpaceToBottomLevelAS;   // Matrix from local primitive space to bottom-level object space.
    XMMATRIX bottomLevelASToLocalSpace;   // Matrix from bottom-level object space to local primitive space.
};

#endif // RAYTRACINGHLSLCOMPAT_H