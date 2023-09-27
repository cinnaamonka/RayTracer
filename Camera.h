#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right);

			return
			{
				right,
				up,
				forward,
				origin
			};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float pixelsPerStep = 1.0f;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
			{

				origin.z += pixelsPerStep * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{

				origin.z -= pixelsPerStep * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{

				origin.x += pixelsPerStep * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{

				origin.x -= pixelsPerStep * deltaTime;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float rotationSpeed{ 1.15f };

			std::cout << mouseX << std::endl;
			if (mouseState == SDL_BUTTON_LEFT)
			{
				totalYaw += float(mouseY) * rotationSpeed;

				totalPitch += float(mouseX) * rotationSpeed;

			}

			Matrix rotMat
			{
			Matrix::CreateRotation(TO_RADIANS * totalYaw, TO_RADIANS * totalPitch, 0.f)
			};

			forward = rotMat.TransformVector(Vector3::UnitZ);
		}
	};
}

