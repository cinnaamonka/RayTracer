#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"
#include <iostream>

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// Calculate the vector from the ray's origin to the sphere's center
			Vector3 oc = ray.origin - sphere.origin;

			float a = Vector3::Dot(ray.direction, ray.direction);
			float b = Vector3::Dot(oc, ray.direction);
			float c = Vector3::Dot(oc, oc) - sphere.radius * sphere.radius;
			float discriminant = b * b - a * c;

			if (discriminant > 0)
			{
				float sqrtD = sqrtf(discriminant);

				// Find the nearest root that is in the acceptable range
				float t = (-b - sqrtD) / a;
				if (t < ray.min)
				{
					t = (-b + sqrtD) / a;
				}
				else
				{
					t = (-b - sqrtD) / a;
				}
				if (t < ray.max and t > ray.min && !ignoreHitRecord)
				{
					// Update hitRecord
					hitRecord.t = t;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = (hitRecord.origin - sphere.origin) / sphere.radius;
					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					return true;
				}
			}

			return false;
		}
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};

			return HitTest_Sphere(sphere, ray, temp, true);
		}


#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			float dotPlaneRay = Vector3::Dot(plane.normal, ray.direction);

			if (std::abs(dotPlaneRay) < 1e-6) {
				// Ray is parallel to the plane
				return false;
			}
			float t = Vector3::Dot(plane.normal, (plane.origin - ray.origin)) / dotPlaneRay;

			if (t >= ray.min && t < ray.max && !ignoreHitRecord) {
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.materialIndex = plane.materialIndex;

				hitRecord.normal = plane.normal;

				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS


		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			float dot = Vector3::Dot(triangle.normal, ray.direction);

			if (dot == 0)
			{
				return false;
			}

			if (ignoreHitRecord) {
				// For shadow rays
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (dot > 0) return false;
				}
				else if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (dot < 0) return false;
				}
			}
			else {
				// For regular rays 
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (dot < 0) return false;
				}
				else if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (dot > 0) return false;
				}
			}

			Vector3 L = triangle.v0 - ray.origin;

			float t = Vector3::Dot(L, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal);

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			Vector3 intersectionPoint = ray.origin + ray.direction * t;

			Vector3 e, p;

			e = triangle.v1 - triangle.v0;
			p = intersectionPoint - triangle.v0;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			e = triangle.v2 - triangle.v1;
			p = intersectionPoint - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			e = triangle.v0 - triangle.v2;
			p = intersectionPoint - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;

			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = triangle.normal;
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			HitRecord hit{};

			for (int i = 0; i < mesh.indices.size(); i += 3)
			{
				Triangle triangle = 
				{
					mesh.transformedPositions[mesh.indices[i]],
					mesh.transformedPositions[mesh.indices[i + 1]],
					mesh.transformedPositions[mesh.indices[i + 2]],
					mesh.transformedNormals[i / 3]
				};
				
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(triangle, ray, hit))
				{
					if (ignoreHitRecord)
						return true;

					if (hit.t < hitRecord.t)
						hitRecord = hit;

				}
			}
			if (hitRecord.didHit)
			{
				hitRecord.materialIndex = mesh.materialIndex;
				return true;
			}
			return false;
		}
	
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			Vector3 direction = { light.origin - origin };


			return  direction;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if (light.type == LightType::Point)
			{
				return { light.color * light.intensity / (light.origin - target).SqrMagnitude() };
			}
			if (light.type == LightType::Directional)
			{
				return { light.color * light.intensity };
			}
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}