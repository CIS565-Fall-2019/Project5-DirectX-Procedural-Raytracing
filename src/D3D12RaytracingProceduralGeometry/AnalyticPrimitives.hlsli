#ifndef ANALYTICPRIMITIVES_H
#define ANALYTICPRIMITIVES_H

// LOOKAT-1.9.3: AnalyticPrimitives.hlsli, contains all logic used to test intersection against analytical geometries (AABB, Spheres)

#include "RaytracingShaderHelper.hlsli"

/********************************************************************************************************/
/************************************************* AABB *************************************************/
/********************************************************************************************************/

// Test if a ray segment <RayTMin(), RayTCurrent()> intersects a hollow Box.
bool RayBoxIntersectionTest(Ray ray, float3 aabb[2], out float tmin, out float tmax)
{
	float3 tmin3, tmax3;
	int3 sign3 = ray.direction > 0;
	tmin3.x = (aabb[1 - sign3.x].x - ray.origin.x) / ray.direction.x;
	tmax3.x = (aabb[sign3.x].x - ray.origin.x) / ray.direction.x;

	tmin3.y = (aabb[1 - sign3.y].y - ray.origin.y) / ray.direction.y;
	tmax3.y = (aabb[sign3.y].y - ray.origin.y) / ray.direction.y;

	tmin3.z = (aabb[1 - sign3.z].z - ray.origin.z) / ray.direction.z;
	tmax3.z = (aabb[sign3.z].z - ray.origin.z) / ray.direction.z;

	tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
	tmax = min(min(tmax3.x, tmax3.z), tmax3.z);

	return tmax > tmin && tmax >= RayTMin() && tmin <= RayTCurrent();
}

// Test if a ray with RayFlags and segment <RayTMin(), RayTCurrent()> intersects a hollow AABB.
bool RayAABBIntersectionTest(Ray ray, float3 aabb[2], out float thit, out ProceduralPrimitiveAttributes attr)
{
	float tmin, tmax;
	if (RayBoxIntersectionTest(ray, aabb, tmin, tmax))
	{
		thit = tmin >= RayTMin() ? tmin : tmax;

		// Set a normal to the normal of a face the hit point lays on.
		float3 hitPosition = ray.origin + thit * ray.direction;
		float3 distanceToBounds[2] = {
			abs(aabb[0] - hitPosition),
			abs(aabb[1] - hitPosition)
		};

		// Determine which face the point is closest to.
		const float eps = 0.0001;
		if (distanceToBounds[0].x < eps) attr.normal = float3(-1, 0, 0);
		else if (distanceToBounds[0].y < eps) attr.normal = float3(0, -1, 0);
		else if (distanceToBounds[0].z < eps) attr.normal = float3(0, 0, -1);
		else if (distanceToBounds[1].x < eps) attr.normal = float3(1, 0, 0);
		else if (distanceToBounds[1].y < eps) attr.normal = float3(0, 1, 0);
		else if (distanceToBounds[1].z < eps) attr.normal = float3(0, 0, 1);

		// Does some range check and culling check
		return is_a_valid_hit(ray, thit, attr.normal);
	}
	return false;
}

/********************************************************************************************************/
/************************************************* Sphere ***********************************************/
/********************************************************************************************************/

// Solve a quadratic equation ax^2 + bx + c = 0. x0 will be <= x1 in the output.
bool SolveQuadraticEqn(float a, float b, float c, out float x0, out float x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false; // imaginary solution
    else if (discr == 0) x0 = x1 = -0.5 * b / a; // 1 root
    else { // 2 roots
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) swap(x0, x1);

    return true;
}

// Analytic solution of an unbounded ray sphere intersection points.
// Sphere equation: (x - center)^2 = radius^2
// Ray equation: x = origin + t * direction
// Substitute x for origin + t * direction in sphere equation to solve for t. 
// This is equivalent to solving the good old quadratic equation.
bool SolveRaySphereIntersectionEquation(in Ray ray, out float tmin, out float tmax, in float3 center, in float radius)
{
    float3 L = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, L);
    float c = dot(L, L) - radius * radius;
    return SolveQuadraticEqn(a, b, c, tmin, tmax);
}

