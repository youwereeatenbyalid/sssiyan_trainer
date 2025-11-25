#pragma once
#include <cmath>
#include <vector>
#include "imgui/imgui.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <glm/glm.hpp>

class WorldVisualizer {
public:
    static glm::mat3x3 CreateRotationMatrix(float pitchRad, float yawRad, float rollRad);

    static glm::vec3 RotatePoint(const glm::vec3& point, const glm::vec3& center, const glm::mat3x3& rotation);

    static bool IsValidScreenPos(float x, float y, float w);

    static bool WorldToScreen(const glm::vec3& worldPos, ImVec2& screenPos);

    static void SetViewProjectionMatrix(const glm::mat4x4& matrix);

    static void SetDrawList(ImDrawList* list = nullptr);

    static ImDrawList* GetDrawList();

    static void DrawWorldLine(const glm::vec3& start, const glm::vec3& end, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);

    static void DrawWorldTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, ImU32 color, float thickness);

    static void DrawWorldFilledTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, ImU32 color);

    static void DrawWorldSphere(const glm::vec3& center, float radius, ImU32 color = IM_COL32(255, 255, 255, 128), int segments = 32, float thickness = 1.0f, const glm::mat3x3* rot = nullptr);

    static void DrawWorldFilledSphere(const glm::vec3& center, float radius, ImU32 color = IM_COL32(255, 255, 255, 128));

    static void DrawWorldCapsule(const glm::vec3& start, const glm::vec3& end, float radius, ImU32 color = IM_COL32(255, 255, 255, 128), int segments = 32, float thickness = 1.0f, const glm::mat3x3* rot = nullptr);

    static void DrawWorldBox(const glm::vec3& min, const glm::vec3& max, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);

    static void DrawWorldPoint(const glm::vec3& pos, float size = 3.0f, ImU32 color = IM_COL32(255, 0, 0, 255));

    static void DrawWorldText(const glm::vec3& pos, const char* text, ImU32 color = IM_COL32(255, 255, 255, 255));

    static void DrawWorldArrow(const glm::vec3& start, const glm::vec3& end, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f, float arrowSize = 10.0f);

    static void DrawWorldPolyline(const std::vector<glm::vec3>& points, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f, bool closed = false);

    static void DrawWorldGrid(const glm::vec3& center, float size, int divisions = 10, ImU32 color = IM_COL32(128, 128, 128, 255), float thickness = 1.0f);

private:
    static ImDrawList* currentDrawList;
    static glm::mat4x4 viewProjectionMatrix;
};
