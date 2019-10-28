#ifndef SDFPRIMITIVES_H
#define SDFPRIMITIVES_H

#include "RaytracingShaderHelper.hlsli"

/********************************************************************************************************/
/************************************************* AABB *************************************************/
/********************************************************************************************************/
//https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

float sdTorus(float3 p, float2 t)
{
	float2 q = float2(length(p.xz) - t.x, p.y);
	return length(q) - t.y;
}

float3 opTwist(in float3 p)
{
	const float k = 10.0; // or some other amount
	float c = cos(k*p.y);
	float s = sin(k*p.y);
	float2x2  m = float2x2(c, -s, s, c);
	float3  q = float3(mul(m, p.xz), p.y);
	return q;
}
float RayMarch(in float3 position, in SDFPrimitive::Enum sdfPrimitive);

float3 sdfGetNormal(in float3 pos, in SDFPrimitive::Enum sdPrimitive)
{
	float2 e = float2(0.001, 0.0);
	return normalize(
		e.xyy * RayMarch(pos + e.xyy, sdPrimitive) + 
		e.yyx * RayMarch(pos + e.yyx, sdPrimitive) +
		e.yxy * RayMarch(pos + e.yxy, sdPrimitive) +
		e.xxx * RayMarch(pos + e.xxx, sdPrimitive));
}

//intersection test
bool RaysdfPrimitiveTest(in Ray ray,
	in SDFPrimitive::Enum sdPrimitive,
	out float thit, 
	out ProceduralPrimitiveAttributes attr)
{
	//raymarching
	float threshold = 0.0001;
	float t = RayTMin();
	int i = 0;
	while (t <= RayTCurrent())
	{
		if (i > 256) { break; }
		float3 curr_p = ray.origin + t * ray.direction;
		float dis = RayMarch(curr_p, sdPrimitive);
 
		if (dis <= threshold * t) {
			float3 curr_normal = sdfGetNormal(curr_p, sdPrimitive);
			if (is_a_valid_hit(ray, t, curr_normal)) {
				thit = t;
				attr.normal = curr_normal;
				return true;
			}
		}
		t += dis;
		i++;
	}
	return false;
}

#endif // ANALYTICPRIMITIVES_H