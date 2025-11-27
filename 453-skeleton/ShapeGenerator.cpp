//
// Created by moham on 3/15/2025.
//

#include "ShapeGenerator.hpp"

#include <glm/ext/matrix_transform.hpp>

//======================================================================================================================
CPU_Geometry ShapeGenerator::Sphere(float const radius, int const slices, int const stacks)
{
    CPU_Geometry geom{};

    // Generate vertices
    for (int i = 0; i <= stacks; ++i) // stacks = horizontal slices
    {
        float v = i / (float)stacks; // 0 to 1
        float phi = v * glm::pi<float>(); // 0 to pi (top to bottom)

        for (int j = 0; j <= slices; ++j) // slices = vertical cuts
        {
            float u = j / (float)slices; // 0 to 1
            float theta = u * (glm::pi<float>() * 2); // 0 to 2pi (around)

            // Calculate vertex position
            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = sinf(theta) * sinf(phi);

            geom.positions.emplace_back(x * radius, y * radius, z * radius);
            geom.normals.emplace_back(x, y, z); // normal is same as position for unit sphere
            geom.uvs.emplace_back(u, 1.0f - v); // flip v coordinate so texture isn't upside down

            // Simple color gradient for debugging
            geom.colors.emplace_back(u, v, 0.5f);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            int first = (i * (slices + 1)) + j; // indices for two triangles forming a quad
            int second = first + slices + 1;

            // first triangle
            geom.indices.push_back(first);
            geom.indices.push_back(second);
            geom.indices.push_back(first + 1);

            // second triangle
            geom.indices.push_back(second);
            geom.indices.push_back(second + 1);
            geom.indices.push_back(first + 1);
        }
    }

    return geom; // sphere
}

//======================================================================================================================

static void colouredTriangles(CPU_Geometry &geom, glm::vec3 col);
static void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);
static void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom);

CPU_Geometry ShapeGenerator::UnitCube()
{
    std::vector<glm::vec3> originQuad{};
    originQuad.emplace_back(-0.5, 0.5, 0.0); // top-left
    originQuad.emplace_back(-0.5, -0.5, 0.0); // bottom-left
    originQuad.emplace_back(0.5, 0.5, 0.0); // top-right

    originQuad.emplace_back(-0.5, -0.5, 0.0); // bottom-left
    originQuad.emplace_back(0.5, -0.5, 0.0); // bottom-right
    originQuad.emplace_back(0.5, 0.5, 0.0); // top-right

    CPU_Geometry square{};

    positiveZFace(originQuad, square);
    colouredTriangles(square, {1.f, 1.f, 0.f});

    positiveXFace(originQuad, square);
    colouredTriangles(square, {1.f, 0.f, 0.f});

    negativeZFace(originQuad, square);
    colouredTriangles(square, {0.f, 1.f, 0.f});

    negativeXFace(originQuad, square);
    colouredTriangles(square, {0.f, 0.f, 1.f});

    positiveYFace(originQuad, square);
    colouredTriangles(square, {1.f, 0.f, 1.f});

    negativeYFace(originQuad, square);
    colouredTriangles(square, {0.f, 1.f, 1.f});

    return square;
}

CPU_Geometry ShapeGenerator::Ring(float innerRadius, float outerRadius, int segments)
{
    CPU_Geometry geom;

    // Generate vertices
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = cos(angle);
        float z = sin(angle);

        // Outer vertex
        geom.positions.emplace_back(x * outerRadius, 0.0f, z * outerRadius);
        geom.normals.emplace_back(0.0f, 1.0f, 0.0f);
        geom.uvs.emplace_back(1.0f, angle / (2.0f * glm::pi<float>()));
        geom.colors.emplace_back(1.0f, 1.0f, 1.0f);

        // Inner vertex
        geom.positions.emplace_back(x * innerRadius, 0.0f, z * innerRadius);
        geom.normals.emplace_back(0.0f, 1.0f, 0.0f);
        geom.uvs.emplace_back(0.0f, angle / (2.0f * glm::pi<float>()));
        geom.colors.emplace_back(1.0f, 1.0f, 1.0f);
    }

    // Generate indices
    for (int i = 0; i < segments; i++)
    {
        int outer1 = i * 2;
        int inner1 = i * 2 + 1;
        int outer2 = (i + 1) * 2;
        int inner2 = (i + 1) * 2 + 1;

        geom.indices.push_back(outer1);
        geom.indices.push_back(inner1);
        geom.indices.push_back(outer2);

        geom.indices.push_back(inner1);
        geom.indices.push_back(inner2);
        geom.indices.push_back(outer2);
    }

    return geom;
}

void colouredTriangles(CPU_Geometry &geom, glm::vec3 col)
{
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
    geom.colors.emplace_back(col);
}

void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.5));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(T * glm::vec4((*i), 1.0));
    }
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
    geom.normals.emplace_back(0.0, 0.0, 1.0);
}

void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(1.0, 0.0, 0.0));
}

void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
    geom.normals.emplace_back(glm::vec3(0.0, 0.0, -1.0));
}

void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
    geom.normals.emplace_back(glm::vec3(-1.0, 0.0, 0.0));
}

void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, 1.0, 0.0));
}

void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom)
{
    const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    for (auto i = originQuad.begin(); i < originQuad.end(); ++i)
    {
        geom.positions.emplace_back(glm::vec3(T * R * glm::vec4((*i), 1.0)));
    }
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
    geom.normals.emplace_back(glm::vec3(0.0, -1.0, 0.0));
}

//======================================================================================================================
