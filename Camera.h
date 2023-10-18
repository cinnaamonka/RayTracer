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

		void HandleZoom(int zoomValue)
		{
			const float minFOV = 0.0f;
			const float maxFOV = 180.0f;

			fovAngle += zoomValue;
			fovAngle = std::max(minFOV, std::min(maxFOV, fovAngle));
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float step = 1.0f;

			Vector3 movementDirection{};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				movementDirection.z = step;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				movementDirection.z = -step;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{

				movementDirection.x = step;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				movementDirection.x = -step;
			}

			const Matrix cameraToWorld = this->CalculateCameraToWorld();

			origin += cameraToWorld.TransformVector(movementDirection) * deltaTime;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float rotationSpeed{ 1.15f };


			if (mouseState == SDL_BUTTON_LEFT)
			{
				totalYaw += TO_RADIANS * float(-mouseY) * rotationSpeed;

				totalPitch += TO_RADIANS * float(mouseX) * rotationSpeed;

			}


			Matrix rotMat
			{
			Matrix::CreateRotation(totalYaw,totalPitch, 0.f)
			};


			forward = rotMat.TransformVector(Vector3::UnitZ);
		}
	};
}

