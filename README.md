# OpenGL Mesh Viewer

![olio_mesh_view](figures/olio_mesh_view.png)


## OpenGL-based Mesh Viewer Implementation
---- 
### Objective
The task involves creating an OpenGL-based mesh viewer capable of displaying multiple models simultaneously, each shaded using the Blinn-Phong illumination model. User interaction functionalities, such as mouse-driven model rotation and camera movement, are also implemented. The viewer is expected to load models, apply transformations for scaling and positioning, and render them alongside interactions and appropriate lighting effects.

---- 

### Components and Implementation
1. Model Loading and Display

    - The application loads models provided as command-line arguments and arranges them uniformly within a 2x2x2 cube centered at the origin.

    - A new TriMesh class manages model loading (Load), bounding box calculation (GetBoundingBox), OpenGL buffer handling (DeleteGLBuffers, UpdateGLBuffers), and rendering (DrawGL).
    Shading and Lighting

    - Implements the Blinn-Phong shading model for each model, computed within the fragment shader, resulting in smoother color transitions and improved specular highlights.
    - Utilizes three point lights with varying positions and colors to illuminate the models, enhancing their visual appearance.

1. User Interaction

    - Model Rotation: Allows users to click and drag the mouse to rotate the models around the origin. The x and y changes in mouse coordinates determine the rotation matrices applied to the models.
    - Camera Movement: Incorporates keyboard input (x and z keys) to adjust the camera's distance from the origin along the z-axis, preventing the camera from moving too close to the models.
    - Reset Functionality: Pressing the spacebar resets both the model rotations and the camera position.
    Camera Setup

    - Initially positions the camera at (0, 0, 2), looking at the origin along the -z direction, with an up-vector of (0, 1, 0), and a vertical field of view of 60 degrees.

### Technical Implementation Notes
1. Shader Implementation
    - Implements new vertex and fragment shaders to facilitate the Blinn-Phong shading model for enhanced visual quality and lighting effects.

1. OpenGL Rendering
    - Utilizes OpenGL functionality to manage and display meshes, leveraging vertex buffer objects for efficient rendering.
1. User Interaction Handling
    - Incorporates GLFW callbacks for mouse button and cursor position events to manage model rotation.
    - Manages keyboard input to adjust the camera's distance from the models and reset functionalities.

### Result and Recommendations
The completed assignment provides a functional OpenGL-based mesh viewer capable of loading, positioning, and shading multiple models using the Blinn-Phong illumination model. It offers interactive features for user-controlled model rotation and camera movement, ensuring a dynamic and immersive viewing experience. Recommendations for further improvements might include optimizations for rendering performance, additional shader effects, or extending the application's functionality to support more complex meshes or textures.





