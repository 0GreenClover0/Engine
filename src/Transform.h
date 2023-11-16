#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>

class Transform : public std::enable_shared_from_this<Transform>
{
public:
	void set_local_position(glm::vec3 const&);
	[[nodiscard]] glm::vec3 get_local_position() const;

	void set_local_scale(glm::vec3 const&);
	[[nodiscard]] glm::vec3 get_local_scale() const;

	void set_euler_angles(glm::vec3 const&);
	[[nodiscard]] glm::vec3 get_euler_angles() const;

	[[nodiscard]] glm::mat4 const& get_model_matrix();
	[[nodiscard]] bool is_local_dirty() const;
	[[nodiscard]] bool is_parent_dirty() const;

	void compute_model_matrix();
	void compute_model_matrix(glm::mat4 const&);

	void compute_local_model_matrix();

	void set_parent(std::shared_ptr<Transform> const& parent);
	void set_parent(std::weak_ptr<Transform> const& parent);

	std::vector<std::shared_ptr<Transform>> children;
	std::weak_ptr<Transform> parent = {};

protected:
	glm::vec3 m_local_position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_euler_angles = { 0.0f, 0.0f, 0.0f };
	glm::vec3 m_local_scale = { 1.0f, 1.0f, 1.0f };

	glm::mat4 m_model_matrix = glm::mat4(1.0f);
	glm::mat4 m_local_model_matrix = glm::mat4(1.0f);
	bool m_local_dirty = true;
	bool m_parent_dirty = true;

	[[nodiscard]] glm::mat4 get_local_model_matrix();

private:
	void add_child(std::shared_ptr<Transform> const& transform);
};
