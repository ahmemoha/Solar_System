#include "SolarSystem.hpp"

#include <filesystem>

#include "GLDebug.h"
#include "Log.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "ShapeGenerator.hpp"

// Step 1: Create a sphere with positions, indices, and uv values
// Step 2: Create the solar system with sun, earth and moon
// Step 3: Add cube map texture for background and

//======================================================================================================================

SolarSystem::SolarSystem()
{
    mPath = AssetPath::Instance();
    mTime = Time::Instance();

    glfwWindowHint(GLFW_SAMPLES, 32);
    mWindow = std::make_unique<Window>(800, 800, "Solar system");

    // Standard ImGui/GLFW middleware
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(mWindow->getGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    glEnable(GL_MULTISAMPLE);
    int samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    Log::info("MSAA Samples: {0}", samples);

    // GLDebug::enable(); // ON Submission you may comments this out to avoid unnecessary prints to the console

    mInputManager = std::make_shared<InputManager>(
        [this](int const width, int const height) -> void { OnResize(width, height); },
        [this](double const xOffset, double const yOffset) -> void { OnMouseWheelChange(xOffset, yOffset); });

    mWindow->setCallbacks(mInputManager);

    // Setup sphere geometries
    mUnitSphereGeometry.resize(NUM_GEOMETRIES);
    mUnitSphereIndexCount.resize(NUM_GEOMETRIES);

    PrepareUnitSphereGeometry(); // make the spheres


    // Initialize textures vector
    mTextures.resize(NUM_TEXTURES);
    mTextures[SUN_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_sun.jpg"), GL_LINEAR);
    mTextures[EARTH_DAY_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_earth_daymap.jpg"), GL_LINEAR);
    mTextures[EARTH_NIGHT_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_earth_nightmap.jpg"), GL_LINEAR);
    mTextures[EARTH_CLOUDS_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_earth_clouds.jpg"), GL_LINEAR);
    mTextures[MOON_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_moon.jpg"), GL_LINEAR);
    mTextures[SKY_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_stars_milky_way.jpg"), GL_LINEAR);
    mTextures[MERCURY_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/8k_mercury.jpg"), GL_LINEAR);
    mTextures[VENUS_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/8k_venus_surface.jpg"), GL_LINEAR);
    mTextures[MARS_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/8k_mars.jpg"), GL_LINEAR);
    mTextures[JUPITER_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/8k_jupiter.jpg"), GL_LINEAR);
    mTextures[SATURN_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures82k_saturn.jpg"), GL_LINEAR);
    mTextures[SATURN_RING_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_saturn_ring_alpha.png"), GL_LINEAR);
    mTextures[URANUS_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_uranus.jpg"), GL_LINEAR);
    mTextures[NEPTUNE_TEXTURE] = std::make_unique<Texture>(mPath->Get("textures/2k_neptune.jpg"), GL_LINEAR);

    // Initialize planet data (relative sizes and distances scaled for visibility)
    mPlanets.resize(8); // Mercury (0) to Neptune (7)

    // Mercury
    mPlanets[0] = {0.0f, 0.0f, 3.0f, 0.4f, 0.5f, 0.1f, 2.0f, 7.0f, 0.2f};

    // Venus
    mPlanets[1] = {0.0f, 0.0f, 4.0f, 0.6f, 0.4f, 0.01f, 177.4f, 3.4f, 0.01f};

    // Earth (already exists, but we'll add it here for completeness)
    mPlanets[2] = {mEarthOrbitAngle, mEarthRotationAngle,    mEarthOrbitRadius,      0.5f, 0.2f, 2.0f,
                   mEarthAxialTilt,  mEarthOrbitInclination, mEarthOrbitEccentricity};

    // Mars
    mPlanets[3] = {0.0f, 0.0f, 6.0f, 0.4f, 0.15f, 1.0f, 25.2f, 1.9f, 0.09f};

    // Jupiter
    mPlanets[4] = {0.0f, 0.0f, 8.0f, 1.2f, 0.05f, 1.5f, 3.1f, 1.3f, 0.05f};

    // Saturn
    mPlanets[5] = {0.0f, 0.0f, 10.0f, 1.0f, 0.03f, 1.2f, 26.7f, 2.5f, 0.06f};

    // Uranus
    mPlanets[6] = {0.0f, 0.0f, 12.0f, 0.8f, 0.02f, 0.8f, 97.8f, 0.8f, 0.05f};

    // Neptune
    mPlanets[7] = {0.0f, 0.0f, 14.0f, 0.7f, 0.01f, 0.7f, 28.3f, 1.8f, 0.01f};

    // Initialize moons (just doing Earth's moon and Jupiter's 4 largest as example)
    mMoons.resize(8); // Moons for each planet

    // Earth's moon (already exists)
    mMoons[2].push_back({mMoonOrbitAngle, mMoonRotationAngle, mMoonOrbitRadius, 0.2f, 0.5f, 0.1f, mMoonAxialTilt, 0.0f,
                         mMoonOrbitEccentricity});

    // Jupiter's moons (Galilean moons)
    mMoons[4].push_back({0.0f, 0.0f, 1.5f, 0.15f, 0.8f, 0.05f, 0.0f, 0.0f, 0.0f}); // Io
    mMoons[4].push_back({0.0f, 0.0f, 2.0f, 0.2f, 0.6f, 0.05f, 0.0f, 0.0f, 0.0f}); // Europa
    mMoons[4].push_back({0.0f, 0.0f, 2.5f, 0.25f, 0.4f, 0.05f, 0.0f, 0.0f, 0.0f}); // Ganymede
    mMoons[4].push_back({0.0f, 0.0f, 3.0f, 0.2f, 0.3f, 0.05f, 0.0f, 0.0f, 0.0f}); // Callisto

    mBasicShader = std::make_unique<ShaderProgram>(mPath->Get("shaders/test.vert"), mPath->Get("shaders/test.frag"));

    // Set camera
    mTurnTableCamera = std::make_unique<TurnTableCamera>();
    mTurnTableCamera->SetTargetBody(TurnTableCamera::TargetBody::SUN); // Starts at the sun
}

//======================================================================================================================

SolarSystem::~SolarSystem()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

//======================================================================================================================

void SolarSystem::Run()
{
    while (mWindow->shouldClose() == false)
    {
        glfwPollEvents(); // Propagate events to the callback class

        mTime->Update();
        Update(mTime->DeltaTimeSec());

        glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
        // https://www.viewsonic.com/library/creative-work/srgb-vs-adobe-rgb-which-one-to-use/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear render screen (all zero) and depth (all max depth)
        glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());

        Render();

        // glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui (if used)

        // Starting the new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        UI();

        ImGui::Render(); 
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

        mWindow->swapBuffers(); 
    }
}

//======================================================================================================================

void SolarSystem::Update(float const deltaTime)
{
    auto const cursorPosition = mInputManager->CursorPosition();

    if (mCursorPositionIsSetOnce == true)
    {
        if (mInputManager->IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) == true)
        {
            auto const deltaPosition = cursorPosition - mPreviousCursorPosition;
            mTurnTableCamera->ChangeTheta(-static_cast<float>(deltaPosition.x) * mRotationSpeed * deltaTime);
            mTurnTableCamera->ChangePhi(-static_cast<float>(deltaPosition.y) * mRotationSpeed * deltaTime);
        }
    }

    if (mIsAnimating) // if animation is playing
    {
        // Clouds are moving
        mCloudRotationAngle += deltaTime * mCloudRotationSpeed * mAnimationSpeed;

        // Update animation angles using kepler's laws for the orbits 
        float timeDelta = deltaTime * mAnimationSpeed;

        // Earth moves faster when closer to sun
        float earthAngularVelocity = 0.2f * pow(1.0f + mEarthOrbitEccentricity * cos(mEarthOrbitAngle), 2) / pow(1.0f - mEarthOrbitEccentricity * mEarthOrbitEccentricity, 1.5f);
        mEarthOrbitAngle += timeDelta * earthAngularVelocity;
        mEarthRotationAngle += timeDelta * 2.0f;

        // Moon does the same thing around earth
        float moonAngularVelocity = 0.5f * pow(1.0f + mMoonOrbitEccentricity * cos(mMoonOrbitAngle), 2) / pow(1.0f - mMoonOrbitEccentricity * mMoonOrbitEccentricity, 1.5f);
        mMoonOrbitAngle += timeDelta * moonAngularVelocity;

        // Sun rotation
        mSunRotationAngle += timeDelta * 0.5f;
    }


    mCursorPositionIsSetOnce = true;
    mPreviousCursorPosition = cursorPosition;

    if (mIsAnimating)
    {
        float timeDelta = deltaTime * mAnimationSpeed;

        // Update all planets
        for (auto &planet : mPlanets)
        {
            // Update orbit angle based on Kepler's laws
            float angularVelocity = planet.orbitSpeed * pow(1.0f + planet.eccentricity * cos(planet.orbitAngle), 2) /
                pow(1.0f - planet.eccentricity * planet.eccentricity, 1.5f);
            planet.orbitAngle += timeDelta * angularVelocity;

            // Update rotation
            planet.rotationAngle += timeDelta * planet.rotationSpeed;
        }

        // Update all moons
        for (auto &moons : mMoons)
        {
            for (auto &moon : moons)
            {
                float angularVelocity = moon.orbitSpeed * pow(1.0f + moon.eccentricity * cos(moon.orbitAngle), 2) /
                    pow(1.0f - moon.eccentricity * moon.eccentricity, 1.5f);
                moon.orbitAngle += timeDelta * angularVelocity;
                moon.rotationAngle += timeDelta * moon.rotationSpeed;
            }
        }

        // Update clouds (existing)
        mCloudRotationAngle += deltaTime * mCloudRotationSpeed * mAnimationSpeed;
    }
}

//======================================================================================================================

void SolarSystem::Render()
{
    glEnable(GL_DEPTH_TEST); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate current animation time=
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = 0.0f;

    // Only update animation if it's playing and we have a previous time
    if (mIsAnimating && mLastAnimationTime > 0.0f)
    {
        deltaTime = (currentTime - mLastAnimationTime) * mAnimationSpeed;

        // Update animation angles
        mSunRotationAngle += deltaTime * 0.5f; // Sun rotates 
        mEarthOrbitAngle += deltaTime * 0.2f; // Earth orbits sun
        mEarthRotationAngle += deltaTime * 2.0f; // Earth rotates
        mMoonOrbitAngle += deltaTime * 0.5f; // Moon orbits earth
        mMoonRotationAngle += deltaTime * 0.1f; // Moon rotates
    }
    mLastAnimationTime = currentTime; // remember this time for next frame

    mBasicShader->use();

    // Calculate aspect ratio: width/height
    float aspectRatio = static_cast<float>(mWindow->getWidth()) / static_cast<float>(mWindow->getHeight());

    // Create perspective projection matrix
    auto projection = glm::perspective(glm::radians(mFovY), aspectRatio, mZNear, mZFar);
    glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "projection"), 1, GL_FALSE, &projection[0][0]);

    // Get camera view matrix from turntable camera
    auto view = mTurnTableCamera->ViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "view"), 1, GL_FALSE, &view[0][0]);

    // Get camera position for specular lighting calculations
    glm::vec3 cameraPos = mTurnTableCamera->GetPosition();
    glUniform3fv(glGetUniformLocation(*mBasicShader, "viewPos"), 1, &cameraPos[0]);

    // light position is at the origin, which is center of the sun
    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

    // Light properties:
    // - Ambient: base lighting level
    // - Diffuse: directional lighting, depends on surface angle to light
    // - Specular: shiny highlights
    glm::vec3 lightAmbient(0.3f, 0.3f, 0.3f); // Low ambient light
    glm::vec3 lightDiffuse(0.8f, 0.8f, 0.8f); // Bright directional light
    glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f); // Strong highlights

    // Send light properties to shader
    glUniform3fv(glGetUniformLocation(*mBasicShader, "light.position"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(*mBasicShader, "light.ambient"), 1, &lightAmbient[0]);
    glUniform3fv(glGetUniformLocation(*mBasicShader, "light.diffuse"), 1, &lightDiffuse[0]);
    glUniform3fv(glGetUniformLocation(*mBasicShader, "light.specular"), 1, &lightSpecular[0]);

    // Draw the sky sphere
    {
        // no transformation as it's the sky sphere
        auto model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "model"), 1, GL_FALSE, &model[0][0]);

        // Tell shader this is the sun, so no lighting calculations)
        glUniform1i(glGetUniformLocation(*mBasicShader, "isSun"), GL_TRUE);

        // Bind sky texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        mTextures[SKY_TEXTURE]->bind();
        glUniform1i(glGetUniformLocation(*mBasicShader, "material.diffuse"), 0);

        // draw the sky sphere
        mUnitSphereGeometry[SKY_GEOMETRY]->bind();
        glDrawElements(GL_TRIANGLES, mUnitSphereIndexCount[SKY_GEOMETRY], GL_UNSIGNED_INT, nullptr);
    }

    // Draw the sun
    {
        // Sun only rotates on its axis
        auto model = glm::rotate(glm::mat4(1.0f), mSunRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "model"), 1, GL_FALSE, &model[0][0]);

        // Tell shader this is the sun, it emits light, doesn't need lighting
        glUniform1i(glGetUniformLocation(*mBasicShader, "isSun"), GL_TRUE);

        // bind sun texture
        glActiveTexture(GL_TEXTURE0);
        mTextures[SUN_TEXTURE]->bind();
        glUniform1i(glGetUniformLocation(*mBasicShader, "material.diffuse"), 0);

        // draw the sun sphere
        mUnitSphereGeometry[SUN_GEOMETRY]->bind();
        glDrawElements(GL_TRIANGLES, mUnitSphereIndexCount[SUN_GEOMETRY], GL_UNSIGNED_INT, nullptr);
    }

    // Draw Earth
    {
        // is5 units from sun along x axis
        // Earth orbits sun and rotates on axis
        // Orbital transformation hierarchy
        auto earthOrbitModel = glm::rotate(glm::mat4(1.0f), glm::radians(mEarthOrbitInclination), glm::vec3(0.0f, 0.0f, 1.0f)); // tilt of orbit plane
        float earthDistance = mEarthOrbitSemiMajorAxis * (1.0f - mEarthOrbitEccentricity * mEarthOrbitEccentricity) / (1.0f + mEarthOrbitEccentricity * cos(mEarthOrbitAngle)); // calculate current distance in elliptical orbit
        earthOrbitModel = glm::rotate(earthOrbitModel, mEarthOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // rotate around the sun based on orbit angle
        earthOrbitModel = glm::translate(earthOrbitModel, glm::vec3(earthDistance, 0.0f, 0.0f)); // move to the current orbital position

        // Earth's own rotation, axial tilt + spin
        auto earthRotation = glm::rotate(glm::mat4(1.0f), glm::radians(mEarthAxialTilt), glm::vec3(0.0f, 0.0f, 1.0f));
        earthRotation = glm::rotate(earthRotation, mEarthRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); 

        // Combine orbit and rotation transformations
        auto earthModel = earthOrbitModel * earthRotation;

        // earth's model matrix to shader
        glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "model"), 1, GL_FALSE, &earthModel[0][0]);
        glUniform1i(glGetUniformLocation(*mBasicShader, "isSun"), GL_FALSE);
        glUniform1i(glGetUniformLocation(*mBasicShader, "isEarth"), GL_TRUE); 
        glUniform1i(glGetUniformLocation(*mBasicShader, "showNightTexture"), mShowNightTexture);
        glUniform1i(glGetUniformLocation(*mBasicShader, "showClouds"), mShowClouds);

        // bind earth texture
        glActiveTexture(GL_TEXTURE0);
        mTextures[EARTH_DAY_TEXTURE]->bind();
        glUniform1i(glGetUniformLocation(*mBasicShader, "material.diffuse"), 0);

        // bind night texture even when not showing night lights
        glActiveTexture(GL_TEXTURE1);
        mTextures[EARTH_NIGHT_TEXTURE]->bind();
        glUniform1i(glGetUniformLocation(*mBasicShader, "material.night"), 1);

        // Set earth material properties
        glm::vec3 earthSpecular(1.0f, 1.0f, 1.0f); // Shiny like oceans
        float earthShininess = 64.0f; // how sharp the highlights are
        glUniform3fv(glGetUniformLocation(*mBasicShader, "material.specular"), 1, &earthSpecular[0]);
        glUniform1f(glGetUniformLocation(*mBasicShader, "material.shininess"), earthShininess);

        // Draw the earth sphere
        mUnitSphereGeometry[EARTH_GEOMETRY]->bind();
        glDrawElements(GL_TRIANGLES, mUnitSphereIndexCount[EARTH_GEOMETRY], GL_UNSIGNED_INT, nullptr);

        // Render clouds if enabled
        if (mShowClouds)
        {
            glEnable(GL_BLEND); // Enable transparency
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard alpha blending

            // Cloud model uses earth's position but adds independent rotation
            auto cloudModel = earthOrbitModel * earthRotation; // Start with earth's transform
            cloudModel = glm::rotate(cloudModel, mCloudRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            cloudModel = glm::scale(cloudModel, glm::vec3(1.005f)); // Slightly larger than earth

            // send cloud's model to shader
            glUniform1f(glGetUniformLocation(*mBasicShader, "cloudRotationAngle"), mCloudRotationAngle);
            glUniform1i(glGetUniformLocation(*mBasicShader, "showClouds"), GL_TRUE);

            // bind cloud texture
            glActiveTexture(GL_TEXTURE2);
            mTextures[EARTH_CLOUDS_TEXTURE]->bind();
            glUniform1i(glGetUniformLocation(*mBasicShader, "material.clouds"), 2);

            // Draw clouds sphere using same geometry as earth
            glDrawElements(GL_TRIANGLES, mUnitSphereIndexCount[EARTH_GEOMETRY], GL_UNSIGNED_INT, nullptr);

            glDisable(GL_BLEND); // Turn off blending
        }
    }

    // Draw Moon
    {
        // Start with Earth's orbital transform
        auto model = glm::rotate(glm::mat4(1.0f), glm::radians(mEarthOrbitInclination), glm::vec3(0.0f, 0.0f, 1.0f));
        float earthDistance = mEarthOrbitSemiMajorAxis * (1.0f - mEarthOrbitEccentricity * mEarthOrbitEccentricity) / (1.0f + mEarthOrbitEccentricity * cos(mEarthOrbitAngle)); // calculate Earth's current position
        model = glm::rotate(model, mEarthOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // apply earth's orbital rotation and position
        model = glm::translate(model, glm::vec3(earthDistance, 0.0f, 0.0f));

        // Moon's orbit around Earth
        float moonDistance = mMoonOrbitSemiMajorAxis * (1.0f - mMoonOrbitEccentricity * mMoonOrbitEccentricity) / (1.0f + mMoonOrbitEccentricity * cos(mMoonOrbitAngle));
        model = glm::rotate(model, mMoonOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // apply moon's orbital rotation and position
        model = glm::translate(model, glm::vec3(moonDistance, 0.0f, 0.0f));

        // Moon's axial tilt and rotation
        model = glm::rotate(model, glm::radians(mMoonAxialTilt), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, mMoonRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(*mBasicShader, "model"), 1, GL_FALSE, &model[0][0]);
        glUniform1i(glGetUniformLocation(*mBasicShader, "isSun"), GL_FALSE);
        glUniform1i(glGetUniformLocation(*mBasicShader, "isEarth"), GL_FALSE);
        glUniform1i(glGetUniformLocation(*mBasicShader, "showNightTexture"), GL_FALSE);
        glUniform1i(glGetUniformLocation(*mBasicShader, "showClouds"), GL_FALSE);

        // bind moon texture, bind moon texture
        glActiveTexture(GL_TEXTURE0);
        mTextures[MOON_TEXTURE]->bind();
        glUniform1i(glGetUniformLocation(*mBasicShader, "material.diffuse"), 0);

        glActiveTexture(GL_TEXTURE1);
        mTextures[MOON_TEXTURE]->bind(); // Use same texture for night since moon has no night texture
        glUniform1i(glGetUniformLocation(*mBasicShader, "material.night"), 1);

        // Moon material properties, less shiny than earth
        glm::vec3 moonSpecular(0.3f, 0.3f, 0.3f);
        float moonShininess = 8.0f;
        glUniform3fv(glGetUniformLocation(*mBasicShader, "material.specular"), 1, &moonSpecular[0]);
        glUniform1f(glGetUniformLocation(*mBasicShader, "material.shininess"), moonShininess);

        // Draw the moon sphere
        mUnitSphereGeometry[MOON_GEOMETRY]->bind();
        glDrawElements(GL_TRIANGLES, mUnitSphereIndexCount[MOON_GEOMETRY], GL_UNSIGNED_INT, nullptr);
    }

    // Update camera target if following a sphere
    if (mTurnTableCamera->GetTargetBody() != TurnTableCamera::TargetBody::NONE && mIsAnimating)
    {
        glm::vec3 targetPos;

        switch (mTurnTableCamera->GetTargetBody())
        {
        case TurnTableCamera::TargetBody::SUN:
            targetPos = glm::vec3(0.0f); // Sun is always at origin
            break;

        case TurnTableCamera::TargetBody::EARTH:
            {
                // Calculate Earth's current position
                auto earthOrbit = glm::rotate(glm::mat4(1.0f), glm::radians(mEarthOrbitInclination), glm::vec3(0.0f, 0.0f, 1.0f));
                float earthDistance = mEarthOrbitSemiMajorAxis * (1.0f - mEarthOrbitEccentricity * mEarthOrbitEccentricity) / (1.0f + mEarthOrbitEccentricity * cos(mEarthOrbitAngle));
                earthOrbit = glm::rotate(earthOrbit, mEarthOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                earthOrbit = glm::translate(earthOrbit, glm::vec3(earthDistance, 0.0f, 0.0f));
                targetPos = glm::vec3(earthOrbit * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                break;
            }

        case TurnTableCamera::TargetBody::MOON:
            {
                // Calculate Moon's current position 
                auto earthOrbit = glm::rotate(glm::mat4(1.0f), glm::radians(mEarthOrbitInclination), glm::vec3(0.0f, 0.0f, 1.0f));
                float earthDistance = mEarthOrbitSemiMajorAxis * (1.0f - mEarthOrbitEccentricity * mEarthOrbitEccentricity) / (1.0f + mEarthOrbitEccentricity * cos(mEarthOrbitAngle));
                earthOrbit = glm::rotate(earthOrbit, mEarthOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                earthOrbit = glm::translate(earthOrbit, glm::vec3(earthDistance, 0.0f, 0.0f));

                float moonDistance = mMoonOrbitSemiMajorAxis * (1.0f - mMoonOrbitEccentricity * mMoonOrbitEccentricity) / (1.0f + mMoonOrbitEccentricity * cos(mMoonOrbitAngle));
                auto moonOrbit = glm::rotate(earthOrbit, mMoonOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                moonOrbit = glm::translate(moonOrbit, glm::vec3(moonDistance, 0.0f, 0.0f));
                targetPos = glm::vec3(moonOrbit * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                break;
            }
        }
        // Update camera to follow the target
        mTurnTableCamera->UpdateTargetPosition(targetPos);
    }
}

//======================================================================================================================

void SolarSystem::UI()
{
    ImGui::Begin("Solar System Controls");

    // Animation controls
    if (ImGui::Button(mIsAnimating ? "Pause" : "Play")) // Play/Pause button to toggle animation state
    {
        mIsAnimating = !mIsAnimating; // Flip the animation state
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset")) // Reset button
    {   // Reset all rotation angles to zero
        mSunRotationAngle = 0.0f;
        mEarthOrbitAngle = 0.0f;
        mEarthRotationAngle = 0.0f;
        mMoonOrbitAngle = 0.0f;
        mMoonRotationAngle = 0.0f;
    }

    ImGui::SliderFloat("Animation Speed", &mAnimationSpeed, 0.1f, 5.0f);
    ImGui::Text("FPS: %.1f", 1.0f / mTime->DeltaTimeSec());
    ImGui::Separator();

    ImGui::Checkbox("Show Night Lights", &mShowNightTexture); // Toggle for showing city lights on Earth's night side
    ImGui::Checkbox("Show Clouds", &mShowClouds); // Toggle for showing cloud layer
    if (mShowClouds)
    {
        ImGui::SliderFloat("Cloud Speed", &mCloudRotationSpeed, 0.0f, 1.0f); // Only show cloud speed control if clouds are visible
    }

    ImGui::Separator();
    ImGui::Text("Camera Focus:"); // choose which celestial body to focus on
    if (ImGui::RadioButton("Sun", mTurnTableCamera->GetTargetBody() == TurnTableCamera::TargetBody::SUN))
    {
        mTurnTableCamera->SetTargetBody(TurnTableCamera::TargetBody::SUN);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Earth", mTurnTableCamera->GetTargetBody() == TurnTableCamera::TargetBody::EARTH))
    {
        mTurnTableCamera->SetTargetBody(TurnTableCamera::TargetBody::EARTH);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Moon", mTurnTableCamera->GetTargetBody() == TurnTableCamera::TargetBody::MOON))
    {
        mTurnTableCamera->SetTargetBody(TurnTableCamera::TargetBody::MOON);
    }
    ImGui::Separator();
    ImGui::Text("Orbit Settings:"); // Sliders to control how elliptical the orbits are
    ImGui::SliderFloat("Earth Orbit Eccentricity", &mEarthOrbitEccentricity, 0.0f, 0.5f);  // Earth's orbit eccentricity (0 = perfect circle, 0.5 = noticeably oval)
    ImGui::SliderFloat("Moon Orbit Eccentricity", &mMoonOrbitEccentricity, 0.0f, 0.5f); // Moon's orbit eccentricity
    ImGui::End();
}

//======================================================================================================================

void SolarSystem::PrepareUnitSphereGeometry()
{
    // Creates all the sphere geometries we need for our solar system
    auto sunSphere = ShapeGenerator::Sphere(1.5f, 64, 64);
    auto earthSphere = ShapeGenerator::Sphere(0.5f, 64, 64);
    auto moonSphere = ShapeGenerator::Sphere(0.2f, 32, 32);
    auto skySphere = ShapeGenerator::Sphere(10.0f, 64, 64);
    auto saturnRing = ShapeGenerator::Ring(1.5f, 2.5f, 64);

    mSaturnRingGeometry = std::make_unique<GPU_Geometry>();
    mSaturnRingGeometry->Update(saturnRing);
    mSaturnRingIndexCount = static_cast<int>(saturnRing.indices.size());

    // Upload each sphere to the GPU:

    // Sun geometry
    mUnitSphereGeometry[SUN_GEOMETRY] = std::make_unique<GPU_Geometry>();
    mUnitSphereGeometry[SUN_GEOMETRY]->Update(sunSphere);
    mUnitSphereIndexCount[SUN_GEOMETRY] = static_cast<int>(sunSphere.indices.size());

    // Earth geometry
    mUnitSphereGeometry[EARTH_GEOMETRY] = std::make_unique<GPU_Geometry>();
    mUnitSphereGeometry[EARTH_GEOMETRY]->Update(earthSphere);
    mUnitSphereIndexCount[EARTH_GEOMETRY] = static_cast<int>(earthSphere.indices.size());

    // Moon geometry
    mUnitSphereGeometry[MOON_GEOMETRY] = std::make_unique<GPU_Geometry>();
    mUnitSphereGeometry[MOON_GEOMETRY]->Update(moonSphere);
    mUnitSphereIndexCount[MOON_GEOMETRY] = static_cast<int>(moonSphere.indices.size());

    // Sky sphere geometry
    mUnitSphereGeometry[SKY_GEOMETRY] = std::make_unique<GPU_Geometry>();
    mUnitSphereGeometry[SKY_GEOMETRY]->Update(skySphere);
    mUnitSphereIndexCount[SKY_GEOMETRY] = static_cast<int>(skySphere.indices.size());
}

//======================================================================================================================

void SolarSystem::OnResize(int width, int height)
{
    // TODO
}

//======================================================================================================================

void SolarSystem::OnMouseWheelChange(double const xOffset, double const yOffset) const
{
    mTurnTableCamera->ChangeRadius(-static_cast<float>(yOffset) * mZoomSpeed * mTime->DeltaTimeSec());
}

//======================================================================================================================
