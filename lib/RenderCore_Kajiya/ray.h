#pragma once
#include "core_settings.h"
#include "tuple"
#include "vector"

class KajiyaPathTracer;
class Triangle;
class Light;

class Ray
{
public:
	Ray(float4 _origin, float4 _direction);
	float4 origin;
	float4 direction;
	float4 GetIntersectionPoint(float intersectionDistance);
	float4 Trace(uint recursionDepth = 0);
	tuple<Triangle*, float> GetNearestIntersection();
	float4 DetermineColor(Triangle* triangle, CoreMaterial* material, float4 intersectionPoint, uint recursionDepth);
	float4 GetRefractionDirection(Triangle* triangle, CoreMaterial* material);
private:
	float4 GetSampleHemisphere(float r1, float r2);
	void CreateCoordinateSystem(float4& N, float4& Nt, float4& Nb);
};
