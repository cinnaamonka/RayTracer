#include "SDL.h"
#include "SDL_surface.h"
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <vector>
#include <thread>

using namespace dae;

const int ALL_NUM_THREADS = std::thread::hardware_concurrency();
const int NUM_THREADS = ALL_NUM_THREADS - ALL_NUM_THREADS / 2;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	float aspectRatioFOV = (m_Width / static_cast<float>(m_Height)) * tan(dae::TO_RADIANS * pScene->GetCamera().fovAngle / 2);
	float reciprocalWidth = 1.0f / static_cast<float>(m_Width);
	float reciprocalHeight = 1.0f / static_cast<float>(m_Height);

	std::vector<std::thread> threads;
	int chunkHeight = m_Height / NUM_THREADS;

	auto renderChunkFunc = [&](int startY, int endY) {
		RenderChunk(pScene, startY, endY, aspectRatioFOV, reciprocalWidth, reciprocalHeight);
	};

	for (int i = 0; i < NUM_THREADS; ++i) {
		int startY = i * chunkHeight;
		int endY = (i == NUM_THREADS - 1) ? m_Height : (startY + chunkHeight);

		threads.emplace_back(renderChunkFunc, startY, endY);
	}

	for (std::thread& thread : threads) {
		thread.join();
	}

	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderChunk(Scene* pScene, int startY, int endY, float aspectRatioFOV, float reciprocalWidth, float reciprocalHeight) const
{
	Camera& camera = pScene->GetCamera();

	float FOV = tan((dae::TO_RADIANS * camera.fovAngle) / 2);

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const Matrix cameraToWorld = camera.CalculateCameraToWorld();


	for (int px = 0; px < m_Width; ++px) 
	{
		float cx = (((2.0f * (static_cast<float>(px) + 0.5f)) * reciprocalWidth) - 1.0f) * aspectRatioFOV;

		for (int py = startY; py < endY; ++py) 
		{
			float cy = (1.0f - ((2.0f * static_cast<float>(py) + 0.5f) * reciprocalHeight)) * FOV;

			Vector3 viewRayDirection = { cx * Vector3::UnitX + cy * Vector3::UnitY + Vector3::UnitZ };
			viewRayDirection.Normalize();

			Vector3 cameraSpaceDirection = { cx, cy, 1.0 };

			Ray viewRay = Ray(camera.origin, cameraToWorld.TransformVector(cameraSpaceDirection));

			Vector3 v = viewRay.direction.Normalized() * (-1.0f);

			HitRecord closestHit{};

			ColorRGB finalColor{};

			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit) {
				for (const Light& light : lights) {
					Vector3 startingPoint = closestHit.origin + closestHit.normal * 0.001f;
					Vector3 directionHitToLight = light.origin - startingPoint;

					const float distance = directionHitToLight.Magnitude();

					Vector3 l = (light.origin - closestHit.origin).Normalized();

					Ray lightRay{
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
	}

	SDL_UpdateWindowSurface(m_pWindow);
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
