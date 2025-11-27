# 🪐CPSC 453 Assignment 4: Virtual Orrery ###

![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green?logo=opengl) ![Language](https://img.shields.io/badge/Language-C%2B%2B-blue?logo=c%2B%2B) ![License](https://img.shields.io/badge/Course-CPSC453-orange)

This project is a 3D solar system simulation developed as part of CPSC 453 at the University of Calgary. The application renders a realistic model of the Sun-Earth-Moon system with accurate orbital mechanics, lighting, and textures.

---

## ✨Features ##
### 🌌 Celestial Simulation
* **Realistic Texturing:** High-resolution textures for the Sun, Earth, and Moon set against a starry skybox.
* **Scale & Distance:** Proper relative sizing and spatial relationships.
* **Orbital Mechanics:**
    * Hierarchical transformation system (Scene Graph).
    * Independent rotation periods and axial tilts.
    * **[Bonus]** Elliptical orbits featuring accurate Keplerian motion.

### 🎨 Rendering Engine
* **Phong Shading:** Implements diffuse, specular, and ambient lighting components.
* **Lighting:** Point light source originating from the Sun's center.
* **[Bonus]** **Night Lights:** City lights appear dynamically on Earth's dark side.
* **[Bonus]** **Atmosphere:** Animated cloud layers with independent rotation.

### 🎥 Camera & Interface
* **Turntable Camera:** Spherical coordinate system with smooth zoom and orbit.
* **Focus System:** Smooth transitions to focus on the Sun, Earth, or Moon.
* **Interactive UI:** Full control over time scale, rendering features, and orbital eccentricity.
---

## 🎮 Controls

| Input | Action |
| :--- | :--- |
| **Right Mouse Drag** | Orbit camera around the current focus point |
| **Mouse Wheel** | Zoom In / Out |
| **Play / Pause** | Toggle simulation animation |
| **Reset** | Return simulation to initial state |
| **Speed Slider** | Adjust simulation time scale |
| **Toggle: Night Lights** | Enable/Disable city lights on Earth's shadow side |
| **Toggle: Clouds** | Enable/Disable Earth's atmospheric layer |
| **Focus Select** | Smoothly pan camera to Sun, Earth, or Moon |
| **Eccentricity** | Adjust the ellipticity of orbits (Keplerian effect) |

---

## 🏗️ Implementation Architecture

### 1. The SolarSystem Class
Acts as the central engine, managing the lifecycle of all celestial bodies. It handles the main loop, coordinates the scene graph, and manages the global time state for frame-rate independent animation.

### 2. Turntable Camera
A spherical coordinate camera system designed for orbital observation. It features a "Follow" mode that locks onto moving celestial bodies and uses smooth interpolation to transition between targets without snapping.

### 3. Advanced Shader System (GLSL)
* **Sun:** Uses emissive shading to act as a light source.
* **Planets:** Uses standard Phong lighting.
* **Blending:** Handles complex texture mixing for the Day/Night cycle and transparency for cloud layers.

### 4. Geometry & Math
Utilization of UV-mapped spheres with precise normal calculation. All movement is driven by GLM vector and matrix mathematics.

---

## 🛠️ Dependencies & Compilation

This project relies on the standard OpenGL stack:
* **GLFW:** Windowing and Input
* **GLAD:** OpenGL Loading Library
* **GLM:** Mathematics (Vectors/Matrices)
* **stb_image:** Texture loading

### 🟦 Visual Studio (Windows)
1.  Open the project in **Visual Studio 2022**.
2.  Ensure dependencies (GLFW, GLAD, GLM) are properly configured.
3.  Select **x64** or **x86** build configuration.
4.  **Build & Run**.

### 🐧 CMake (Linux / macOS)
```bash
# 1. Create build directory
mkdir build && cd build

# 2. Configure project
cmake ..

# 3. Compile
cmake --build .

# 4. Run application
./cpsc453-solarsystem

### 🤖 AI Development Log ##

This project utilized AI assistance for debugging complex linear algebra and optimizing rendering techniques. Below is a log of key challenges solved.

When implementing Phong shading, my specular highlights were behaving incorrectly when the Earth and Moonrotated. Instead of the highlights moving smoothly with the Sun, they appeared misaligned and inconsistent. With the help of ChatGPT, this solution ensured realistic specular reflections while maintaining smooth rendering. The debugging process also deepened my understanding of normal transformations, which later helped when implementing advanced features like normal mapping.

**Prompt:** My Phong specular highlights are wrong when objects rotate. Is this the correct normal matrix calculation?

``normal = mat3(transpose(inverse(model))) * inNormal;``

**AI Response:** Your calculation is correct but expensive. For better performance:
1. Calculate on CPU and pass as uniform:
   `glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));`
2. In shader:
   `normal = normalize(normalMatrix * inNormal);`
Also verify your normals are normalized after transformation.

---


I used ChatGPT for advice to implement independent axial rotation for celestial bodies while maintaining correct parent-child orbital relationships, but was getting incorrect rotations affecting child objects. It helped in making Earth's orbital transformation. So it's organizing the objects so that the Earth’s position can be described relative to the Sun, and the Moon’s relative to the Earth" while maintaining visible axial tilts.

**Prompt:** For a solar system simulation in OpenGL, how can I make a planet's axial rotation (day/night cycle) independent from its orbital rotation around the sun? The axial tilt should be visible but shouldn't affect the position of orbiting moons. Can you explain the matrix math without giving complete code?

**AI Response:** You need to separate the transformation hierarchy into three distinct components:

1. Orbital Transform: Parent-space rotation and translation (affects child objects)
* First apply orbital plane tilt (inclination)
* Then orbital position rotation
* Finally orbital radius translation

2. Axial Transform: Local rotation only (doesn't affect children)
* Apply axial tilt rotation (e.g., Earth's 23.5° tilt)
* Then daily rotation

Implementation order:
**FinalMatrix = OrbitRotation * OrbitTranslation * AxialRotation**

Key points:
- Do all orbital transforms BEFORE axial rotations
- Use separate rotation matrices for orbital vs. axial rotations
- Normalize all rotation axes to prevent skewing

---
When implementing Phong shading, my Moon was being incorrectly illuminated through Earth's transform hierarchy. This caused the Moon's dark side to face the Sun during certain orbits. ChatGPT helped me understand world space vs. model space lighting calculations.

**Prompt:** I'm implementing a Phong-shaded solar system where the Moon's lighting should come directly from the Sun's position, unaffected by Earth's transform. How should light direction be properly calculated in a matrix hierarchy?

**AI Response:** Light direction must be calculated in world space:
1. Transform light position (sun) and fragment position to world space
2. Calculate direction in world space, not model space
3. For Moon lighting:
   * Use sun's world position (typically origin)
   * Don't apply Earth's transform to light direction
4. In shader:
   `vec3 lightDir = normalize(sunWorldPos - fragWorldPos);`
---
I asked ChatGPT in regard of my celestial animations is maintained consistent speeds across different hardware configurations. While I understood the concept of delta time, I wanted to verify best practices for astronomical simulations where precise timing is crucial. The AI helped clarify how to implement truly framerate-independent movement while avoiding common pitfalls like time accumulation errors.

**Prompt:** How to implement solar system animations in OpenGL that maintain constant speed across different framerates? The Earth should complete one orbit every 10 seconds regardless of FPS

**AI Response:** For framerate-independent animation:
1. Track system time using glfwGetTime()
2. Calculate deltaTime = currentTime - lastFrameTime
3. Scale movement by deltaTime:
   orbitAngle += orbitSpeed * deltaTime
4. For smooth transitions:
   - Use floating-point angles (not integers)
   - Apply modulo 2π to prevent overflow
Critical considerations:
- Never tie animation to render calls
- Use system time, not frame counters
- Test with extreme framerates (10 FPS vs 144 FPS)

---
For the elliptical orbit bonus feature, I required confirmation on implementing Kepler's laws correctly in real-time. I asked ChatGPT to help guide my gap between textbook orbital mechanics and practical OpenGL implementation, particularly for the variable angular velocity calculations.

**Prompt:** Physics-accurate way to calculate position and speed in elliptical orbits for a solar system demo?


**AI Response:**
Keplerian orbital mechanics:
1. Calculate current distance:
   `r = a(1-e²)/(1+e·cos(θ))`
2. Angular velocity:
   `ω = √(G(M+m)/a³) · (1+e·cos(θ))²/(1-e²)^(3/2)`
3. Implementation tips:
   - Precompute constant terms
   - Use polar coordinates
   - Convert to Cartesian only for rendering
   - Exaggerate `e` for visible effect

---
I asked AI about camera interpolation techniques when implementing the bonus feature for celestial body tracking. This helped in making the implementation for the target body and doing that bonus. I did provide code but used it as a reference point

**Prompt:** How to smoothly interpolate a turntable camera between focus points in a 3D solar system simulation? The camera should maintain its current orbit radius when switching targets


**AI Response:** This camera system allows a smooth, spherical-orbit-style camera (turntable camera) to interpolate between focus points (e.g. planets) in a solar system simulation. When switching between targets, the camera maintains its current orbit radius and orientation.

### Turntable Camera Overview

A turntable camera:
- Orbits a **focus point** (target) at a constant **radius**
- Uses spherical coordinates: **theta** (azimuth), **phi** (elevation)
- Calculates camera position with:

```cpp
cameraPos = focus + sphericalToCartesian(radius, theta, phi);
```

- Always looks at the focus point using `glm::lookAt`.


### Goal

When switching focus from one planet to another:
- Keep the **orbit radius** and **camera angles (theta, phi)** unchanged
- Smoothly interpolate the **focus point**
- Maintain consistent camera behavior for cinematic transitions


### Implementation Steps

### 1. Store Current and Target Focus Points

```cpp
glm::vec3 oldFocus = currentFocus;
glm::vec3 newFocus = targetPlanetPosition;

