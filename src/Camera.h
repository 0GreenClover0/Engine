#pragma once
#include <memory>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

constexpr double default_yaw = -90.0;
constexpr double default_pitch = 0.0;
constexpr float default_speed = 2.5f;
constexpr double default_sensitivity = 0.1;

class Camera
{
public:
	static void set_main_camera(std::shared_ptr<Camera> const& camera)
	{
		main_camera = camera;
	}

	static std::shared_ptr<Camera> get_main_camera()
	{
		return main_camera;
	}

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	glm::mat4 projection;

	double yaw;
	double pitch;

	float speed;
	double sensitivity;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
	       double yaw = default_yaw, double pitch = default_pitch);
	Camera(float x, float y, float z, float up_x, float up_y, float up_z, double yaw, double pitch);

	glm::mat4 get_view_matrix() const;

	void update();
private:
	void update_camera_vectors();

	inline static std::shared_ptr<Camera> main_camera;
};