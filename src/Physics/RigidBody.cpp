#include "RigidBody.h"

#include <Core/Printer.h>

RigidBody::RigidBody(double mass_in, Transform& tr_ptr, bool is_st) : tr(tr_ptr), mass(static_cast<float>(mass_in)), is_static(is_st)
{
	//btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
	//startPosition = tr.position;
	//tr.matrix = construct_quat({ 0,0,0 }, 0);
}

void RigidBody::update(float dt)
{
	if (is_static)
		return;

	glm::vec3 acceleration = force_accumulator;
	acceleration *= mass;
	velocity = velocity + dt * acceleration;
	velocity *= 0.98;
	//tr.position = tr.position + dt * velocity;
	tr.setPosition(tr.getPosition() + dt * velocity);

	//TODO(darius) finish it
	/*glm::vec4 angular_accelerationtmp = glm::inverse(inertia_tensor) * glm::vec4(torque_accumulator.x, torque_accumulator.y, torque_accumulator.z, 0);
	glm::vec3 angular_acceleration = { angular_accelerationtmp.x, angular_accelerationtmp.y, angular_accelerationtmp.z };
	angular_velocity = angular_velocity + dt * angular_acceleration;
	angular_velocity *= 0.98;

	tr.matrix = glm::toMat4(construct_quat(angular_velocity, dt)) * tr.matrix;
	*/
}

void RigidBody::apply_impulse(const glm::vec3 impulse) {
	if (is_static) {
		return;
	}

	//if (glm::length(velocity) < 0.2)
	//	return;

	velocity = velocity + impulse;
}

void RigidBody::apply_epa(glm::vec3 epa)
{
	if (is_static)
		return;
	apply_impulse({ 0,1,0 });
	//tr.position += epa;
}

void RigidBody::apply_rotational_impulse(const glm::vec3& point, const glm::vec3& impulse) {
	if (is_static) {
		return;
	}

	glm::vec3 torque = glm::cross((point - tr.getPosition()), impulse);
	glm::vec3 angular_acceleration = glm::inverse(inertia_tensor) * glm::vec4(torque.x, torque.y, torque.z, 0);
	angular_velocity = angular_velocity + angular_acceleration;
}

void RigidBody::add_force(glm::vec3 force_in)
{
	if (glm::length(force_in) < 0.2)
		return;
	force_accumulator += glm::vec3(force_in.x, force_in.y, force_in.z);
}

void RigidBody::add_angular_force(glm::vec3 force_in)
{
	angular_velocity = force_in;//vec3{ force_in.x, force_in.y, force_in.z };
}

glm::mat4 RigidBody::create_box_inertia_tensor(float mass, const glm::vec3& half_lengths) {
	auto m = glm::mat4(
		(1.0 / 12.0) * mass * (half_lengths.x + half_lengths.y), 0.0, 0.0, 0.0,
		0.0, (1.0 / 12.0) * mass * (half_lengths.y + half_lengths.z), 0.0, 0.0,
		0.0, 0.0, (1.0 / 12.0) * mass * (half_lengths.z + half_lengths.x), 0.0,
		0.0, 0.0, 0.0, 1.0
	);
	inertia_tensor = m;

	return m;
}

void RigidBody::reset()
{
	//tr.getPosition = { 0,0,0 };
	tr.setPosition({0,0,0});
	velocity = { 0,0,0 };
}

void RigidBody::resetFroces()
{
	//tr.getPosition = { 0,0,0 };
	velocity = { 0,0,0 };
	angular_velocity = {0,0,0};
	//force_accumulator = {0,0,0};
	torque_accumulator = {0,0,0};
}

void RigidBody::set_pos(glm::vec3 pos)
{
	tr.setPosition(pos);
}

glm::vec3 RigidBody::get_pos() const
{
	return tr.getPosition();
}

glm::mat4 RigidBody::get_quatmat()
{
	glm::mat4 RotationMatrix = tr.matrix;// glm::toMat4(tr.q);
	return RotationMatrix;
}

bool& RigidBody::get_is_static_ref()
{
	return is_static;
}

//TODO(darius) stop fucking with incapsulation
/*glm::quat& RigidBody::get_orientation_quaternion_ref()
{
	return tr.matrix;
}
*/

glm::quat RigidBody::normalize_quat(const glm::quat& q)
{
	float mag = glm::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);

	if (mag == 0) {
		return q;
	}

	return glm::quat(q.w / mag, q.x / mag, q.y / mag, q.z / mag);
}

glm::quat RigidBody::construct_quat(const glm::vec3& v, float theta)
{
	float temp = sin(theta / 2.0f);
	float x = temp * v.x;
	float y = temp * v.y;
	float z = temp * v.z;
	float w = cos(theta / 2.0f);
	glm::quat qres(w, x, y, z);

	return normalize_quat(qres);

}

glm::quat RigidBody::multiply_quats(const glm::quat& u, const glm::quat& v)
{
	float x = v.w * u.x + v.x * u.w - v.y * u.z + v.z * u.y;
	float y = v.w * u.y + v.x * u.z + v.y * u.w - v.z * u.x;
	float z = v.w * u.z - v.x * u.y + v.y * u.x + v.z * u.w;
	float w = v.w * u.w - v.x * u.x - v.y * u.y - v.z * u.z;

	return normalize_quat(glm::quat(w, x, y, z));
}

void RigidBody::set_quat_from_angles()
{
	if (!is_static)
		return;

	glm::mat4 transformX = glm::eulerAngleX(ex);
	glm::mat4 transformY = glm::eulerAngleY(ey);
	glm::mat4 transformZ = glm::eulerAngleZ(ez);

	glm::mat4 transform1 =
		transformX * transformY * transformZ;

	//tr.q = glm::quat_cast(transform1);
}

float& RigidBody::get_ex() {
	return ex;
}

float& RigidBody::get_ey() {
	return ey;
}

float& RigidBody::get_ez() {
	return ez;
}

