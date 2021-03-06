#pragma once
#include "core_settings.h"
#include "tuple"
#include "vector"

class Ray;
class BVH;
class Triangle;

class KajiyaPathTracer
{
public:
	static int recursionThreshold;
	static vector<Triangle*> scene;
	static vector<Triangle*> lights;
	static vector<CoreMaterial> materials;
	static vector<BVH*> bvhs;

	static Ray primaryRay;
	static Ray shadowRay;

	static float4 globalIllumination;

	static void Initialise();
	static void AddTriangle(float4 v0, float4 v1, float4 v2, uint materialIndex);
	static void Render(const ViewPyramid& view, const Bitmap* screen);
	static void TraceRay(const lighthouse2::ViewPyramid& view, const lighthouse2::Bitmap* screen, int x, int y, bool cameraStill);
private:

	/** Old camera position */
	static int stillFrames;
	static float3 oldCameraPos;
	static float3 oldCameraP1;
	static float3 oldCameraP2;
	static float3 oldCameraP3;

	/** Adaptive sampling */
	static float samplingThreshold;
	static float targetVariance;
	static uint* numberOfSamples;
	static float4* sums;
	static float4* sumSquared;
	static void ResetAdaptiveSampling();
	static float EstimateSampleVariance(int index);

	static float3 GetPointOnScreen(const ViewPyramid& view, const Bitmap* screen, const int x, const int y);
	static float4 GetRayDirection(const ViewPyramid& view, float3 point);
	static int ConvertColorToInt(float4 color);
	static float4 ConvertIntToColor(int color);
};

