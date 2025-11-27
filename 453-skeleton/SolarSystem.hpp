#pragma once

#include "AssetPath.h"
#include "Geometry.h"
#include "InputManager.hpp"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Time.hpp"
#include "TurnTableCamera.hpp"

class SolarSystem
{
public:

    explicit SolarSystem();

    ~SolarSystem();

    void Run();

private:

    void Update(float deltaTime);

    void Render();

    void UI();

    void PrepareUnitSphereGeometry();

    void OnResize(int width, int height);

    void OnMouseWheelChange(double xOffset, double yOffset) const;

    std::shared_ptr<AssetPath> mPath{};
    std::shared_ptr<Time> mTime{};
    std::unique_ptr<Window> mWindow;
    std::shared_ptr<InputManager> mInputManager{};

    std::unique_ptr<ShaderProgram> mBasicShader{};

    // Textures for all our celestial bodies
    std::vector<std::unique_ptr<Texture>> mTextures;

    // Enum to identify textures 
    enum TextureIndex
    {
        SUN_TEXTURE,
        EARTH_DAY_TEXTURE, 
        EARTH_NIGHT_TEXTURE,
        EARTH_CLOUDS_TEXTURE,
        MOON_TEXTURE,
        SKY_TEXTURE,
        MERCURY_TEXTURE,
        VENUS_TEXTURE,
        MARS_TEXTURE,
        JUPITER_TEXTURE,
        SATURN_TEXTURE,
        SATURN_RING_TEXTURE,
        URANUS_TEXTURE,
        NEPTUNE_TEXTURE,
        NUM_TEXTURES
    };

    // geometry that stores all sphere geometries
    std::vector<std::unique_ptr<GPU_Geometry>> mUnitSphereGeometry;
    std::vector<int> mUnitSphereIndexCount;

    // Identifiers for different sphere geometries
    enum SphereIndex
    {
        SUN_GEOMETRY,
        EARTH_GEOMETRY,
        MOON_GEOMETRY,
        SKY_GEOMETRY,
        MERCURY_GEOMETRY,
        VENUS_GEOMETRY,
        MARS_GEOMETRY,
        JUPITER_GEOMETRY,
        SATURN_GEOMETRY,
        URANUS_GEOMETRY,
        NEPTUNE_GEOMETRY,
        NUM_GEOMETRIES
    };

    std::unique_ptr<TurnTableCamera> mTurnTableCamera{};
    glm::dvec2 mPreviousCursorPosition {};
    bool mCursorPositionIsSetOnce = false;

    bool mShowNightTexture = false; // Show earth's night lights

    glm::mat4 mProjectionMatrix{};

    float mFovY = 120.0f;
    float mZNear = 0.01f;
    float mZFar = 100.0f;
    float mZoomSpeed = 20.0f;
    float mRotationSpeed = 0.25f;

    // Animation state
    bool mIsAnimating = true;
    float mAnimationSpeed = 1.0f;
    float mLastAnimationTime = 0.0f;

    // Cloud state
    bool mShowClouds = false;
    float mCloudRotationAngle = 0.0f;
    float mCloudRotationSpeed = 0.1f;
    float mCloudOpacity = 0.3f; // How see-through clouds are

    // Celestial body animation parameters
    float mSunRotationAngle = 0.0f;
    float mEarthOrbitAngle = 0.0f;
    float mEarthRotationAngle = 0.0f;
    float mMoonOrbitAngle = 0.0f;
    float mMoonRotationAngle = 0.0f;

    // Orbital parameters
    const float mEarthOrbitRadius = 5.0f;
    const float mMoonOrbitRadius = 1.5f;
    const float mEarthAxialTilt = 23.5f;
    const float mMoonAxialTilt = 6.68f;
    const float mEarthOrbitInclination = 5.0f; // Orbit tilt, exaggerated for visibility

    // Camera focus options
    enum class CameraFocus
    {
        SUN,
        EARTH,
        MOON,
        MERCURY,
        VENUS,
        MARS,
        JUPITER,
        SATURN,
        URANUS,
        NEPTUNE
    };
    CameraFocus mCurrentFocus = CameraFocus::SUN;

    std::unique_ptr<GPU_Geometry> mSaturnRingGeometry;
    int mSaturnRingIndexCount;

    // Add planet animation parameters
    struct PlanetData
    {
        float orbitAngle = 0.0f;
        float rotationAngle = 0.0f;
        float orbitRadius = 0.0f;
        float size = 0.0f;
        float orbitSpeed = 0.0f;
        float rotationSpeed = 0.0f;
        float axialTilt = 0.0f;
        float orbitInclination = 0.0f;
        float eccentricity = 0.0f;
    };

    std::vector<PlanetData> mPlanets;
    std::vector<std::vector<PlanetData>> mMoons; // Moons for each planet


    // Elliptic orbit parameters
    float mEarthOrbitEccentricity = 0.0f;
    float mMoonOrbitEccentricity = 0.0f; 
    float mEarthOrbitSemiMajorAxis = 5.0f;
    float mMoonOrbitSemiMajorAxis = 1.5f;
};

