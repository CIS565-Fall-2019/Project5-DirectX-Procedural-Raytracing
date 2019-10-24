#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-2.5: This will hopefully make you understand how vertex and index data is being setup for the GPU.
// The triangle data will end up mapping to g_indices and g_vertices in Raytracing.hlsl.
// The steps are to:
// (1) Fill in an array of indices (ints), and vertex positions  (float4).
// (2) Allocate a d3d12 (DirectX 12) upload buffer for each one of these, and then upload the data from the CPU to the GPU. 
//     This can be easily done by using the function AllocateUploadBuffer().
// (3) Create a ShaderResourceView (SRV) for the index buffer, and another for the vertex position buffer.
//	   This basically means "let the shader know where the data will be on the GPU by filling in a descriptor
//     that we will pass to it later on". So this will create a descriptor, and also give us a GPU pointer to the data itself.
//     This is done by using the CreateBufferSRV(), which uses the AllocateDescriptor() function.
void DXProceduralProject::BuildPlaneGeometry()
{
	auto device = m_deviceResources->GetD3DDevice();

	// Plane indices:
	// 0--------1
	// '		'
	// '		'
	// 3--------2
	//
	// Each triple represents a triangle in counter-clockwise order.
	// If you look at the positions of these vertices below, you will notice 
	// that triangle cross products point up, which needed if we want the plane to face upwards
	// i.e have a normal vector pointing up.
	Index indices[] =
	{
		3,1,0,
		2,1,3,

	};

	// Cube vertices positions and corresponding triangle normals.
	Vertex vertices[] =
	{
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // vertex 0: position 0, normal 0
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // vertex 1: position 1, normal 1
		{ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // vertex 2..
		{ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // vertex 3
	};

	AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer.resource);
	AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer.resource);

	// Vertex buffer is passed to the shader along with index buffer as a descriptor range.
	UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, sizeof(indices) / 4, 0);
	UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
	ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
}

// TODO-2.5: Build AABBs for procedural geometry that will be used within the acceleration structure.
void DXProceduralProject::BuildProceduralGeometryAABBs()
{
	auto device = m_deviceResources->GetD3DDevice();

	// Set up AABBs on a grid.
	{
		// 9x3 slots = 9 slots. Note that one procedural geometry can take up multiple slots.
		// You could have a small sphere that takes up 1 slot, and another that is giant and takes up 4 slots.
		XMINT3 aabbGrid = XMINT3(3, 1, 3);

		// The base position of the grid.
		// The formula used is: -((size of all slots in component) + (size of separation between slots in component)) / 2
		// The negation makes the grid starts in the -x,-z plane
		// The division by 2 centers the grid. The base position is still at the -x,-z corner.
		const XMFLOAT3 basePosition =
		{
			-(aabbGrid.x * c_aabbWidth + (aabbGrid.x - 1) * c_aabbDistance) / 2.0f,
			-(aabbGrid.y * c_aabbWidth + (aabbGrid.y - 1) * c_aabbDistance) / 2.0f,
			-(aabbGrid.z * c_aabbWidth + (aabbGrid.z - 1) * c_aabbDistance) / 2.0f,
		};

		// The stride is "how much to move" for the next slot. This is basically the size of a slot + it's separation from its direct
		// neighbor
		XMFLOAT3 stride = XMFLOAT3(c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance);
		
		// TODO-2.5: Lookup the DXR API for the D3D12_RAYTRACING_AABB struct and fill up this lamda function that creates
		// and returns an D3D12_RAYTRACING_AABB for you.
		// Note that you are only filling an axis-aligned bounding box.
		// This should take into account the basePosition and the stride defined above.
		auto InitializeAABB = [&](auto& offsetIndex, auto& size)
		{
			D3D12_RAYTRACING_AABB aabb{};
			aabb.MinX = basePosition.x + offsetIndex.x * stride.x;
			aabb.MinY = basePosition.y + offsetIndex.y * stride.y;
			aabb.MinZ = basePosition.z + offsetIndex.z * stride.z;
			aabb.MaxX = aabb.MinX + size.x;
			aabb.MaxY = aabb.MinY + size.y;
			aabb.MaxZ = aabb.MinZ + size.z;
			return aabb;
		};
		m_aabbs.resize(IntersectionShaderType::TotalPrimitiveCount);
		UINT offset = 0;

		// Analytic primitives.
		{
			using namespace AnalyticPrimitive;
			m_aabbs[offset + AABB] = InitializeAABB(XMFLOAT3(0.5f, 0.0f, 0.0f), XMFLOAT3(2.0f, 3.0f, 2.0f));
			m_aabbs[offset + Spheres] = InitializeAABB(XMFLOAT3(1.0f, 0.75f, -0.5f), XMFLOAT3(3, 3, 3));
			offset += AnalyticPrimitive::Count;
		}

		// Volumetric primitives.
		{
			using namespace VolumetricPrimitive;
			m_aabbs[offset + Metaballs] = InitializeAABB(XMINT3(-1, 0, 0), XMFLOAT3(6, 6, 6));
			offset += VolumetricPrimitive::Count;
		}

		// TODO-2.5: Allocate an upload buffer for this AABB data.
		// The base data lives in m_aabbs.data() (the stuff you filled in!), but the allocationg should be pointed
		// towards m_aabbBuffer.resource (the actual D3D12 resource that will hold all of our AABB data as a contiguous buffer).
		AllocateUploadBuffer(device, m_aabbs.data(), m_aabbs.size()*sizeof(m_aabbs), &m_aabbBuffer.resource);
	}
}

// TODO-2.5: Build geometry used in the project. As easy as calling both functions above :)
void DXProceduralProject::BuildGeometry()
{
	BuildPlaneGeometry();
	BuildProceduralGeometryAABBs();
}