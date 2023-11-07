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
				origin += (step * deltaTime) * forward.Normalized();
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= (step * deltaTime) * forward.Normalized();
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{

				origin += (step * deltaTime) * right.Normalized();
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= (step * deltaTime) * right.Normalized();
			}

			

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			float rotationSpeed{ 1.5f };

			const bool isRightMousePressed{ mouseState == SDL_BUTTON_X1 };
			const bool isLeftMousePressed{ mouseState == SDL_BUTTON_LEFT };
			const bool areBothButtonsPressed{ mouseState == SDL_BUTTON_X2 };
			//RMB + Mouse Move X
			
			if (isRightMousePressed && mouseX)
			{
				totalYaw += TO_RADIANS * rotationSpeed * deltaTime * mouseX;
			}
			//RMB + Mouse Move Y
			if (isRightMousePressed && mouseY)
			{
				totalPitch += TO_RADIANS * mouseY * rotationSpeed * deltaTime;
			}
			//LMB + Mouse Move Y
			if (isLeftMousePressed && mouseY)
			{
				origin -= step * deltaTime * forward.Normalized() * mouseY;
			}
			//LMB + Mouse Move X
			if (isLeftMousePressed && mouseX)
			{
				totalYaw += TO_RADIANS * rotationSpeed * deltaTime * mouseX;
			}
			//(LMB + RMB + Mouse Move Y)
			if (areBothButtonsPressed && mouseY)
			{
				origin += step * deltaTime * up.Normalized() * mouseY;
			}


			const Matrix cameraToWorld = this->CalculateCameraToWorld();

			origin += cameraToWorld.TransformVector(movementDirection) * deltaTime;


			Matrix rotMat
			{
			Matrix::CreateRotation(totalPitch,totalYaw, 0.f)
			};


			forward = rotMat.TransformVector(Vector3::UnitZ);
		}
	};
}

