#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <glfw3.h>          // GLFW library

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Headers 
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Camera class
#include <Camera.h>

using namespace std; // Uses the standard namespace

// Shader program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Justin Starr CS-360 Module 5 Milestone - Texturing Objects in a 3D Scene"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 800;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;     // Handles for the vertex buffer objects
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Triangle mesh data for the tip of the pen.
    GLMesh gPyramidMesh;

    // Cylinder mesh data for the body of the pen
    GLMesh gCylinderMesh;

    // Plane mesh data  for the desk
    GLMesh gPlaneMesh;

    // Texture id for the plane
    GLuint gTextureIdPlane;

    // Texture id for the tip of the pen
    GLuint gTextureIdTipOfPen;

    // Texture id for the body of the pen
    GLuint gTextureIdBodyOfPen;
    GLuint gTextureIdBodyOfPen2;
    

    // Shader program
    GLuint gProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 25.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // Time between current frame and last frame
    float gLastFrame = 0.0f;

    // for the projection
    bool perspective = false;

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */

bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);

void UCreatePyramidMesh(GLMesh& mesh);
void UCreateCylinderMesh(GLMesh& mesh);
void UCreatePlaneMesh(GLMesh& mesh);

bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);

void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

// callback functions to handle mouse input
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Vertex Shader Source Code. 
const GLchar* vertexShaderSource = GLSL(440,
layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 2) in vec2 textureCoordinate;  // Texture data from Vertex Attrib Pointer 2

out vec2 vertexTextureCoordinate; // variable to transfer texture data to the fragment shader

// Global variable for the transform matricies
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms verticies to clip coordinates
    vertexTextureCoordinate = textureCoordinate;
}
);

// Fragment Shader Source Code.
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;


void main()
{
    // used for advanced texturing technique
    // mixes two different textures - last parameter determines how much the textures are mixed. Values range from 0 to 1.
    fragmentColor = mix(texture(uTexture1, vertexTextureCoordinate), texture(uTexture2, vertexTextureCoordinate), 0.5); // Sends texture to the GPU for rendering
    
}
);

// Images are loaded with Y axis goinf down, but OpenGL's Y axis goes up, so we will flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create each object's mesh
    UCreatePlaneMesh(gPlaneMesh); // Calls the function to create the Vertex Buffer Object

    UCreatePyramidMesh(gPyramidMesh); // Calls the function to create the Vertex Buffer Object

    UCreateCylinderMesh(gCylinderMesh); // Calls the function to create the Vertex buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load texture (relative to project's directory)
    const char* texFilename = "plane_texture_2.png";
    if (!UCreateTexture(texFilename, gTextureIdPlane))
    {
        cout << "Failed to load texture for the plane " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture (relative to project's directory)
    texFilename = "tip_of_pen_texture.png";
    if (!UCreateTexture(texFilename, gTextureIdTipOfPen))
    {
        cout << "Failed to load texture for the tip of the pen " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture (relative to project's directory)
    texFilename = "body_of_pen_texture.png";
    if (!UCreateTexture(texFilename, gTextureIdBodyOfPen))
    {
        cout << "Failed to load plane texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // Load texture (relative to project's directory)
    texFilename = "body_of_pen_texture2.png";
    if (!UCreateTexture(texFilename, gTextureIdBodyOfPen2))
    {
        cout << "Failed to load plane texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture1"), 0);
    // We set the texture as texture unit 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture2"), 1);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // uncomment the following line to render the objects as a 3D mesh
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render Scene
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gPyramidMesh);
    UDestroyMesh(gCylinderMesh);
    UDestroyMesh(gPlaneMesh);

    // Release textures
    UDestroyTexture(gTextureIdPlane);
    UDestroyTexture(gTextureIdTipOfPen);
    UDestroyTexture(gTextureIdBodyOfPen);
    UDestroyTexture(gTextureIdBodyOfPen2);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // for users using MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

    //if there is an error while creating the window, an error message is displayed to the user
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // Register Mouse callbacks
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    // if glew does not initialize the error is displayed
    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // specifically checks if the escape key is pressed
        glfwSetWindowShouldClose(window, true); // if the escape key is pressed it sets the glfwWindowShouldClose to true

    float cameraOffset = cameraSpeed * gDeltaTime;

    bool keypress = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cout << "You pressed W! ";
        keypress = true;
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);  // moves the camera forward
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cout << "You pressed S! ";
        keypress = true;
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime); // moves the camera backward
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cout << "You pressed A! ";
        keypress = true;
        gCamera.ProcessKeyboard(LEFT, gDeltaTime); // moves the camera left
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cout << "You pressed D! ";
        keypress = true;
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime); // moves the camera right
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cout << "You pressed Q! ";
        keypress = true;
        gCamera.ProcessKeyboard(DOWN, gDeltaTime); // moves the camera right
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cout << "You pressed E! ";
        keypress = true;
        gCamera.ProcessKeyboard(UP, gDeltaTime); // moves the camera right
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        cout << "You pressed P! ";
        keypress = true;
        if (perspective)
        {
            perspective = false; // sets the projection to perspective when the 'P' key is pressed and if projection was ortho
            cout << "Perspective Projection Set ";
        }
        else
        {
            perspective = true; // sets the projection to ortho when the 'P' key is pressed and if projection was perspective
            cout << "Orthographic Projection Set ";
        }
    }
    if (keypress)
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        cout << "Cursor at position (" << x << ", " << y << ")" << endl;
    }
}

