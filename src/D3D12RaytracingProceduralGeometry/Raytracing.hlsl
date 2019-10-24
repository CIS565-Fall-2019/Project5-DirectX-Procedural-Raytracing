#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

// LOOKAT-1.9.5: Raytracing.hlsl
// This is where most of the important GPU code lives.
// The root signatures you will define will be used here.
// The hitgroups you will define will also be used here.

#define HLSL
#include "RaytracingHlslCompat.h"
#include "ProceduralPrimitivesLibrary.hlsli"
#include "RaytracingShaderHelper.hlsli"

//***************************************************************************
//*****------ Shader resources bound via root signatures -------*************
//***************************************************************************

// Scene wide resources.
//  g_* - bound via a global root signature.
//  l_* - bound via a local root signature.
RaytracingAccelerationStructure g_scene : register(t0, space0); // accel structure
RWTexture2D<float4> g_renderTarget : register(u0); // output texture
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0); // camera projections, lights, time elapsed

// Triangle resources
ByteAddressBuffer g_indices : register(t1, space0); // triangle indices
StructuredBuffer<Vertex> g_vertices : register(t2, space0); // triangle positions

// Procedural geometry resources
StructuredBuffer<PrimitiveInstancePerFrameBuffer> g_AABBPrimitiveAttributes : register(t3, space0); // transforms per procedural
ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1); // material data per procedural
ConstantBuffer<PrimitiveInstanceConstantBuffer> l_aabbCB: register(b2); // other meta-data: type, instance indices


//***************************************************************************
//*********************------ Utilities. -------*****************************
//***************************************************************************

// TODO-3.5: Diffuse lighting calculation. This is just a Lambert shading term.
// HINT:	See https://en.wikipedia.org/wiki/Lambertian_reflectance
// Remember to clamp the dot product term!
float CalculateDiffuseCoefficient(in float3 incidentLightRay, in float3 normal)
{
	return abs(dot(normalize(incidentLightRay), normalize(normal)));
}

// TODO-3.5: Phong lighting specular component.
// The equation should be coefficient = (reflectedRay . reverseRayDirection) ^ (specularPower).
// HINT:	Consider using built-in DirectX functions to find the reflected ray. Remember that a reflected ray is reflected
//			with respect to the normal of the hit position.
// Remember to normalize the reflected ray, and to clamp the dot product term 
float4 CalculateSpecularCoefficient(in float3 incidentLightRay, in float3 normal, in float specularPower)
{
    float3 reflectedRay = normalize(reflect(normalize(incidentLightRay), normalize(normal)));
    return pow(abs(dot(normalize(WorldRayDirection()), reflectedRay)), specularPower);
}

