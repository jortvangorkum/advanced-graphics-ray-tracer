#include "ray.h"
#include "core_settings.h"
#include "limits"
#include "triangle.h"
#include "kajiya_path_tracer.h"
#include "bvh.h"
#include "bvhnode.h"
#include "vector"

Ray::Ray(float4 _origin, float4 _direction) {
	origin = _origin;
	direction = _direction;
}

float4 Ray::GetIntersectionPoint(float intersectionDistance) {
	return origin + (direction * intersectionDistance);
}

bool Ray::IntersectionBounds(aabb& bounds, float& distance) {
	float4 invDir = 1.0 / this->direction;

	float4 bmin = make_float4(bounds.bmin[0], bounds.bmin[1], bounds.bmin[2], bounds.bmin[3]);
	float4 bmax = make_float4(bounds.bmax[0], bounds.bmax[1], bounds.bmax[2], bounds.bmax[3]);

	float4 t1 = (bmin - this->origin) * invDir;
	float4 t2 = (bmax - this->origin) * invDir;

	float4 tmin = fminf(t1, t2);
	float4 tmax = fmaxf(t1, t2);

	float dmin = max(tmin.x, max(tmin.y, tmin.z));
	float dmax = min(tmax.x, min(tmax.y, tmax.z));

	if (dmax < 0 || dmin > dmax) {
		return false;
	}
	distance = dmin;
	return true;
}

float4 Ray::Trace(BVH* bvh, bool lastSpecular, uint recursionDepth) {
	/** check if we reached our recursion depth */
	if (recursionDepth > KajiyaPathTracer::recursionThreshold) {
		return make_float4(0, 0, 0, 0);
	}

	/** Intersect BVH */
	tuple<Triangle*, float, Ray::HitType> nearestIntersection = make_tuple<Triangle*, float, Ray::HitType>(NULL, NULL, Ray::HitType::Nothing);
	bvh->root->Traverse(*this, bvh->pool, bvh->triangleIndices, nearestIntersection);
	/** Intersect Lights */
	nearestIntersection = IntersectLights(nearestIntersection);

	Triangle* nearestTriangle = get<0>(nearestIntersection);
	float intersectionDistance = get<1>(nearestIntersection);
	Ray::HitType hitType = get<2>(nearestIntersection);

	if (intersectionDistance > 0) {
		/** Hit a light */
		if (hitType == Ray::HitType::Light) {
			if (lastSpecular) {
				CoreMaterial lightMaterial = KajiyaPathTracer::materials[nearestTriangle->materialIndex];
				return make_float4(lightMaterial.color.value, 0);
			}
			else {
				return make_float4(0);
			}
		}

		CoreMaterial material = KajiyaPathTracer::materials[nearestTriangle->materialIndex];
		float4 BRDF = make_float4(material.color.value / PI, 0);
		float4 intersectionPoint = this->GetIntersectionPoint(intersectionDistance);

		/** 
		----- 
		Direct light 
		----- 
		*/
		float4 directLight = make_float4(0);
		
		int randomLightIndex = Rand(KajiyaPathTracer::lights.size() - 1);
		Triangle* randomLight = KajiyaPathTracer::lights[randomLightIndex];
		float4 randomLightPoint = randomLight->GetRandomPoint();

		float4 vectorToLight = randomLightPoint - intersectionPoint;
		KajiyaPathTracer::shadowRay.direction = normalize(vectorToLight);
		KajiyaPathTracer::shadowRay.origin = intersectionPoint + KajiyaPathTracer::shadowRay.direction * EPSILON;

		float4 lightNormal = randomLight->GetNormal();
		float ndotl = dot(nearestTriangle->GetNormal(), KajiyaPathTracer::shadowRay.direction);
		float nldotl = dot(lightNormal, -KajiyaPathTracer::shadowRay.direction);

	    float distanceToLight = length(vectorToLight);
		float solidAngle = (nldotl * randomLight->GetArea()) / (distanceToLight * distanceToLight);

		if (
			ndotl > 0 && 
			nldotl > 0
		) {
			tuple<Triangle*, float, Ray::HitType> lightIntersection = make_tuple<Triangle*, float, Ray::HitType>(NULL, NULL, Ray::HitType::Nothing);
			bvh->root->Traverse(KajiyaPathTracer::shadowRay, bvh->pool, bvh->triangleIndices, lightIntersection);
			Triangle* intersect = get<0>(lightIntersection);
			float directIntersectionDist = get<1>(lightIntersection);

			if (intersect == NULL || distanceToLight < directIntersectionDist + EPSILON) {
				CoreMaterial lightMaterial = KajiyaPathTracer::materials[randomLight->materialIndex];
				float misPDF = (1 / solidAngle) + (1 / (2.0 * PI));
				directLight = make_float4(lightMaterial.color.value, 0) * BRDF * (ndotl / misPDF) * KajiyaPathTracer::lights.size();
			}
		}


		/** 
		----- 
		Indirect light 
		----- 
		*/
		float4 normal = nearestTriangle->GetNormal();
		

		float randomChoice = RandomFloat();

		/** If material = reflection, given a certain chance it calculates the reflection color */
		float reflectionChance = KajiyaPathTracer::materials[nearestTriangle->materialIndex].reflection.value;
		if (randomChoice < reflectionChance) {
			this->direction = this->direction - 2.0f * normal * dot(normal, this->direction);
			this->origin = intersectionPoint + EPSILON * this->direction;
			return this->Trace(bvh, true, recursionDepth + 1);
		}

		/** If material = refraction, given a certain chance it calculates the refraction color */
		float refractionChance = KajiyaPathTracer::materials[nearestTriangle->materialIndex].refraction.value;
		if (randomChoice < refractionChance + reflectionChance) {
			float4 refractionDirection = this->GetRefractionDirection(nearestTriangle, &material);
			if (length(refractionDirection) > EPSILON) {
				this->origin = intersectionPoint + (refractionDirection * EPSILON);
				this->direction = refractionDirection;
				return this->Trace(bvh, true, recursionDepth + 1);
			}
		}

		/** Calculate a random direction on the hempisphere */
		float x = RandomFloat();
		float y = RandomFloat();
		float4 uniformSample = normalize(make_float4(UniformSampleSphere(x, y)));
		
		/** Flips the direction away from the normal if needed */
		float4 r = this->direction = (dot(uniformSample, normal) > 0) ? uniformSample : -uniformSample;
		this->origin = intersectionPoint + (this->direction * EPSILON);
		float4 hitColor = this->Trace(bvh, false, recursionDepth + 1);
		float misPDF = solidAngle > 0 ? (1 / solidAngle) + (1 / (2.0 * PI)) : (1 / (2.0 * PI));
		float4 indirectLight = (dot(r, normal) / misPDF) * BRDF * hitColor;

		return indirectLight + directLight;
	}

	return make_float4(0,0,0,0);
}