// glfw: Whenever the mouse moves, this callback is called.
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    cout << "Mouse at (" << xpos << ", " << ypos << ")" << endl;
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Whenever the mouse scroll wheel scrolls, this callback is called.
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
    cout << "Mouse wheel (" << xoffset << ", " << yoffset << ")" << endl;
}

// glfw: Handle mouse button events.
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height); // resizes the viewport
}

void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);
    
    // clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // -------------------  For the Plane  -------------------------

    // 1. Scales the object
    glm::mat4 scale = glm::scale(glm::vec3(12.0f, 8.0f, 0.0f));

    // 2. Rotates shape by 0 degrees on the y axis
    glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    // 3. Place object at origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, -0.1f));

    // Model matrix: Transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    glm::mat4 projection;
    if (!perspective)
    {
        // create a perspective projection matrix
        // First parameter is the field of view
        // Second parameter is the aspect ratio
        // Third parameter is the distance of the near plane to the camera
        // Fourth parameter is the distance of the far plane to the camera
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); // creates the ortho projection if the perspective is set to true
    }

    // Set the shader to be used
    glUseProgram(gProgramId);

    // retrieves and passes transform matricies to the shader program - All three matricies are transferred to the vertex shader as uniform variables
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gPlaneMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdPlane);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureIdPlane);

    // Draws the plane
    glDrawArrays(GL_TRIANGLES, 0, gPlaneMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // ----------------- for the cylinder ---------

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.10f, 0.10f, 1.75f));

    // 2. Rotates shape by 55 degrees on the x axis
    rotation = glm::rotate(55.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    // Rotate the shape on its y-axis 120 degrees
    rotation = glm::rotate(rotation, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 3. Place object at origin
    translation = glm::translate(glm::vec3(2.0f, -1.0f, 0.0f));

    // Model matrix: Transformations are applied right-to-left order
    model = translation * rotation * scale;

    // camera/view transformation
    view = gCamera.GetViewMatrix();

    if (!perspective)
    {
        // create a perspective projection matrix
        // First parameter is the field of view
        // Second parameter is the aspect ratio
        // Third parameter is the distance of the near plane to the camera
        // Fourth parameter is the distance of the far plane to the camera
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); // creates the ortho projection if the perspective is set to true
    }

    // retrieves and passes transform matricies to the shader program - All three matricies are transferred to the vertex shader as uniform variables
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gCylinderMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBodyOfPen);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureIdBodyOfPen2); // different color applied gives the pen the more stainless color that it does actually have.

    // Draws the cylinder
    glDrawArrays(GL_TRIANGLES, 0, gCylinderMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // -------------------- For the pyramid or tip of the pen -----------------------

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.10f, 0.10f, 0.5f));

    // 2. Rotates shape by 55 degrees on the x axis
    rotation = glm::rotate(55.0f, glm::vec3(1.0f, 0.0f, 0.0f));

    // Rotate the shape on its y-axis 120 degrees
    rotation = glm::rotate(rotation, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 3. Place object at origin
    translation = glm::translate(glm::vec3(2.0f, -1.0f, 0.0f));

    // Model matrix: Transformations are applied right-to-left order
    model = translation * rotation * scale;

    // camera/view transformation
    view = gCamera.GetViewMatrix();

    if (!perspective)
    {
        // create a perspective projection matrix
        // First parameter is the field of view
        // Second parameter is the aspect ratio
        // Third parameter is the distance of the near plane to the camera
        // Fourth parameter is the distance of the far plane to the camera
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
    {
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); // creates the ortho projection if the perspective is set to true
    }

    // retrieves and passes transform matricies to the shader program - All three matricies are transferred to the vertex shader as uniform variables
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gPyramidMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTipOfPen);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureIdTipOfPen);

    // Draws the cone for the tip of the pen.
    glDrawArrays(GL_TRIANGLES, 0, gPyramidMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
    
}

