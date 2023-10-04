#pragma once

#include <cstdint>

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

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
		bool SaveBufferToImage() const;

		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; };
		void CycleLightingMode();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined

		};

		LightingMode m_CurrentLightingMode = { LightingMode::ObservedArea };

		int m_Width{};
		int m_Height{};

		bool m_ShadowsEnabled = true;
	};
}