float transitionTime = 1.0f; // duration in seconds
float t = 0.0f;              // interpolation parameter (0 to 1)
```


### 2. Interpolate Focus in the Update Loop

```cpp
t += deltaTime / transitionTime;
t = glm::clamp(t, 0.0f, 1.0f);

// Smooth interpolation
currentFocus = glm::mix(oldFocus, newFocus, smoothstep(0.0f, 1.0f, t));
```

> `smoothstep` makes the transition ease-in and ease-out for smooth motion.


### 3. Maintain Spherical Orbit

Recalculate the camera position each frame using the unchanged radius, theta, and phi:

```cpp
glm::vec3 cameraOffset = sphericalToCartesian(radius, theta, phi);
cameraPos = currentFocus + cameraOffset;
```

### 4. Look At the Focus Point

Update the view matrix using `glm::lookAt`:

```cpp
viewMatrix = glm::lookAt(cameraPos, currentFocus, upVector);
```

### Summary

- Interpolate the **focus point** using `glm::mix` and `smoothstep`
- Maintain **radius**, **theta**, and **phi**
- Recalculate camera position each frame
- Use `glm::lookAt` to look at the interpolated focus

This creates smooth and cinematic camera transitions between planets while preserving the camera's current orbit.

---
For Part IV (Animation), I needed to implement frame-rate independent orbital motion with pause/speed control functionality. I asked ChatGPT in help for this, which Validate glfwGetTime() for precision timing. Which helped structure the animation state machine for pausing, implement delta-time clamping to prevent clock anomalies. This ensured accurate orbital periods regardless of framerate or system time changes.

**Prompt:** Is using glfwGetTime() delta the most robust way to implement frame-rate independent planetary motion? Need to handle: Animation pausing, Speed scaling, and System clock changes.

**AI Reponse:** 
glfwGetTime() Analysis:
Pros:
- High precision (µs resolution)
- Unaffected by frame rate spikes
- Continues during UI pauses

Recommended Implementation:
1. Store animation state separately:
   float animTime = paused ? lastTime : glfwGetTime() * speedFactor;
2. Calculate delta:
   float delta = animTime - lastAnimTime;
3. Update lastAnimTime = animTime;

Edge Cases:
- Handle negative deltas (system clock adjustments)
- Clamp deltas >100ms to avoid jumps

---

## References ##

Coordinate Systems: https://learnopengl.com/Getting-started/Coordinate-Systems  
Camera: https://learnopengl.com/Getting-started/Camera  
Lighting: https://learnopengl.com/Lighting/Basic-Lighting  
Blending: https://learnopengl.com/Advanced-OpenGL/Blending
LearnOpenGL - Coordinate Systems (n.d.). https://learnopengl.com/Getting-started/Coordinate-Systems
Tutorial 6 : Keyboard and Mouse. Opengl-Tutorial.org. https://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
Tutorial 17 : Rotations. (n.d.). https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
OpenGL 4 Reference Pages: https://registry.khronos.org/OpenGL/
University of Calgary. CPSC 453: Course notes. D2L Brightspace. https://d2l.ucalgary.ca/d2l/le/content/647982/viewContent/6819150/View
College Algebra: https://courses.lumenlearning.com/waymakercollegealgebra/chapter/equations-of-ellipses/
Elliptic orbit: https://space.fandom.com/wiki/Elliptic_orbit
Sphere: https://www.songho.ca/opengl/gl_sphere.html
Chapter 15: Hierarchical Transformations: https://graphicscompendium.com/opengl/13-hierarchical-modeling
Shaders: https://learnopengl.com/Getting-started/Shaders
Basic Lighting: https://learnopengl.com/Lighting/Basic-Lighting
Orbits and Kepler's Laws - NASA Science Kepler's Laws: https://science.nasa.gov/solar-system/orbits-and-keplers-laws/#:~:text=Kepler's%20Laws%20of%20Planetary%20Motion,-The%20story%20of&text=They%20describe%20how%20(1)%20planets,the%20size%20of%20its%20orbit.
Kepler's Three Laws: https://www.physicsclassroom.com/class/circles/lesson-4/kepler-s-three-laws