float4 Ray::GetRefractionDirection(Triangle* triangle, CoreMaterial* material) {
	float4 normal = triangle->GetNormal();
	float cosi = clamp(-1.0, 1.0, dot(this->direction, normal));
	float etai = 1;
	float etat = material->ior.value;
	float4 normalRefraction = normal;

	/** Outside the surface */
	if (cosi < 0) {
		cosi = -cosi;
	}

	/** Inside the surface */
	else {
		normalRefraction = -normal;
		std::swap(etai, etat);
	}

	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);

	if (k < 0) {
		return make_float4(0, 0, 0, 0);
	}
	else {
		return normalize(eta * this->direction + (eta * cosi - sqrtf(k)) * normalRefraction);
	}

}

tuple<Triangle*, float, Ray::HitType> Ray::IntersectLights(tuple<Triangle*, float, Ray::HitType> &intersection) {
	Triangle* nearestPrimitive = get<0>(intersection);
	float minDistance = get<1>(intersection);
	Ray::HitType hitType = get<2>(intersection);

	/** Intersect lights */
	for (int i = 0; i < KajiyaPathTracer::lights.size(); i++) {
		Triangle* triangle = KajiyaPathTracer::lights[i];
		float distance = triangle->Intersect(*this);

		if (
			((minDistance == NULL) || (distance < minDistance))
			&& (distance > 0)
			) {
			minDistance = distance;
			nearestPrimitive = triangle;
			hitType = Ray::HitType::Light;
		}
	}

	return make_tuple(nearestPrimitive, minDistance, hitType);
}



