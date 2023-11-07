#pragma once
#include <string>
#include <vector>

#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;
	struct Light;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		bool DoesHit(const Ray& ray) const;

		const std::vector<Plane>& GetPlaneGeometries() const { return m_PlaneGeometries; }
		const std::vector<Sphere>& GetSphereGeometries() const { return m_SphereGeometries; }
		const std::vector<Light>& GetLights() const { return m_Lights; }
		const std::vector<Material*> GetMaterials() const { return m_Materials; }

	protected:
		std::string	sceneName;

		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};


		Camera m_Camera{};

		Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
		Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
		unsigned char AddMaterial(Material* pMaterial);
	};

	class Scene_W4_ReferenceScene final : public Scene
	{
	public:
		Scene_W4_ReferenceScene() = default;
		~Scene_W4_ReferenceScene() override = default;

		Scene_W4_ReferenceScene(const Scene_W4_ReferenceScene&) = delete;
		Scene_W4_ReferenceScene(Scene_W4_ReferenceScene&&) noexcept = delete;
		Scene_W4_ReferenceScene& operator=(const Scene_W4_ReferenceScene&) = delete;
		Scene_W4_ReferenceScene& operator=(Scene_W4_ReferenceScene&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;
	private:
		TriangleMesh* m_Meshes[3]{};
	};
	class Scene_W4_BunnyScene final : public Scene
	{
	public:
		Scene_W4_BunnyScene() = default;
		~Scene_W4_BunnyScene() override = default;

		Scene_W4_BunnyScene(const Scene_W4_BunnyScene&) = delete;
		Scene_W4_BunnyScene(Scene_W4_BunnyScene&&) noexcept = delete;
		Scene_W4_BunnyScene& operator=(const Scene_W4_BunnyScene&) = delete;
		Scene_W4_BunnyScene& operator=(Scene_W4_BunnyScene&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;
	private:
		TriangleMesh* m_pBunnyMesh = nullptr;
	};
	class Scene_LowpolyMan final : public Scene
	{
	public:
		Scene_LowpolyMan() = default;
		~Scene_LowpolyMan() override = default;

		Scene_LowpolyMan(const Scene_LowpolyMan&) = delete;
		Scene_LowpolyMan(Scene_LowpolyMan&&) noexcept = delete;
		Scene_LowpolyMan& operator=(const Scene_LowpolyMan&) = delete;
		Scene_LowpolyMan& operator=(Scene_LowpolyMan&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;
	private:
		TriangleMesh* m_pLowpolyMan = nullptr;
	};
}
