#include "SDL.h"
#include "SDL_surface.h"
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <vector>

//#define PARALLEL_EXECUTION
using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	float aspectRatio = m_Width / static_cast<float>(m_Height);

	float FOV = tan((dae::TO_RADIANS * camera.fovAngle) / 2);
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	uint32_t amountOfPixels = uint32_t(m_Width * m_Height);
#if defined(PARALLEL_EXECUTION)
	//define some code
#else
	for (uint32_t pixelIndex = 0; pixelIndex < amountOfPixels; pixelIndex++)
	{
		RenderPixel(pScene, pixelIndex, FOV, aspectRatio, cameraToWorld, camera.origin);
	}
#endif
	SDL_UpdateWindowSurface(m_pWindow);

}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWWorld, const Vector3 cameraToOrigin) const
{
	const std::vector<dae::Material*> materials = pScene->GetMaterials();
	const std::vector<dae::Light> lights = pScene->GetLights();

	const uint32_t px = pixelIndex % m_Width;
	const uint32_t py = pixelIndex / m_Width;

	float rx = px + 0.5f;
	float ry = py + 0.5f;
	float cx = (2.f * rx / static_cast<float>(m_Width) - 1.f) * aspectRatio * fov;
	float cy = (1.f - (2.f * ry) / static_cast<float>(m_Height)) * fov;

	Vector3 viewRayDirection = { cx * Vector3::UnitX + cy * Vector3::UnitY + Vector3::UnitZ };
	viewRayDirection.Normalize();

	Vector3 cameraSpaceDirection = { cx, cy ,1 };

	Ray viewRay = Ray(cameraToOrigin, cameraToWWorld.TransformVector(cameraSpaceDirection));

	Vector3 v = viewRay.direction.Normalized() * (-1.0f);

	HitRecord closestHit{};

	ColorRGB finalColor{};

	pScene->GetClosestHit(viewRay, closestHit);

	if (closestHit.didHit)
	{
		for (const Light& light : lights)
		{
			Vector3 startingPoint = closestHit.origin + closestHit.normal * 0.001f;
			Vector3 directionHitToLight = light.origin - startingPoint;

			const float distance = directionHitToLight.Magnitude();

			Vector3 l = (light.origin - closestHit.origin).Normalized();

			Ray lightRay
			{
				startingPoint,
				directionHitToLight.Normalized(),
				0.0001f,
				distance
			};

			lightRay.max = distance;

			float cosAngle = Vector3::Dot(closestHit.normal, lightRay.direction);

			if (m_ShadowsEnabled && pScene->DoesHit(lightRay)) continue;

			switch (m_CurrentLightingMode) {
			case LightingMode::ObservedArea:
				if (cosAngle < 0) continue;

				finalColor += ColorRGB{ cosAngle, cosAngle, cosAngle };

				break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(light, closestHit.origin);

				break;
			case LightingMode::BRDF:
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, l, v);

				break;
			case LightingMode::Combined:
				if (cosAngle < 0) continue;

				finalColor += (LightUtils::GetRadiance(light, closestHit.origin) *
					materials[closestHit.materialIndex]->Shade(closestHit, l, v) *
					cosAngle);

				break;
			}
		}
	}

	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}


bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	int currentLightingMode = static_cast<int>(m_CurrentLightingMode);
	++currentLightingMode %= 4;
	m_CurrentLightingMode = LightingMode{ currentLightingMode };
}
