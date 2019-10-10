#include "stdafx.h"
#include "DXProceduralProject.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// LOOKAT-1.8.2: We create a giant descriptor heap that will contain all the descriptor information about anything that is not 
// a raytracing pipeline state object (RTPSO).
// This will generally mean that you will have a descriptor per SRV (shader resource), UAV (unifrom access), or CBV (constant buffer)
void DXProceduralProject::CreateDescriptorHeap()
{
	auto device = m_deviceResources->GetD3DDevice();

	// Allocate a heap for 6 descriptors:
	// 2 - vertex and index  buffer SRVs
	// 1 - raytracing output texture SRV
	// 3 - 2x bottom + 1 top level acceleration structure fallback wrapped pointer UAVs
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 6;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
	NAME_D3D12_OBJECT(m_descriptorHeap);

	m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}