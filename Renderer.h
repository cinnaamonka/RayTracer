#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;


namespace dae
{
	class Scene;

	struct Matrix;
	struct Vector3;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWWorld, const Vector3 cameraToOrigin) const;
		bool SaveBufferToImage() const;

		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; };
		void CycleLightingMode();

	private:
		SDL_Window* m_pWindow = {};

		SDL_Surface* m_pBuffer = {};
		uint32_t* m_pBufferPixels = {};

		enum class LightingMode
		{
			Combined,
			ObservedArea,
			Radiance,
			BRDF
		};

		LightingMode m_CurrentLightingMode = { LightingMode::Combined };


		int m_Width;
		int m_Height;

		bool m_ShadowsEnabled;
	};
}
