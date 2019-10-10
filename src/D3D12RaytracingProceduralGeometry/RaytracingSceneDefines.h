#pragma once

// LOOKAT-1.5: Defines Enums/Namespaces to make code reusable.

#include "RayTracingHlslCompat.h"

// LOOKAT-1.5: The global root signature will contain each of these slots. See the top of Raytracing.hlsl to understand 
// where this fits on the GPU.
namespace GlobalRootSignature {
    namespace Slot {
        enum Enum {
            OutputView = 0,
            AccelerationStructure,
            SceneConstant,
            AABBattributeBuffer,
            VertexBuffers,
            Count
        };
    }
}

// We will create a local root signature that differs per type of geometry we render.
namespace LocalRootSignature {
    namespace Type {
        enum Enum {
            Triangle = 0,
            AABB,
            Count
        };
    }
}

// LOOKAT-1.5: Triangle local root signature: material buffer.
namespace LocalRootSignature {
    namespace Triangle {
        namespace Slot {
            enum Enum {
                MaterialConstant = 0,
                Count
            };
        }
        struct RootArguments {
            PrimitiveConstantBuffer materialCb;
        };
    }
}

// LOOKAT-1.5: AABB local root signature: material buffer and the instance buffer (which contains transforms)
namespace LocalRootSignature {
    namespace AABB {
        namespace Slot {
            enum Enum {
                MaterialConstant = 0,
                GeometryIndex,
                Count
            };
        }
        struct RootArguments {
            PrimitiveConstantBuffer materialCb;
            PrimitiveInstanceConstantBuffer aabbCB;
        };
    }
}

namespace LocalRootSignature {
    inline UINT MaxRootArgumentsSize()
    {
        return max(sizeof(Triangle::RootArguments), sizeof(AABB::RootArguments));
    }
}

namespace GeometryType {
    enum Enum {
        Triangle = 0,
        AABB,       // Procedural geometry with an application provided AABB.
        Count
    };
}

namespace GpuTimers {
    enum Enum {
        Raytracing = 0,
        Count
    };
}

// Bottom-level acceleration structures (BottomLevelASType).
// This project uses two BottomLevelASType, one for AABB and one for Triangle geometry.
// Mixing of geometry types within a BLAS is not supported.
namespace BottomLevelASType = GeometryType;

namespace IntersectionShaderType {
    enum Enum {
        AnalyticPrimitive = 0,
        VolumetricPrimitive,
        Count
    };
    inline UINT PerPrimitiveTypeCount(Enum type)
    {
        switch (type)
        {
        case AnalyticPrimitive: return AnalyticPrimitive::Count;
        case VolumetricPrimitive: return VolumetricPrimitive::Count;
        }
        return 0;
    }
    static const UINT MaxPerPrimitiveTypeCount =
        max(AnalyticPrimitive::Count, VolumetricPrimitive::Count);
    static const UINT TotalPrimitiveCount =
        AnalyticPrimitive::Count + VolumetricPrimitive::Count;
}