// TODO-3.5: Phong lighting model = ambient + diffuse + specular components.
// See https://en.wikipedia.org/wiki/Phong_reflection_model for the full simple equation.
// We have filled in the ambient color for you.
// HINT 1: remember that you can get the world position of the hitpoint using HitWorldPosition() 
//		   from RaytracingShaderHelper.hlsli. The light position is somewhere in g_sceneCB.
// HINT 1: first you need to figure out the diffuse coefficient using CalculateDiffuseCoefficient()
//		   then you need to figure out the specular coefficient using CalculateSpecularCoefficient()
//		   then you need to combine the diffuse, specular, and ambient colors into one color.
// Remember that if the ray is a shadow ray, then the hit position should be very dim. Consider using
// InShadowRadiance from RaytracingHlslCompat.h tp dim down the diffuse color. The specular color should 
// be completely black if the ray is a shadow ray.
float4 CalculatePhongLighting(in float4 albedo, in float3 normal, in bool isInShadow,
	in float diffuseCoef = 1.0, in float specularCoef = 1.0, in float specularPower = 50)
{
	// Ambient component
	// Fake AO: Darken faces with normal facing downwards/away from the sky a little bit
	float4 ambientColor = g_sceneCB.lightAmbientColor;
	float4 ambientColorMin = g_sceneCB.lightAmbientColor - 0.1;
	float4 ambientColorMax = g_sceneCB.lightAmbientColor;
	float a = 1 - saturate(dot(normal, float3(0, -1, 0)));
	ambientColor = albedo * lerp(ambientColorMin, ambientColorMax, a);

	float3 lightRay = normalize(g_sceneCB.lightPosition.xyz - HitWorldPosition());
    // Diffuse component
	float4 diffColor = g_sceneCB.lightDiffuseColor;
    float4 diffuseColor = albedo * diffColor * diffuseCoef * CalculateDiffuseCoefficient(lightRay, normal);

    // Specular component
    float4 specularColor = float4(1.0f, 1.0f, 1.0f, 1.0f) * specularCoef * CalculateSpecularCoefficient(lightRay, normal, specularPower);

    // Changes if in shadow
    if (isInShadow)
    {
        diffuseColor *= InShadowRadiance;
        specularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

	return ambientColor + diffuseColor + specularColor;
}

//***************************************************************************
//*****------ TraceRay wrappers for radiance and shadow rays. -------********
//***************************************************************************

// LOOKAT-1.9.5: Trace a radiance ray into the scene and returns a shaded color.
// You should read about the TraceRay() function to understand what this does.
float4 TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1 };

	// TraceRay() is a built-in DXR function. Lookup its documentation to see what it does.
	// To understand how a ray "finds out" what type of object it hit and therefore call the correct shader hitgroup, it indexes into the shader
	// table as follows:
	// hitgroup to choose = ptr to shader table + size of a shader record + (ray contribution + (geometry index * geometry stride) + instance contribution)
	// * `ray contribution` and `geometry stride` are given here.
	// * `ptr to shader table` + `size of a shader record` are given in BuildShaderTables() in DXR-ShaderTable.cpp
	// * `geometry index` is implicitly given when you build your bottom-level AS in BuildGeometryDescsForBottomLevelAS() in DXR-AccelerationStructure.cpp
	// * `instance contribution` is given in BuildBottomLevelASInstanceDescs() in DXR-AccelerationStructure.cpp
	// Essentially, when a ray hits a Geometry in a Bottom-Level AS Instance contained within a Top Level AS, it does the following:
	// (1) Identify which Instance of a BLAS it hit (Triangle or AABB) --> this gives it the `instance contribution`
	// (2) Identify which Geometry *inside* this Instance it hit
	//			(If hit Triangle instance, then Triangle geom!, if hit AABB instance, then might be Sphere, Metaball, Fractal, etc..)
	//			--> this gives it the `geometry index`
	// (3) Identify what type of Ray it is --> this is given right here, say a Radiance ray
	// (4) Combines all of these inputs to index into the correct shader into the hitgroup shader table.
    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Radiance],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Radiance],
        rayDesc, rayPayload);

    return rayPayload.color;
}

// TODO-3.2: Trace a shadow ray and return true if it hits any geometry. Very similar to TraceRay() above
// Hint 1: for the TraceRay() flags, make sure you cull back facing triangles, skip any hit shaders, skip closest hit shaders, 
//		   and just accept any geometry you hit
// Hint 2: remember what the ShadowRay payload looks like. See RaytracingHlslCompat.h
bool TraceShadowRayAndReportIfHit(in Ray ray, in UINT currentRayRecursionDepth)
{
	if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    ShadowRayPayload rayPayload = { true };

    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER | RAY_FLAG_FORCE_OPAQUE,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Shadow],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Shadow],
        rayDesc, rayPayload);

    return rayPayload.hit;
}

//***************************************************************************
//********************------ Ray gen shader -------**************************
//***************************************************************************

// TODO-3.1: Complete the ray generation shader.
// (1) Generate a ray using the function GenerateCameraRay() in RaytracingShaderHelper.hlsli
// (2) Trace a radiance ray using the generated ray to obtain a color
// (3) Write that color to the render target
[shader("raygeneration")]
void MyRaygenShader()
{
    Ray camRay = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
    float4 col = TraceRadianceRay(camRay, 0);

	// Write the color to the render target
    g_renderTarget[DispatchRaysIndex().xy] = col;
}

//***************************************************************************
//******************------ Closest hit shaders -------***********************
//***************************************************************************