void UCreatePlaneMesh(GLMesh& mesh)
{
    GLfloat plane[] = {

        //Triangle 1
        //Vertex             //Texture
        -1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 
        1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 


        //Traingle 2
        //Vertex             //Texture
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 
        -1.0f, 1.0f, 0.0f,   0.0f, 1.0f 

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(plane) / (sizeof(plane[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateMesh function
void UCreatePyramidMesh(GLMesh& mesh)
{
    // Specifies normalized device coordinates (x,y,z) and color for square vertices
    GLfloat verts[] = {

        //pyramid
        // base
        //vertex            //Texture
        0.7f, 0.7f, 0.0f,   0.0f, 0.0f, 
        0.7f, -0.7f, 0.0f,  0.0f, 0.0f, 
        -0.7f, 0.7f, 0.0f,  0.0f, 0.0f, 

        -0.7f, 0.7f, 0.0f,  0.0f, 0.0f, 
        -0.7f, -0.7f, 0.0f, 0.0f, 0.0f, 
        0.7f, -0.7f, 0.0f,  0.0f, 0.0f, 

        //side 1
        0.0f, 0.0f, -1.0f,  0.5f, 1.0f, 
        0.7f, 0.7f, 0.0f,   1.0f, 0.0f, 
        0.7f, -0.7f, 0.0f,  0.0f, 0.0f, 

        //side 2
        0.0f, 0.0f, -1.0f,  0.5f, 1.0f, 
        0.7f, -0.7f, 0.0f,  1.0f, 0.0f, 
        -0.7f, -0.7f, 0.0f, 0.0f, 0.0f, 

        //side 3
        0.0f, 0.0f, -1.0f,  0.5f, 1.0f, 
        -0.7f, -0.7f, 0.0f, 1.0f, 0.0f, 
        -0.7f, 0.7f, 0.0f,  0.0f, 0.0f, 

        //side 4
        0.0f, 0.0f, -1.0f,  0.5f, 1.0f, 
        -0.7f, 0.7f, 0.0f,  1.0f, 0.0f, 
        0.7f, 0.7f, 0.0f,   0.0f, 0.0f

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);

}

void UCreateCylinderMesh(GLMesh& mesh)
{
    GLfloat verts[] = {

        //base 1 of cylinder
        //vertex            //Texture
        1.0f, 0.4f, 0.0f,   1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        1.0f, -0.4f, 0.0f,  1.0f, 1.0f, 

        1.0f, -0.4f, 0.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        0.4f, -1.0f, 0.0f,  1.0f, 1.0f, 

        0.4f, -1.0f, 0.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        -0.4f, -1.0f, 0.0f, 1.0f, 1.0f, 

        -0.4f, -1.0f, 0.0f, 1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        -1.0f, -0.4f, 0.0f, 1.0f, 1.0f, 

        -1.0f, -0.4f, 0.0f, 1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        -1.0f, 0.4, 0.0f,   1.0f, 1.0f, 

        -1.0f, 0.4f, 0.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        -0.4f, 1.0f, 0.0f,  1.0f, 1.0f, 

        -0.4f, 1.0f, 0.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        0.4f, 1.0f, 0.0f,   1.0f, 1.0f, 

        0.4f, 1.0f, 0.0f,   1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 
        1.0f, 0.4f, 0.0f,   1.0f, 1.0f, 

        //base 2 of cylinder
        //vertex            //Texture
        1.0f, 0.4f, 2.0f,   1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        1.0f, -0.4f, 2.0f,  1.0f, 1.0f, 

        1.0f, -0.4f, 2.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        0.4f, -1.0f, 2.0f,  1.0f, 1.0f, 

        0.4f, -1.0f, 2.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        -0.4f, -1.0f, 2.0f, 1.0f, 1.0f, 

        -0.4f, -1.0f, 2.0f, 1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        -1.0f, -0.4f, 2.0f, 1.0f, 1.0f, 

        -1.0f, -0.4f, 2.0f, 1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        -1.0f, 0.4, 2.0f,   1.0f, 1.0f, 

        -1.0f, 0.4f, 2.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        -0.4f, 1.0f, 2.0f,  1.0f, 1.0f, 

        -0.4f, 1.0f, 2.0f,  1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        0.4f, 1.0f, 2.0f,   1.0f, 1.0f, 

        0.4f, 1.0f, 2.0f,   1.0f, 1.0f, 
        0.0f, 0.0f, 2.0f,   1.0f, 1.0f, 
        1.0f, 0.4f, 2.0f,   1.0f, 1.0f, 

        //side 1
        //vertex            //Texture
        1.0f, 0.4f, 0.0f,   0.25f, 1.0f, 
        1.0f, -0.4f, 0.0f,  0.0f, 1.0f, 
        1.0f, 0.4f, 2.0f,   0.0f, 0.0f, 

        1.0f, -0.4f, 0.0f,  0.0f, 1.0f, 
        1.0f, 0.4f, 2.0f,   0.25f, 0.0f, 
        1.0f, -0.4f, 2.0f,  0.0f, 0.0f, 

        //side 2
        1.0f, -0.4f, 0.0f,  0.5f, 1.0f, 
        0.4f, -1.0f, 0.0f,  0.25f, 1.0f, 
        1.0f, -0.4f, 2.0f,  0.25f, 0.0f, 

        0.4f, -1.0f, 0.0f,  0.25f, 1.0f, 
        1.0f, -0.4f, 2.0f,  0.5f, 0.0f, 
        0.4f, -1.0f, 2.0f,  0.25f, 0.0f, 

        //side 3
        0.4f, -1.0f, 0.0f,  0.75f, 1.0f, 
        -0.4f, -1.0f, 0.0f, 0.5f, 1.0f, 
        0.4f, -1.0f, 2.0f,  0.5f, 0.0f, 

        -0.4f, -1.0f, 0.0f, 0.5f, 1.0f, 
        0.4f, -1.0f, 2.0f,  0.75f, 0.0f, 
        -0.4f, -1.0f, 2.0f, 0.5f, 0.0f, 

        //side 4
        -0.4f, -1.0f, 0.0f, 1.0f, 1.0f, 
        -1.0f, -0.4f, 0.0f, 0.75f, 1.0f, 
        -0.4f, -1.0f, 2.0f, 0.75f, 0.0f, 

        -1.0f, -0.4f, 0.0f, 1.0f, 1.0f, 
        -0.4f, -1.0f, 2.0f, 1.0f, 0.0f, 
        -1.0f, -0.4f, 2.0f, 0.75f, 0.0f, 

        //side 5
        -1.0f, -0.4f, 0.0f, 0.25f, 1.0f, 
        -1.0f, 0.4f, 0.0f,  0.0f, 1.0f, 
        -1.0f, -0.4f, 2.0f, 0.0f, 0.0f, 

        -1.0f, 0.4f, 0.0f,  0.0f, 1.0f, 
        -1.0f, -0.4f, 2.0f, 0.25f, 0.0f, 
        -1.0f, 0.4f, 2.0f,  0.0f, 0.0f, 

        //side 6
        -1.0f, 0.4f, 0.0f,  0.5f, 1.0f, 
        -0.4f, 1.0f, 0.0f,  0.25f, 1.0f, 
        -1.0f, 0.4f, 2.0f,  0.25f, 0.0f, 

        -0.4f, 1.0f, 0.0f,  0.25f, 1.0f, 
        -1.0f, 0.4f, 2.0f,  0.5f, 0.0f, 
        -0.4f, 1.0f, 2.0f,  0.25f, 0.0f, 

        //side 7
        -0.4f, 1.0f, 0.0f,  0.75, 1.0f, 
        0.4f, 1.0f, 0.0f,   0.5f, 1.0f, 
        -0.4f, 1.0f, 2.0f,  0.5f, 0.0f, 

        0.4f, 1.0f, 0.0f,   0.5f, 1.0f, 
        -0.4f, 1.0f, 2.0f,  0.75f, 0.0f, 
        0.4f, 1.0f, 2.0f,   0.5f, 0.0f, 

        //side 8
        0.4f, 1.0f, 0.0f,   1.0f, 1.0f, 
        1.0f, 0.4f, 0.0f,   0.75f, 1.0f, 
        0.4f, 1.0f, 2.0f,   0.75f, 0.0f, 

        1.0f, 0.4f, 0.0f,   1.0f, 1.0f, 
        0.4f, 1.0f, 2.0f,   1.0f, 0.0f, 
        1.0f, 0.4f, 2.0f,   0.75f, 0.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(2);
}

// de-allocates resources once they have outlived their purpose
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // Set the texture wrapping parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set texture filtering parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture.

        return true;
    }

    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);

    // if shader compile has an error, the error is displayed to the user
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);

    // if shader compile has an error, the error is displayed to the user
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);

    // if there is a linking error, the error is displayed to the user
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}

// de-allocates resource once it has outlived its purpose
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}