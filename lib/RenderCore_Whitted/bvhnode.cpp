#include "bvhnode.h"
#include "whitted_ray_tracer.h"
#include "triangle.h"
#include "ray.h"
#include "tuple"

void BVHNode::SubdivideNode(BVHNode* pool, int* triangleIndices, int &poolPtr) {
	if (this->count < 5) return;
	cout << "Amount BVHNode " << poolPtr << "\n";
	this->left = poolPtr;
	poolPtr += 2;
	this->PartitionTriangles(pool, triangleIndices);
	BVHNode* left = &pool[this->left];
	BVHNode* right = &pool[this->left + 1];
	if (left->count == 0 || right->count == 0) { return; }
	left->SubdivideNode(pool, triangleIndices, poolPtr);
	right->SubdivideNode(pool, triangleIndices, poolPtr);
	this->isLeaf = false;
}

void swap(int* triangleIndices, int x, int y) {
	int prev = triangleIndices[x];
	triangleIndices[x] = triangleIndices[y];
	triangleIndices[y] = prev;
}


/** https://www.geeksforgeeks.org/iterative-quick-sort/ */
int partition(int axis, float splitPoint, int* triangleIndices, int start, int end) {
	int x = triangleIndices[end];
	int i = start - 1;

	for (int j = start; j <= end; j++) {
		Triangle* triangle = WhittedRayTracer::scene[triangleIndices[j]];

		/** Determine sort point */
		float trianglePoint;
		if (axis == 0) { trianglePoint = triangle->centroid.x; } 
		else if (axis == 1) { trianglePoint = triangle->centroid.y;} 
		else { trianglePoint = triangle->centroid.z; }

		if (trianglePoint <= splitPoint) {
			i++;
			swap(triangleIndices, i, j);
		}
	}

	swap(triangleIndices, i + 1, end);
	return i + 1;
}

void BVHNode::PartitionTriangles(BVHNode* pool, int* triangleIndices) {
	int axis = this->bounds.LongestAxis();
	float splitPoint = this->bounds.Center(axis);

	cout << "Axis: " << axis << "\n";
	cout << "SplitPoint: " << splitPoint << "\n";
	cout << "First: " << this->first << "\n";
	cout << "Count: " << this->count << "\n";

	int j = partition(axis, splitPoint, triangleIndices, this->first, this->first + this->count - 1);

	BVHNode* left = &pool[this->left];
	BVHNode* right = &pool[this->left + 1];

	left->first = this->first;
	left->count = j;
	right->first = j + this->first;
	right->count = this->count - j;

	left->UpdateBounds(triangleIndices);
	right->UpdateBounds(triangleIndices);
}

void BVHNode::UpdateBounds(int* triangleIndices) {
	this->bounds = aabb();
	
	for (int i = 0; i < this->count; i++) {
		Triangle* triangle = WhittedRayTracer::scene[triangleIndices[this->first + i]];

		this->UpdateBounds(triangle->v0);
		this->UpdateBounds(triangle->v1);
		this->UpdateBounds(triangle->v2);
	}
}

void BVHNode::UpdateBounds(float4 point) {
	this->bounds.Grow(make_float3(point));
}

void BVHNode::Traverse(Ray &ray, BVHNode* pool, int* triangleIndices, tuple<Triangle*, float> &intersection) {
	if (get<0>(intersection) != NULL) { return; }

	float test;
	if (!ray.IntersectionBounds(this->bounds, test)) { return; }

	if (this->isLeaf) { 
		this->IntersectTriangles(ray, triangleIndices, intersection); 
		return; 
	}
	
	BVHNode* left = &pool[this->left];
	BVHNode* right = &pool[this->left + 1];

	float leftNodeDist;
	bool hitLeft = ray.IntersectionBounds(left->bounds, leftNodeDist);
	float rightNodeDist;
	bool hitRight = ray.IntersectionBounds(right->bounds, rightNodeDist);

	if (!hitLeft && !hitRight) { return; }

	if (hitLeft && !hitRight) {
		left->Traverse(ray, pool, triangleIndices, intersection);
	}
	else if (!hitLeft && hitRight) {
		right->Traverse(ray, pool, triangleIndices, intersection);
	} else if (leftNodeDist < rightNodeDist) {
		left->Traverse(ray, pool, triangleIndices, intersection);
		right->Traverse(ray, pool, triangleIndices, intersection);
	} else {
		right->Traverse(ray, pool, triangleIndices, intersection);
		left->Traverse(ray, pool, triangleIndices, intersection);
	}
}

void BVHNode::IntersectTriangles(Ray &ray, int* triangleIndices,  tuple<Triangle*, float> &intersection) {
	float minDistance = NULL;
	Triangle* nearestPrimitive = NULL;

	for (int i = 0; i < this->count; i++) {
		Triangle* triangle = WhittedRayTracer::scene[triangleIndices[this->first + i]];
		float distance = triangle->Intersect(ray);

		if (
			((minDistance == NULL) || (distance < minDistance))
			&& (distance > EPSILON)
			) {
			minDistance = distance;
			nearestPrimitive = triangle;
		}
	}

	intersection = make_tuple(nearestPrimitive, minDistance);
}

void BVHNode::Swap(int* triangleIndices, int x, int y) {
	int prev = triangleIndices[x];
	triangleIndices[x] = triangleIndices[y];
	triangleIndices[y] = prev;
}
