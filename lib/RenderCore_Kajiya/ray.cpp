#include "ray.h"
#include "core_settings.h"
#include "limits"
#include "triangle.h"
#include "kajiya_path_tracer.h"
#include "vector"

Ray::Ray(float4 _origin, float4 _direction) {
	origin = _origin;
	direction = _direction;
}

float4 Ray::GetIntersectionPoint(float intersectionDistance) {
	return origin + (direction * intersectionDistance);
}

float4 Ray::Trace(uint recursionDepth) {
	/** check if we reached our recursion depth */
	if (recursionDepth > KajiyaPathTracer::recursionThreshold) {
		return make_float4(0, 0, 0, 0);
	}

	tuple<Triangle*, float> nearestIntersection = Ray::GetNearestIntersection();
	Triangle* nearestTriangle = get<0>(nearestIntersection);
	float intersectionDistance = get<1>(nearestIntersection);

	if (intersectionDistance > 0) {
		/** Hit a light */
		if (nearestTriangle->materialIndex == -1) {
			return make_float4(1, 1, 1, 0);
		}

		CoreMaterial material = KajiyaPathTracer::materials[nearestTriangle->materialIndex];
		float4 normal = nearestTriangle->GetNormal();
		float4 intersectionPoint = this->GetIntersectionPoint(intersectionDistance);

		/** Hit a mirror */
		if (KajiyaPathTracer::materials[nearestTriangle->materialIndex].reflection.value > EPSILON) {
			this->direction = this->direction - 2.0f * normal * dot(normal, this->direction);;
			this->origin = intersectionPoint + EPSILON * this->direction;
			return this->Trace(recursionDepth + 1);
		}

		/** Hit a glass */
		if (KajiyaPathTracer::materials[nearestTriangle->materialIndex].refraction.value > EPSILON) {
			float4 refractionDirection = this->GetRefractionDirection(nearestTriangle, &material);
			if (length(refractionDirection) > EPSILON) {
				this->origin = intersectionPoint + (refractionDirection * EPSILON);
				this->direction = refractionDirection;
				return this->Trace(recursionDepth + 1);
			}
		}

		float4 Nt;
		float4 Nb;
		this->CreateCoordinateSystem(normal, Nt, Nb);

		float r1 = KajiyaPathTracer::GetRandomFloat(0, 1);
		float r2 = KajiyaPathTracer::GetRandomFloat(0, 1);
		float4 sample = this->GetSampleHemisphere(r1, r2);

		float4 sampleWorld = normalize(make_float4(
			sample.x * Nb.x + sample.y * normal.x + sample.z * Nt.x,
			sample.x * Nb.y + sample.y * normal.y + sample.z * Nt.y,
			sample.x * Nb.z + sample.y * normal.z + sample.z * Nt.z,
			0
		));

		this->origin = intersectionPoint + (sampleWorld * EPSILON);
		this->direction = sampleWorld;
		float4 tracedIllumination = this->Trace(recursionDepth + 1);
		float pdf = 1.0 / (2.0 * PI);
		float4 indirectIllumination = r1 * tracedIllumination / pdf;

		return indirectIllumination * make_float4(material.color.value, 0) / PI;
	}

	return make_float4(0,0,0,0);
}

float4 Ray::DetermineColor(Triangle* triangle, CoreMaterial* material, float4 intersectionPoint, uint recursionDepth) {
	float reflection = material->reflection.value;
	float refraction = material->refraction.value;
	float diffuse    = 1 - (reflection + refraction);

	float4 materialColor = make_float4(material->color.value, 0);
	float4 color = make_float4(0,0,0,0);
	float4 normal = triangle->GetNormal();

	if (diffuse > EPSILON) {
		float energy = 1;  triangle->CalculateEnergyFromLights(intersectionPoint);
		float4 diffuseColor = materialColor * energy;
		color += diffuse * diffuseColor;
	}

	if (reflection > EPSILON) {
		float4 reflectDir = this->direction - 2.0f * normal * dot(normal, this->direction);
		this->origin = intersectionPoint + (reflectDir * EPSILON);
		this->direction = normalize(reflectDir);
		color += this->Trace(recursionDepth + 1) * reflection;
	}

	if (refraction > EPSILON) {
		float4 refractionDirection = this->GetRefractionDirection(triangle, material);
		if (length(refractionDirection) > 0) {
			this->origin = intersectionPoint + (refractionDirection * EPSILON);
			this->direction = refractionDirection;
			color += this->Trace(recursionDepth + 1) * refraction;
		}

	}

	return color;
}

float4 Ray::GetSampleHemisphere(float r1, float r2) {
	float sinTheta = sqrtf(1 - r1 * r1);
	float phi = 2 * PI * r2;
	float x = sinTheta * cosf(phi);
	float z = sinTheta * sinf(phi);
	return make_float4(x, r1, z, 0);
}

void Ray::CreateCoordinateSystem(float4& N, float4& Nt, float4& Nb) {
	if (std::fabs(N.x) > std::fabs(N.y)) {
		Nt = make_float4(N.z, 0, -N.x, 0) / sqrtf(N.x * N.x + N.z * N.z);
	} else {
		Nt = make_float4(0, -N.z, N.y, 0) / sqrtf(N.y * N.y + N.z * N.z);
	}
	Nb = cross(N, Nt);
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



tuple<Triangle*, float> Ray::GetNearestIntersection() {
	float minDistance = NULL;
	Triangle* nearestPrimitive = NULL;


	/** Intersect scene objects */
	for (int i = 0; i < KajiyaPathTracer::scene.size(); i++) {
		Triangle* triangle = KajiyaPathTracer::scene[i];
		float distance = triangle->Intersect(*this);

		if (
			((minDistance == NULL) || (distance < minDistance))
			&& (distance > 0)
		) {
			minDistance = distance;
			nearestPrimitive = triangle;
		}
	}

	/** Intersect lights */
	for (int i = 0; i < KajiyaPathTracer::lights.size(); i++) {
		Triangle* triangle = KajiyaPathTracer::lights[i]->shape;
		float distance = triangle->Intersect(*this);

		if (
			((minDistance == NULL) || (distance < minDistance))
			&& (distance > 0)
			) {
			minDistance = distance;
			nearestPrimitive = triangle;
		}
	}

	return make_tuple(nearestPrimitive, minDistance);
}



