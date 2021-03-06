#include "kajiya_path_tracer.h"
#include "core_settings.h"
#include "ray.h"
#include "material.h"
#include "triangle.h"
#include "light.h"
#include "tuple"
#include "vector"

vector<Triangle*> KajiyaPathTracer::scene = vector<Triangle*>();
vector<Triangle*> KajiyaPathTracer::lights = vector<Triangle*>();
vector<CoreMaterial> KajiyaPathTracer::materials;
vector<BVH*> KajiyaPathTracer::bvhs;

float4 KajiyaPathTracer::globalIllumination = make_float4(0.2, 0.2, 0.2, 0);

void KajiyaPathTracer::Initialise() {
	/** Lights */
	// 15, 30, 25
	lights.push_back(new Triangle(
		make_float4(-15, 60, 0, 0),
		make_float4(15, 60, 0, 0),
		make_float4(0, 60, 15, 0),
		0
	));
}

/** Keeps track if the camera has moved */
int KajiyaPathTracer::stillFrames = 1;
float3 KajiyaPathTracer::oldCameraPos = make_float3(0, 0, 0);
float3 KajiyaPathTracer::oldCameraP1 = make_float3(0, 0, 0);
float3 KajiyaPathTracer::oldCameraP2 = make_float3(0, 0, 0);
float3 KajiyaPathTracer::oldCameraP3 = make_float3(0, 0, 0);

/** Adaptive sampling */
float KajiyaPathTracer::samplingThreshold = 8;
float KajiyaPathTracer::targetVariance = 0.05;

uint* KajiyaPathTracer::numberOfSamples = new uint[SCRHEIGHT * SCRWIDTH];
float4* KajiyaPathTracer::sums = new float4[SCRHEIGHT * SCRWIDTH];
float4* KajiyaPathTracer::sumSquared = new float4[SCRHEIGHT * SCRWIDTH];

int KajiyaPathTracer::recursionThreshold = 3;
Ray KajiyaPathTracer::primaryRay = Ray(make_float4(0, 0, 0, 0), make_float4(0, 0, 0, 0));
Ray KajiyaPathTracer::shadowRay = Ray(make_float4(0, 0, 0, 0), make_float4(0, 0, 0, 0));



void KajiyaPathTracer::Render(const ViewPyramid& view, const Bitmap* screen) {
	bool cameraStill = (
		view.pos == KajiyaPathTracer::oldCameraPos && 
		view.p1 == KajiyaPathTracer::oldCameraP1 && 
		view.p2 == KajiyaPathTracer::oldCameraP2 && 
		view.p3 == KajiyaPathTracer::oldCameraP3
	);

	if (!cameraStill) {
		KajiyaPathTracer::ResetAdaptiveSampling();
	}

	int varianceCount = 0;

	for (int y = 0; y < screen->height; y++) {
		for (int x = 0; x < screen->width; x++) {
			int index = x + y * screen->width;
			KajiyaPathTracer::TraceRay(view, screen, x, y, cameraStill);
			if (cameraStill) {
				float variance = KajiyaPathTracer::EstimateSampleVariance(index);
				if (variance > targetVariance) {
					float samples = variance / targetVariance;
					int amountSamples = min(samples * samples, KajiyaPathTracer::samplingThreshold) ;
					varianceCount += amountSamples;

					for (int i = 0; i < amountSamples; i++) {
						KajiyaPathTracer::TraceRay(view, screen, x, y, cameraStill);
					}
				}
			}
			/** Update Screen */
			screen->pixels[index] = KajiyaPathTracer::ConvertColorToInt(KajiyaPathTracer::sums[index] / KajiyaPathTracer::numberOfSamples[index]);
		}
	}

	cout << "Variance Count: " << varianceCount << endl;

	/** Update the old position of the camera */
	KajiyaPathTracer::oldCameraPos = view.pos;
	KajiyaPathTracer::oldCameraP1 = view.p1;
	KajiyaPathTracer::oldCameraP2 = view.p2;
	KajiyaPathTracer::oldCameraP3 = view.p3;
	
	/** Keeps track of how many still frames have passed by */
	if (cameraStill) {
		KajiyaPathTracer::stillFrames++;
	}
	else {
		KajiyaPathTracer::stillFrames = 1;
	}

	cout << "Amount of still frames: " << KajiyaPathTracer::stillFrames << endl;
}

void KajiyaPathTracer::TraceRay(const ViewPyramid& view, const Bitmap* screen, int x, int y, bool cameraStill) {
	/** Setup the ray from the screen */
	float3 point = KajiyaPathTracer::GetPointOnScreen(view, screen, x, y);
	float4 rayDirection = KajiyaPathTracer::GetRayDirection(view, point);

	/** Reset the primary, it can be used as a reflective ray */
	primaryRay.origin = make_float4(view.pos, 0);
	primaryRay.direction = rayDirection;

	/** Trace the ray */
	float4 color = primaryRay.Trace(KajiyaPathTracer::bvhs[0], true, 0);
	int index = x + y * screen->width;

	/** Update values for adaptive sampling */
	KajiyaPathTracer::numberOfSamples[index]++;
	KajiyaPathTracer::sums[index] += color;
	KajiyaPathTracer::sumSquared[index] += color * color;
}

void KajiyaPathTracer::AddTriangle(float4 v0, float4 v1, float4 v2, uint materialIndex) {
	Triangle* triangle = new Triangle(v0, v1, v2, materialIndex);
	scene.push_back(triangle);
}

/** Calculates the point on the camera screen given the x and y position */
float3 KajiyaPathTracer::GetPointOnScreen(const ViewPyramid& view, const Bitmap* screen, const int x, const int y) {
	float u = (float)x / (float)screen->width;
	float v = (float)y / (float)screen->height;
	float3 point = view.p1 + u * (view.p2 - view.p1) + v * (view.p3 - view.p1);
	return point;
}

/** Calculates the ray direction from the camera to the screen */
float4 KajiyaPathTracer::GetRayDirection(const ViewPyramid& view, float3 point) {
	float3 originToPoint = point - view.pos;
	float3 rayDirection = normalize((originToPoint) / length(originToPoint));
	return make_float4(rayDirection, 0);
}

int KajiyaPathTracer::ConvertColorToInt(float4 color) {
	int red = clamp((int)(color.x * 256), 0, 255);
	int green = clamp((int)(color.y * 256), 0, 255);
	int blue = clamp((int)(color.z * 256), 0, 255);
	return (blue << 16) + (green << 8) + red;
}

float4 KajiyaPathTracer::ConvertIntToColor(int color) {
	float red = color & 0xFF;
	float green = (color >> 8) & 0xFF;
	float blue = (color >> 16) & 0xFF;
	return make_float4(red, green, blue, 0) / 255;
}



void KajiyaPathTracer::ResetAdaptiveSampling() {
	for (int y = 0; y < SCRHEIGHT; y++) {
		for (int x = 0; x < SCRWIDTH; x++) {
			int index = x + y * SCRWIDTH;
			KajiyaPathTracer::sums[index] = make_float4(0);
			KajiyaPathTracer::sumSquared[index] = make_float4(0);
			KajiyaPathTracer::numberOfSamples[index] = 0;
		}
	}
}

float KajiyaPathTracer::EstimateSampleVariance(int index)
{
	float4 sum = KajiyaPathTracer::sums[index];
	float4 sum_sq = KajiyaPathTracer::sumSquared[index];
	uint n = KajiyaPathTracer::numberOfSamples[index];

	float4 variance = sum_sq / (n * (n - 1)) - sum * sum / ((n - 1) * n * n);

	return variance.x + variance.y + variance.z;
}
