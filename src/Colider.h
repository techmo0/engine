#pragma once

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <array>

#include <Transform.h>
#include <RigidBody.h>
#include <Simplex.h>

class Simplex;

class Colider
{
public:
    Colider(glm::vec3 size_in, Transform& tr_in, int tag_in = 0, bool active_in = true);

    glm::vec3 get_position() const;

    glm::vec3 get_size() const;
    glm::vec3& get_size_ref();

    std::vector<glm::vec3> get_points();

    bool contains_point(const glm::vec3& point);

	glm::vec3 support(glm::vec4 dir);
	glm::vec3 dotSupport(glm::vec4 dir);
	glm::vec3 supportOriented(glm::vec4 dir);

    //https://www.youtube.com/watch?v=ajv46BSqcK4&ab_channel=Reducible
	bool gjk(Colider* coll1, Colider* coll2);
	
	bool check_collision(const Colider& c) const;

	glm::vec3 get_pos() const;

	glm::vec3& get_pos_ref();

	int get_tag() const;

	float minX() const;

	float minY() const;

	float minZ() const;

	float maxX() const;

	float maxY() const;

	float maxZ() const;

	glm::vec3 get_render_start_point() const;
	glm::vec3& get_render_shift();

	glm::vec3 colider_position() const;

	glm::vec3 get_epa() 
	{
		auto res = epa_collision_value;
		epa_collision_value = {0,0,0};
		return res;
	}
	
	bool is_active() const {
		return active;
	}

	Transform& get_transform()
	{
		return tr;
	}

private:
    //not cool, dangle may happen
	Transform& tr;
	glm::vec3 size = {1,1,1};
	bool active = true;
	int tag = 0;
	glm::vec3 shift = {0,0,0};

	glm::vec3 epa_collision_value = { 0,0,0 };
};