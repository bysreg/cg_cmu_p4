#ifndef _SCENE_H_
#define _SCENE_H_

#include <SFML/System/String.hpp>
#include <scene/objmodel.hpp>
#include <renderer/camera.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include "glm/gtc/quaternion.hpp"
#include <glm/glm.hpp>

namespace bey
{
	struct BoundingBox
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	struct StaticModel
	{
		glm::vec3 position;
		glm::quat orientation;		
		glm::vec3 scale;		

		// you may want to change this when you build meshes
		const ObjModel * model;

		StaticModel() : scale(1.0, 1.0, 1.0)
		{
		}

		BoundingBox get_bounding_box() const; // return bounding box of this model in local position
	};

	struct DirectionalLight
	{
		glm::vec3 direction;
		glm::vec3 color;
		float ambient;
	};

	struct SpotLight
	{
		glm::vec3 position;
		glm::quat orientation;
		glm::vec3 color;
		float exponent;
		float angle;
		float base_radius;
		float Kc, Kl, Kq; // attenuation constants
		float cutoff;
		float correction;

		//animation
		glm::quat from;
		glm::quat to;
		bool is_slerping;

		SpotLight() : position(glm::vec3(0.0f, 0.0f, 0.0f)),
			orientation(glm::quat()),
			color(glm::vec3(0.0f, 0.0f, 0.0f)),
			exponent(0.0f),
			angle(0.0f),
			base_radius(0.0f),
			Kc(0.0f), Kl(0.0f), Kq(0.0f), 
			correction(1.0f), 
			is_slerping(false)
		{
		};
	};

	struct PointLight
	{
		glm::vec3 position;
		glm::vec3 color;
		float velocity;
		float Kc; // constant component of attenuation
		float Kl; // linear component of attenuation
		float Kq; // quadrat component of attenuation
		float cutoff; // calculate this only once based on the attenuation values

		static float calc_bounding_sphere_scale(float Kc, float Kl, float Kq, const glm::vec3& color);
	};

	class Scene {
	public:		

	private:
		std::unordered_map<std::string, ObjModel> objmodels;		
		std::vector<StaticModel> models;
		DirectionalLight sunlight;
		std::vector<SpotLight> spotlights;
		std::vector<PointLight> pointlights;		

	public:
		Scene();
		bool loadFromFile(std::string filename);
		~Scene();

		Camera camera;

		const StaticModel* get_static_models() const;
		size_t num_static_models() const;
		const DirectionalLight& get_sunlight() const;
		const PointLight* get_point_lights() const;
		PointLight* get_mutable_point_lights();
		size_t num_point_lights() const;
		const SpotLight* get_spot_lights() const;
		SpotLight* get_mutable_spot_lights();
		size_t num_spot_lights() const;

		BoundingBox bounding_box;
	};
}

#endif // #ifndef _SCENE_H_