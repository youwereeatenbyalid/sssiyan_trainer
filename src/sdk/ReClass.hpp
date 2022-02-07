#pragma once

#include <cstdint>
#include "Math.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
using Matrix3x3f = glm::mat3x3;
using Matrix3x4f = glm::mat3x4;
using Matrix4x4f = glm::mat4x4;

#pragma pack(push, r1, 1)
#include "ReClass_Internal.hpp"
#pragma pack(pop, r1)

#include "Enums_Internal.hpp"