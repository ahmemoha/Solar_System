#pragma once

#include "Math.hpp"

class TurnTableCamera
{
public:

    struct Params
    {
        float defaultRadius = 0.0f;
        float defaultTheta = 0.0f;
        float defaultPhi = 0.0f;

        float defaultDistance = 5.0f;
        float minDistance = 1.0f;
        float maxDistance = 20.0f;
    };

    // Set the position the camera looks at
    void SetTargetPosition(const glm::vec3 &position)
    {
        m_targetPosition = position;
        _isDirty = true; // Mark that we need to update the view matrix
    }

    // Enum for which celestial body to follow
    enum class TargetBody
    {
        SUN,
        EARTH,
        MOON,
        NONE
    };
    void SetTargetBody(TargetBody body) { m_targetBody = body; }
    TargetBody GetTargetBody() const { return m_targetBody; }

    // For the bonus camera needs to be able to follow a target

    // Angles are in radians
    explicit TurnTableCamera(/*Transform & target*/);

    // Angles are in radians
    explicit TurnTableCamera(/*Transform & target, */const Params &params);

    // void ChangeTarget(Transform & target);

    void ChangeTheta(float deltaTheta);

    void ChangePhi(float deltaPhi);

    void ChangeRadius(float deltaRadius);

    // Update camera to follow moving target
    void UpdateTargetPosition(const glm::vec3 &position);


    [[nodiscard]]
    glm::mat4 ViewMatrix();

    [[nodiscard]]
    glm::vec3 Position();

    [[nodiscard]]
    glm::vec3 GetPosition() const
    {
        // Calculate camera position based on spherical coordinates
        float x = _distance * sinf(_phi) * cosf(_theta);
        float y = _distance * cosf(_phi); // y is "up" in our coordinate system
        float z = _distance * sinf(_phi) * sinf(_theta);
        return glm::vec3(x, y, z);
    }

private:

    glm::vec3 m_targetPosition{0.0f, 0.0f, 0.0f};  // The position we're looking at
    TargetBody m_targetBody = TargetBody::NONE;  // Which body we're following
    glm::vec3 m_lastTargetPosition{0.0f};  // Last known position of target
    void UpdateViewMatrix();  // Recalculates view matrix if needed

    // Transform * _target;

    float _distance{};
    float _minDistance{};
    float _maxDistance{};

    float _theta {};
    float _phi {};

    bool _isDirty = true;

    glm::mat4 _viewMatrix {};
    glm::vec3 _position {};
};
