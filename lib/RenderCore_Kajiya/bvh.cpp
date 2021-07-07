#include "bvh.h"
#include "bvhnode.h"
#include "bin.h"
#include "vector"

int BVH::binCount = 16;

Bin* BVH::bins = new Bin[BVH::binCount];
Bin* BVH::binsLeft = new Bin[BVH::binCount - 1];
Bin* BVH::binsRight = new Bin[BVH::binCount - 1];

BVH::BVH(int triangleIndex, int triangleCount) {
	int size = triangleCount * 2 - 1;
	this->pool = new BVHNode[size];
	this->root = &this->pool[0];

	this->triangleIndices = new int[triangleCount];
	for (int i = 0; i < triangleCount; i++) {
		this->triangleIndices[i] = triangleIndex + i;
	}

	this->root->first = 0;
	this->root->count = triangleCount;
	this->root->UpdateBounds(this->triangleIndices);
	this->root->SubdivideNode(this->pool, this->triangleIndices, 0, size);
}