// Calculate a normal for a hit point on a sphere.
float3 CalculateNormalForARaySphereHit(in Ray ray, in float thit, float3 center)
{
	float3 hitPosition = ray.origin + thit * ray.direction;
	return normalize(hitPosition - center); // normal is direction of radius vector
}

// Test if a ray with RayFlags and segment <RayTMin(), RayTCurrent()> intersects a hollow sphere.
bool RaySphereIntersectionTest(in Ray ray, out float thit, out float tmax, in ProceduralPrimitiveAttributes attr, in float3 center = float3(0, 0, 0), in float radius = 1)
{
    float t0, t1; // solutions for t if the ray intersects 

	if (!SolveRaySphereIntersectionEquation(ray, t0, t1, center, radius)) {
		return false;
	}
    tmax = t1;

    if (t0 < RayTMin())
    {
        // t0 is before RayTMin, let's use t1 instead .
        if (t1 < RayTMin()) return false; // both t0 and t1 are before RayTMin

        attr.normal = CalculateNormalForARaySphereHit(ray, t1, center);
        if (is_a_valid_hit(ray, t1, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    else
    {
		// use t0.
        attr.normal = CalculateNormalForARaySphereHit(ray, t0, center);
        if (is_a_valid_hit(ray, t0, attr.normal))
        {
            thit = t0;
            return true;
        }

        attr.normal = CalculateNormalForARaySphereHit(ray, t1, center);
        if (is_a_valid_hit(ray, t1, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    return false;
}

// Test if a ray segment <RayTMin(), RayTCurrent()> intersects a solid sphere.
// Limitation: this test does not take RayFlags into consideration and does not calculate a surface normal.
bool RaySolidSphereIntersectionTest(in Ray ray, out float thit, out float tmax, in float3 center = float3(0, 0, 0), in float radius = 1)
{
    float t0, t1; // solutions for t if the ray intersects 

	if (!SolveRaySphereIntersectionEquation(ray, t0, t1, center, radius)) {
		return false;
	}

    // Since it's a solid sphere, clip intersection points to ray extents.
    thit = max(t0, RayTMin());
    tmax = min(t1, RayTCurrent());

    return true;
}

// TODO-3.4.1: Change this code to support intersecting multiple spheres (~3 spheres). 
// You can hardcode the local centers/radii of the spheres, just try to maintain them between 1 and -1 (and > 0 for the radii).
bool RayMultipleSpheresIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr)
{
	// Define the spheres in local space (within the aabb)
	float3 center = float3(-0.2, 0, -0.2);
	float radius = 0.7f;

    float3 center2 = float3(0.7, 0, 0.7);
    float radius2 = 0.2f;

    float3 center3 = float3(0.3, 0.7, 0.3);
    float radius3 = 0.4f;

	thit = INFINITY;
	float tmax, thitTemp;
	ProceduralPrimitiveAttributes attrTemp;
	bool hit = false;
	if (RaySphereIntersectionTest(ray, thitTemp, tmax, attrTemp, center, radius) && thitTemp < thit)
	{
		thit = thitTemp;
		attr = attrTemp;
		hit = true;
	}
	if (RaySphereIntersectionTest(ray, thitTemp, tmax, attrTemp, center2, radius2) && thitTemp < thit)
	{
		thit = thitTemp;
		attr = attrTemp;
		hit = true;
	}
	if (RaySphereIntersectionTest(ray, thitTemp, tmax, attrTemp, center3, radius3) && thitTemp < thit)
	{
		thit = thitTemp;
		attr = attrTemp;
		hit = true;
	}

	if (!hit)
	{
		thit = RayTCurrent();
	}
	return hit;
}

#endif // ANALYTICPRIMITIVES_H