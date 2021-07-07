#pragma once

#include "core_settings.h"

class BVHNode;
class Bin;

class BVH
{
public:
	static int binCount;
	static Bin* bins;
	static Bin* binsLeft;
	static Bin* binsRight;

	BVHNode* pool;
	BVHNode* root;
	int* triangleIndices;
	BVH(int triangleIndex, int triangleCount);
};

