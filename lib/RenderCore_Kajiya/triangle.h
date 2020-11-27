#pragma once

#include "core_settings.h"
#include "ray.h"

class Triangle {
public:
	uint materialIndex;
	explicit Triangle(float4 _v0, float4 _v1, float4 _v2, uint _material);
	float Intersect(Ray& ray);
	float4 GetNormal();
	float CalculateEnergyFromLights(const float4 intersectionPoint);
	bool IsLightBlocked(float shadowRayLength);
private:
	float4 v0;
	float4 v1;
	float4 v2;
	float4 v0v2;
	float4 v0v1;
};