// LOOKAT-1.9.5: ClosestHitShader for a triangle.
// This is invoked after:
// (1) a call to TraceRay() and
// (2) the ray hits the closest possible Triangle geometry to it
[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;

	// PrimitiveIndex() is the triangle index within the mesh. For Procedural primitives, this is the index into the AABB array defining the geometry.
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up three 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex, g_indices);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 triangleNormal = g_vertices[indices[0]].normal;

	// This is the intersection point on the triangle.
	float3 hitPosition = HitWorldPosition();

    // Trace a ray from the hit position towards the single light source we have. If on our way to the light we hit something, then we have a shadow!
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    // Reflected component ray.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001 )
    {
        // Trace a reflection ray from the intersection points using Snell's law. The reflect() HLSL built-in function does this for you!
		// See https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions
        Ray reflectionRay = { hitPosition, reflect(WorldRayDirection(), triangleNormal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color.
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo, triangleNormal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
    float4 color = (phongColor + reflectedColor);

	// TODO: Apply a visibility falloff.
	// If the ray is very very very far away, tends to sample the background color rather than the color you computed.
	// This is to mimic some form of distance fog where farther objects appear to blend with the background.
	// Hint 1: look at the intrinsic function RayTCurrent() that returns how "far away" your ray is.
	// Hint 2: use the built-in function lerp() to linearly interpolate between the computed color and the Background color.
	//		   When t is big, we want the background color to be more pronounced.
    float t = 1.0f - min(RayTCurrent()/500.0f, 1.0f);
    float4 falloffColor = lerp(BackgroundColor, color, t);

	rayPayload.color = falloffColor;
}

// TODO: Write the closest hit shader for a procedural geometry.
// You suppose this is called after the ray successfully iterated over all geometries and determined it intersected with some AABB.
// The attributes of the AABB are in attr (basically the normal)
// You need to:
// (0) Realize that you do not need to load in indices or vertices because we're talking procedural geometry here (wohoo!)
// (1) Trace a shadow ray to determine if this ray is a shadow ray.
// (2) Trace a reflectance ray --> compute the reflected color.
// (3) Use the fact that your ray is a shadow ray or not to compute the Phong lighting.
// (4) Combine the reflect color and the phong color into one color.
// (5) Apply visibility falloff to select some interpolation between the computed color or the background color
// (6) Fill the payload color with whatever final color you computed
[shader("closesthit")]
void MyClosestHitShader_AABB(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
    // This is the intersection point on the triangle.
    float3 hitPosition = HitWorldPosition();
    
    // Trace a shadow ray to determine if this ray is a shadow ray
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    // Reflected component ray.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001)
    {
        Ray reflectionRay = { hitPosition, reflect(WorldRayDirection(), attr.normal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color with phong lighting
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo, attr.normal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
    float4 color = (phongColor + reflectedColor);

    float t = 1.0f - min(RayTCurrent() / 500.0f, 1.0f);
    float4 falloffColor = lerp(BackgroundColor, color, t);

    rayPayload.color = falloffColor;
}

//***************************************************************************
//**********************------ Miss shaders -------**************************
//***************************************************************************

// TODO-3.3: Complete the Radiance ray miss shader. What color should you output if you hit no geometry?
// Make sure you edit the rayPayload so your color gets passed down to other shaders.
// NOTE: whether we missed a Triangle or a Procedural Geometry does not matter. The miss output should be the same!
[shader("miss")]
void MyMissShader(inout RayPayload rayPayload)
{
    rayPayload.color = BackgroundColor;
}

// TODO-3.3: Complete the Shadow ray miss shader. Is this ray a shadow ray if it hit nothing?
[shader("miss")]
void MyMissShader_ShadowRay(inout ShadowRayPayload rayPayload)
{
    rayPayload.hit = false;
}

//***************************************************************************
//*****************------ Intersection shaders-------************************
//***************************************************************************

// Get ray in AABB's local space.
Ray GetRayInAABBPrimitiveLocalSpace()
{
    PrimitiveInstancePerFrameBuffer attr = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

    // Retrieve a ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    Ray ray;
    ray.origin = mul(float4(ObjectRayOrigin(), 1), attr.bottomLevelASToLocalSpace).xyz;
    ray.direction = mul(ObjectRayDirection(), (float3x3) attr.bottomLevelASToLocalSpace);
    return ray;
}

// LOOKAT-3.4.1: Analytic Primitive intersection shader.
// Analytic primitives are spheres and boxes in this project.
[shader("intersection")]
void MyIntersectionShader_AnalyticPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    AnalyticPrimitive::Enum primitiveType = (AnalyticPrimitive::Enum) l_aabbCB.primitiveType;

	// The point of the intersection shader is to:
	// (1) find out what is the t at which the ray hits the procedural
	// (2) pass on some attributes used by the closest hit shader to do some shading (e.g: normal vector)
    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

		// Make sure the normals are stored in BLAS space and not the local space
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal));

		// thit is invariant to the space transformation
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

// TODO-3.4.2: Volumetric primitive intersection shader. In our case, we only have Metaballs to take care of.
// The overall structure of this function is parallel to MyIntersectionShader_AnalyticPrimitive() 
// except you have to call the appropriate intersection test function (see RayVolumetricGeometryIntersectionTest())
[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    VolumetricPrimitive::Enum primitiveType = (VolumetricPrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayVolumetricGeometryIntersectionTest(localRay, primitiveType, thit, attr, g_sceneCB.elapsedTime))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

        // Make sure the normals are stored in BLAS space and not the local space
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal));

        // thit is invariant to the space transformation
        ReportHit(thit, 0, attr);
    }
}
#endif // RAYTRACING_HLSL