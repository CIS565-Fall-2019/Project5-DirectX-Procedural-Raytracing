#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

// LOOKAT-2.1: Initialize scene rendering parameters.
// This will fill in most constant and structured buffers (material properties, plane color, camera, lights)
// The member variables altered are m_aabbMaterialCB, m_planeMaterialCB, camera (m_eye, m_right, ...), and m_sceneCB.
void DXProceduralProject::InitializeScene()
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	// Setup materials.
	{
		// Function pointer that sets up material properties for a procedural primitive
		auto SetAttributes = [&](
			UINT primitiveIndex,
			const XMFLOAT4& albedo,
			float reflectanceCoef = 0.0f,
			float diffuseCoef = 0.9f,
			float specularCoef = 0.7f,
			float specularPower = 50.0f)
		{
			auto& attributes = m_aabbMaterialCB[primitiveIndex];
			attributes.albedo = albedo;
			attributes.reflectanceCoef = reflectanceCoef;
			attributes.diffuseCoef = diffuseCoef;
			attributes.specularCoef = specularCoef;
			attributes.specularPower = specularPower;
		};

		// Changes plane color
		m_planeMaterialCB = { XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), 0.7f, 1, 0.4f, 50};

		// Albedos
		XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
		XMFLOAT4 purple = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
		XMFLOAT4 chromium_purple = XMFLOAT4(purple.x * ChromiumReflectance.x, 
											purple.y * ChromiumReflectance.y,
											purple.z * ChromiumReflectance.z,
											1.0f);

		UINT offset = 0;
		// Analytic primitives.
		{
			using namespace AnalyticPrimitive;
			SetAttributes(offset + AABB, yellow, 0.3f);
			SetAttributes(offset + Spheres, chromium_purple, 0.8f);
			offset += AnalyticPrimitive::Count;
		}

		// Volumetric primitives.
		{
			using namespace VolumetricPrimitive;
			SetAttributes(offset + Metaballs, ChromiumReflectance, 1);
			offset += VolumetricPrimitive::Count;
		}
	}

	// Setup camera.
	{
		// Initialize the view and projection inverse matrices.
		m_eye = { 0.0f, 5.3f, -17.0f, 1.0f };
		m_at = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

		XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
		m_up = XMVector3Normalize(XMVector3Cross(direction, right));

		// Rotate camera around Y axis.
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
		m_eye = XMVector3Transform(m_eye, rotate);
		m_up = XMVector3Transform(m_up, rotate);

		UpdateCameraMatrices();
	}

	// Setup lights.
	{
		// Initialize the lighting parameters.
		XMFLOAT4 lightPosition;
		XMFLOAT4 lightAmbientColor;
		XMFLOAT4 lightDiffuseColor;

		lightPosition = XMFLOAT4(0.0f, 18.0f, -20.0f, 0.0f);
		m_sceneCB->lightPosition = XMLoadFloat4(&lightPosition);

		lightAmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_sceneCB->lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

		float d = 0.8f;
		lightDiffuseColor = XMFLOAT4(d, d, d, 1.0f);
		m_sceneCB->lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
	}
}

// LOOKAT-2.1: Creates the scene constant buffer.
// See ConstantBuffer in DXR-Structs.h to understand what this means.
void DXProceduralProject::CreateConstantBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	m_sceneCB.Create(device, frameCount, L"Scene Constant Buffer");
}

// TODO-2.1: Create the AABB Primitive Attribute Buffer.
// This means you will need to allocate the AABB attributes on the GPU and map them onto the CPU.
//	*	This should be very similar to CreateConstantBuffers(), which allocates 
//		1 constant buffer on the GPU and maps it on the CPU for writing
//	*	As a refresher:	constant buffers are generally for data that doesn't change on the CPU side (e.g a view matrix)
//		structured buffers are for structs that have dynamic data (e.g lights in a scene, or AABBs in this case)
void DXProceduralProject::CreateAABBPrimitiveAttributesBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	// second param is num_Elements, the number of aabbs stored in the buffer
	m_aabbPrimitiveAttributeBuffer.Create(device, 3, frameCount, L"AABB Primitive Attribute Buffer");
}

// LOOKAT-2.1: Update camera matrices stored in m_sceneCB.
void DXProceduralProject::UpdateCameraMatrices()
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	m_sceneCB->cameraPosition = m_eye;
	float fovAngleY = 45.0f;
	XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
	XMMATRIX viewProj = view * proj;
	m_sceneCB->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// TODO-2.1: Update the PrimitiveInstancePerFrameBuffer for every AABB stored in m_aabbPrimitiveAttributeBuffer[].
//	*	The localSpaceToBottomLevelAS will transform the AABB from local space (a.k.a model space) to bottom-level AS space
//		which is an intermediate transform state. Think of it as taking a bland T-posing model and making a funny pose with it,
//		say now it is squatting instead.
//	*	Again, this should be very similar to what we do in UpdateCameraMatrices() which updates a ConstantBuffer
//		In this case, we are updating a *slot* in a StructuredBuffer, which is basically the same thing except it holds structs
//		instead of constants.
//	*	You will have to do this for every frame we render, which is easy if you read into how the operator[] works in 
//		the StructuredBuffer class in DXR-Structs.h.
void DXProceduralProject::UpdateAABBPrimitiveAttributes(float animationTime)
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	XMMATRIX mIdentity = XMMatrixIdentity();

	// Different scale matrices
	XMMATRIX mScale15y = XMMatrixScaling(1, 1.5, 1);
	XMMATRIX mScale15 = XMMatrixScaling(1.5, 1.5, 1.5);
	XMMATRIX mScale2 = XMMatrixScaling(2, 2, 2);

	// Rotation matrix that changes over time
	XMMATRIX mRotation = XMMatrixRotationY(-2 * animationTime);

	
	auto SetTransformForAABB = [&](UINT primitiveIndex, XMMATRIX& mScale, XMMATRIX& mRotation)
	{
		XMVECTOR vTranslation =
			0.5f * (XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&m_aabbs[primitiveIndex].MinX))
				+ XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&m_aabbs[primitiveIndex].MaxX))); // i.e middle of AABB.
		XMMATRIX mTranslation = XMMatrixTranslationFromVector(vTranslation);

		// TODO-2.1: Fill in this lambda function.
		// It should create a transform matrix that is equal to scale * rotation * translation.
		// This matrix would transform the AABB from local space to bottom level AS space.
		// You can infer what the bottom level AS space to local space transform should be.
		// The intersection shader tests in this project work with local space, but the geometries are provided in bottom level 
		// AS space. So this data will be used to convert back and forth from these spaces.
		XMMATRIX locToBLAS = mScale * mRotation * mTranslation;
		m_aabbPrimitiveAttributeBuffer[primitiveIndex].localSpaceToBottomLevelAS = locToBLAS;
		m_aabbPrimitiveAttributeBuffer[primitiveIndex].bottomLevelASToLocalSpace = XMMatrixInverse(nullptr, locToBLAS);
	};

	UINT offset = 0;
	// Analytic primitives.
	{
		using namespace AnalyticPrimitive;
		SetTransformForAABB(offset + AABB, mScale15y, mIdentity);
		SetTransformForAABB(offset + Spheres, mScale15, mRotation);
		offset += AnalyticPrimitive::Count;
	}

	// Volumetric primitives.
	{
		using namespace VolumetricPrimitive;
		SetTransformForAABB(offset + Metaballs, mScale2, mRotation);
		offset += VolumetricPrimitive::Count;
	}
}
