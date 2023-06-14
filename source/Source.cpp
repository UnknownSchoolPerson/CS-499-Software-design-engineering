#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions
#include "objectHandler.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/string_cast.hpp> // https://gist.github.com/donaldmunro/38841d72c65a1c32f2bf83a4a00a2c9a

#include <learnOpengl/camera.h> // Camera class
#include <glm/gtc/type_ptr.hpp>

//#include "meshes.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "4-5 Milestone CS-330"; // Macro for window title

	// For speed scaling
	float speedOffset = 1.0f;

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	//GLMesh gMesh;
	// Texture
	GLuint planeTex;
	GLuint waxTex;
	GLuint wickTex;
	GLuint glassTex;
	GLuint selectedTex;
	GLuint greyTex;
	GLuint cyanTex;
	GLuint woodTex;
	GLuint jarGlassTex;
	GLuint plasticTex;
	GLuint whiteTex;
	glm::vec2 gUVScale(1.0f, 1.0f);
	GLint gTexWrapMode = GL_REPEAT;
	// Shader program
	GLuint gProgramId;

	//Object Changer Vars
	int vecSpot = 0;
	enum tool {
		Rotate,
		Scale,
		Translation
	};
	tool currentTool = Rotate;
	GLuint prevTex;
	bool enableSelectedTex = false;
	float changeBy = 1.0;
	float stepBy = 1.0;
	bool editX = false, editY = false, editZ = false;

	// camera
	Camera gCamera(glm::vec3(0.0f, 3.0f, 10.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;
	bool gLockCamera = false;
	float gZoom = 2.0f;
	bool keyUp[348];

	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

	// Cube and light color
	//m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
	//glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
	glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 gSecondLightColor(1.0f, 1.0f, 1.0f);
	//GLfloat specularIntensity = 0.8f;
	//GLfloat highlightSize = 16.0f;

	// Light position and scale
	glm::vec3 gSecondLightPosition(-1.0f, 0.5f, 3.0f);
	glm::vec3 gLightPosition(-2.0f, 4.5f, -5.0f);
	glm::vec3 gLightScale(0.3f);

	//Shape Meshes from Professor Brian
	//Meshes meshes;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char*[], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window, objectHandler &items);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
//void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
//void UCreateMesh(GLMesh &mesh);
//void UDestroyMesh(GLMesh &mesh);
void URender(objectHandler &items);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId);
void UDestroyShaderProgram(GLuint programId);
bool bindTex(const char* texFilename, GLuint& texToBind);
void createObjects(objectHandler &items);
void createTestObjects(objectHandler& items);
void objectChanger(objectHandler& items, int key);


/* Vertex Shader Source Code*/
/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout(location = 1) in vec3 normal;  // Color data from Vertex Attrib Pointer 1
    layout(location = 2) in vec2 textureCoordinate;

    //out vec4 vertexColor; // variable to transfer color data to the fragment shader
    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    //in vec4 vertexColor; // Variable to hold incoming color data from vertex shader
    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec4 objectColor;
    uniform vec3 ambientColor;
    uniform vec3 light1Color;
    uniform vec3 light1Position;
    uniform vec3 light2Color;
    uniform vec3 light2Position;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;
    uniform bool ubHasTexture;
    uniform float ambientStrength = 0.1f; // Set ambient or global lighting strength
    uniform float specularIntensity1 = 0.1f;
    uniform float highlightSize1 = 16.0f;
    uniform float specularIntensity2 = 1.0f;
    uniform float highlightSize2 = 16.0f;

    void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting
        vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

        //**Calculate Diffuse lighting**
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 light1Direction = normalize(light1Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color
        vec3 light2Direction = normalize(light2Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact2 = max(dot(norm, light2Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse2 = impact2 * light2Color; // Generate diffuse light color

        //**Calculate Specular lighting**
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
        vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;
        vec3 reflectDir2 = reflect(-light2Direction, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
        vec3 specular2 = specularIntensity2 * specularComponent2 * light2Color;

        //**Calculate phong result**
        //Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
        vec3 phong1;
        vec3 phong2;

        if (ubHasTexture == true)
        {
            phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
            phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
        }
        else
        {
            phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
            phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
        }

        fragmentColor = vec4(phong1 + phong2, 1.0); // Send lighting results to GPU
        //fragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
);


int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	//UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
	//meshes.CreateMeshes();

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	for (auto &key : keyUp)
		key = true;
	// Load texture
	//https://commons.wikimedia.org/wiki/File:Red-brick-wall-texture-clean.jpg
	if (!bindTex("../../resources/textures/brick-wall.jpg", planeTex))
		return EXIT_FAILURE;
	//https://www.freepik.com/premium-photo/soy-wax-flakes-texture-closeup_36977469.htm
	if (!bindTex("../../resources/textures/wax.jpg", waxTex))
		return EXIT_FAILURE;
	//https://pxhere.com/en/photo/1021832
	if (!bindTex("../../resources/textures/wick.jpg", wickTex))
		return EXIT_FAILURE;
	//https://commons.wikimedia.org/wiki/File:Solarsystemscope_texture_2k_uranus.jpg
	if (!bindTex("../../resources/textures/glass.jpg", glassTex))
		return EXIT_FAILURE;
	//https://gmod.fandom.com/wiki/Missing_textures?file=The_Missing_textures.png
	if (!bindTex("../../resources/textures/untex.png", selectedTex))
		return EXIT_FAILURE;
	//Me
	if (!bindTex("../../resources/textures/grey.png", greyTex))
		return EXIT_FAILURE;
	if (!bindTex("../../resources/textures/cyan.png", cyanTex))
		return EXIT_FAILURE;
	//https://pxhere.com/en/photo/598879
	if (!bindTex("../../resources/textures/wood.jpg", woodTex))
		return EXIT_FAILURE;
	//https://commons.wikimedia.org/wiki/File:Wallpaper_glass_fiber_texture.jpg
	if (!bindTex("../../resources/textures/jarglass.jpg", jarGlassTex))
		return EXIT_FAILURE;

	//https://www.pexels.com/photo/photo-of-a-clear-plastic-taped-on-a-white-surface-4587829/
	if (!bindTex("../../resources/textures/plastic.jpg", plasticTex))
		return EXIT_FAILURE;
	//me
	if (!bindTex("../../resources/textures/white.png", whiteTex))
		return EXIT_FAILURE;
	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Putting this into unnamed namespace cause problems. IDK why. Here my fix.
	objectHandler items;
	createObjects(items);
	//createTestObjects(items);

	cout << "Controls for object editor." << endl;
	cout << "1 - Add to current object with selected tool" << endl;
	cout << "2 - Subtract from current object with selected tool" << endl;
	cout << "3 - Change to Rotate (Current Tool)" << endl;
	cout << "4 - Change to Translation" << endl;
	cout << "5 - Change to Scale" << endl;
	cout << "Note - Scaling is multiplied with Change by" << endl;
	cout << "or when shrinking (1 / Change by)" << endl;
	cout << "6 - Enable/Disable Edit X" << endl;
	cout << "7 - Enable/Disable Edit Y" << endl;
	cout << "8 - Enable/Disable Edit Z" << endl;
	cout << "9 - Decrease Step" << endl;
	cout << "0 - Increase Step" << endl;
	cout << "- - Decrease Change by" << endl;
	cout << "+ - Increase Change by" << endl;
	cout << "[ - Change Object" << endl;
	cout << "] - Change Object" << endl;
	cout << "Z - Enable/Disable Selected Object Texture" << endl;
	cout << "X - Dump selected object Matrix" << endl;
	cout << "C - Change Object Shape" << endl;
	cout << "V - Spawn Object (Always at 5, 5, 0 with no changes, make sure to look up!)" << endl;




	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{

		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;
		// input
		// -----
		UProcessInput(gWindow, items);

		// Render this frame
		URender(items);

		glfwPollEvents();
	}

	// Release mesh data
	//UDestroyMesh(gMesh);
	//meshes.DestroyMeshes();

	// Release texture
	//UDestroyTexture(planeTex);
	//UDestroyTexture(waxTex);
	//UDestroyTexture(wickTex);
	//UDestroyTexture(glassTex);

	// Release shader program
	UDestroyShaderProgram(gProgramId);

	return EXIT_SUCCESS; // Terminates the program successfully
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

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	//glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

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
void UProcessInput(GLFWwindow* window, objectHandler& items)
{
	float speed = gDeltaTime * speedOffset;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// My code
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_P] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_X] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_1] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_2] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_3] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_4] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_5] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_6] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_7] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_8] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_RIGHT_BRACKET] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_LEFT_BRACKET] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_Z] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_MINUS] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_EQUAL] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_9] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_0] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_V] = true;
	}

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
	{
		keyUp[GLFW_KEY_C] = true;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && keyUp[GLFW_KEY_P])
	{
		keyUp[GLFW_KEY_P] = false;
		if (!gLockCamera)
		{
			gLockCamera = true;
			gCamera.Position = glm::vec3(0.0f, 3.0f, 10.0f);
			//cout << gCamera.Pitch << endl;
			//cout << gCamera.Yaw << endl;
			gCamera.Pitch = -10.0f;
			gCamera.Yaw = -90.0f;
			gCamera.ProcessMouseMovement(0.0f, 0.0f);
		}
		else
			gLockCamera = false;
	}
	if (gLockCamera)
		return;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed = gDeltaTime * 40.0f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, speed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, speed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, speed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, speed);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, speed);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, speed);

	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && keyUp[GLFW_KEY_MINUS])
	{
		keyUp[GLFW_KEY_MINUS] = false;
		changeBy -= stepBy;
		if (changeBy < 0.0f)
			changeBy = 0.0f;
		cout << "Current Change is " << changeBy << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && keyUp[GLFW_KEY_EQUAL])
	{
		keyUp[GLFW_KEY_EQUAL] = false;
		changeBy += stepBy;
		if (changeBy > 10.0f)
			changeBy = 10.0f;
		cout << "Current Change is " << changeBy << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS && keyUp[GLFW_KEY_9])
	{
		keyUp[GLFW_KEY_9] = false;
		stepBy *= 0.1f;
		if (stepBy < 0.0001f)
			stepBy = 0.0001f;
		cout << "Current Step is " << stepBy << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && keyUp[GLFW_KEY_0])
	{
		keyUp[GLFW_KEY_0] = false;
		stepBy *= 10.0f;
		if (stepBy > 10.0f)
			stepBy = 10.0f;
		cout << "Current Step is " << stepBy << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && keyUp[GLFW_KEY_Z])
	{
		keyUp[GLFW_KEY_Z] = false;
		if (enableSelectedTex) {
			cout << "Selected Texture Off" << endl;
			enableSelectedTex = false;
			items.getObjectList()->at(vecSpot).texture = prevTex;
		}
		else {
			cout << "Selected Texture On" << endl;
			enableSelectedTex = true;
			items.getObjectList()->at(vecSpot).texture = selectedTex;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS && keyUp[GLFW_KEY_LEFT_BRACKET])
	{
		keyUp[GLFW_KEY_LEFT_BRACKET] = false;
		if (enableSelectedTex)
			items.getObjectList()->at(vecSpot).texture = prevTex;
		vecSpot--;
		if (vecSpot < 0)
			vecSpot = items.getObjectList()->size() - 1;
		if (enableSelectedTex) {
			prevTex = items.getObjectList()->at(vecSpot).texture;
			items.getObjectList()->at(vecSpot).texture = selectedTex;
		}
		cout << "Current Object is " << vecSpot << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS && keyUp[GLFW_KEY_RIGHT_BRACKET])
	{
		keyUp[GLFW_KEY_RIGHT_BRACKET] = false;
		if (enableSelectedTex)
			items.getObjectList()->at(vecSpot).texture = prevTex;
		vecSpot++;
		if (vecSpot == items.getObjectList()->size())
			vecSpot = 0;
		if (enableSelectedTex) {
			prevTex = items.getObjectList()->at(vecSpot).texture;
			items.getObjectList()->at(vecSpot).texture = selectedTex;
		}
		cout << "Current Object is " << vecSpot << endl;
	}
	//https://gist.github.com/donaldmunro/38841d72c65a1c32f2bf83a4a00a2c9a
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && keyUp[GLFW_KEY_X])
	{
		keyUp[GLFW_KEY_X] = false;
		objectChanger(items, GLFW_KEY_X);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && keyUp[GLFW_KEY_1])
	{
		keyUp[GLFW_KEY_1] = false;
		objectChanger(items, GLFW_KEY_1);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && keyUp[GLFW_KEY_2])
	{
		keyUp[GLFW_KEY_2] = false;
		objectChanger(items, GLFW_KEY_2);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && keyUp[GLFW_KEY_3])
	{
		keyUp[GLFW_KEY_3] = false;
		cout << "Current tool is Rotate" << endl;
		currentTool = Rotate;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && keyUp[GLFW_KEY_4])
	{
		keyUp[GLFW_KEY_4] = false;
		cout << "Current tool is Translation" << endl;
		currentTool = Translation;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && keyUp[GLFW_KEY_5])
	{
		keyUp[GLFW_KEY_5] = false;
		cout << "Current tool is Scale" << endl;
		currentTool = Scale;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && keyUp[GLFW_KEY_6])
	{
		keyUp[GLFW_KEY_6] = false;
		if (editX) {
			cout << "X won't be changed." << endl;
			editX = false;
		}
		else {
			cout << "X will be changed." << endl;
			editX = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && keyUp[GLFW_KEY_7])
	{
		keyUp[GLFW_KEY_7] = false;
		if (editY) {
			cout << "Y won't be changed." << endl;
			editY = false;
		}
		else {
			cout << "Y will be changed." << endl;
			editY = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS && keyUp[GLFW_KEY_8])
	{
		keyUp[GLFW_KEY_8] = false;
		if (editZ) {
			cout << "Z won't be changed." << endl;
			editZ = false;
		}
		else {
			cout << "Z will be changed." << endl;
			editZ = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && keyUp[GLFW_KEY_C])
	{
		keyUp[GLFW_KEY_C] = false;
		objectChanger(items, GLFW_KEY_C);
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && keyUp[GLFW_KEY_V])
	{
		keyUp[GLFW_KEY_V] = false;
		// 1. Scales the object
		glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
		// 2. Rotate the object
		glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		// 3. Position the object
		glm::mat4 translation = glm::translate(glm::vec3(5.0f, 5.0f, 0.0f));
		// Model matrix: transformations are applied right-to-left order
		glm::mat4 model = translation * rotation * scale;
		items.addObject(model, glassTex, "box", gProgramId);
		cout << "Object Spawned! Look up!" << endl;
	}
}

void objectChanger(objectHandler& items, int key) {
	auto* vec = items.getObjectList();
	if (key == GLFW_KEY_X) {
		cout << glm::to_string(vec->at(vecSpot).location) << std::endl;
		return;
	}
	if (key == GLFW_KEY_C) {

		int num = vec->at(vecSpot).mesh;
		num++;
		if (num >= 10)
			num = 0;
		vec->at(vecSpot).mesh = objectHandler::meshShape(num);
		cout << "Object shape is now " << num << endl;
		return;
	}
	/*
	glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	glm::mat4 translation = glm::translate(glm::vec3(5.0f, 5.0f, 5.5f));
	*/
	float x = 0.0, y = 0.0, z = 0.0;
	if (editX)
	{
		x = changeBy;
		cout << "X Changed" << endl;
	}
	if (editY)
	{
		y = changeBy;
		cout << "Y Changed" << endl;
	}
	if (editZ)
	{
		z = changeBy;
		cout << "Z Changed" << endl;
	}
	// If nothing is being changed, pass
	if (x + y + z == 0)
		return;
	switch (currentTool) {
		case Rotate:
			//https://www.reddit.com/r/opengl/comments/sih6lc/4x4_matrix_to_position_rotation_and_scale/
			glm::vec3 position = vec->at(vecSpot).location[3]; // 4th column of the model matrix
			if (!editX)
				position.x = 0.0;
			if (!editY)
				position.y = 0.0;
			if (!editZ)
				position.z = 0.0;
			if (position.x == 0.0 && position.y == 0.0 && position.z == 0.0)
				return; //This is a bug. Object breaks rotating at a 0 axis.
			//https://stackoverflow.com/questions/8844585/glm-rotate-usage-in-opengl
			if (key == GLFW_KEY_1)
				vec->at(vecSpot).location = glm::rotate(vec->at(vecSpot).location, glm::radians(changeBy), glm::vec3(position.x, position.y, position.z));
			else
				vec->at(vecSpot).location = glm::rotate(vec->at(vecSpot).location, glm::radians(-changeBy), glm::vec3(position.x, position.y, position.z));
			//glm::vec3 position = vec->at(vecSpot).location[3]; // 4th column of the model matrix
			//cout << position.x << endl;
			return;
		case Translation:
			if (key == GLFW_KEY_1)
				vec->at(vecSpot).location = glm::translate(vec->at(vecSpot).location, glm::vec3(x, y, z));
			else
				vec->at(vecSpot).location = glm::translate(vec->at(vecSpot).location, glm::vec3(-x, -y, -z));
			return;
		case Scale:
			if (x < 1.0)
				x = 1.0;
			if (y < 1.0)
				y = 1.0;
			if (z < 1.0)
				z = 1.0;
			if (key == GLFW_KEY_1)
				vec->at(vecSpot).location = glm::scale(vec->at(vecSpot).location, glm::vec3(x, y, z));
			else
				vec->at(vecSpot).location = glm::scale(vec->at(vecSpot).location, glm::vec3(1.0f / x, 1.0f / y, 1.0f / z));
			return;
		default:
			return;
	}

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// Functioned called to render a frame
void URender(objectHandler &items)
{
	//glm::mat4 scale;
	//glm::mat4 rotation;
	//glm::mat4 translation;
	//glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	//GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	//GLint objectColorLoc;

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Transforms the camera
	//if (gLockCamera)
	//view = glm::translate(glm::vec3(0.0f, -3.0f, -10.0f));
	//view = glm::lookAt(gCameraPos, gCameraPos + gCameraFront, gCameraUp);
	//else
	view = gCamera.GetViewMatrix();

	// Creates a orthographic projection
	if (gLockCamera)
		projection = glm::ortho(-gZoom, gZoom, -gZoom, gZoom, 0.1f, 100.0f);
	//projection = glm::perspective(glm::radians(60.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	else
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
	//objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
	GLint ambientColorLoc = glGetUniformLocation(gProgramId, "ambientColor");
	GLint lightColorLoc = glGetUniformLocation(gProgramId, "light1Color");
	GLint lightPositionLoc = glGetUniformLocation(gProgramId, "light1Position");
	GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
	GLint light2ColorLoc = glGetUniformLocation(gProgramId, "light2Color");
	GLint light2PositionLoc = glGetUniformLocation(gProgramId, "light2Position");

	// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
	//glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
	glUniform3f(ambientColorLoc, gSecondLightColor.r, gSecondLightColor.g, gSecondLightColor.b);
	glUniform3f(lightColorLoc, gSecondLightColor.r, gSecondLightColor.g, gSecondLightColor.b);
	glUniform3f(light2ColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
	glUniform3f(lightPositionLoc, gSecondLightColor.x, gSecondLightColor.y, gSecondLightColor.z);
	glUniform3f(light2PositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);

	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	bool ubHasTextureVal = true;
	GLint uHasTextureLoc = glGetUniformLocation(gProgramId, "ubHasTexture");
	glUniform1i(uHasTextureLoc, ubHasTextureVal);

	// Retrieves and passes transform matrices to the Shader program
	//modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	//objectColorLoc = glGetUniformLocation(gProgramId, "uObjectColor");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	items.renderAll();

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId)
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
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
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
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	//gCamera.ProcessMouseScroll(yoffset);
	//cout << yoffset << endl;
	if (gLockCamera)
	{
		gZoom += yoffset;
		gZoom = max(gZoom, 2.0f);
		gZoom = min(gZoom, 10.0f);
		//cout << gZoom << endl;
	}
	else
	{
		speedOffset += yoffset;
		speedOffset = max(speedOffset, 1.0f);
		speedOffset = min(speedOffset, 20.0f);
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gLockCamera)
		return;
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		//flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}


void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}

bool bindTex(const char* texFilename, GLuint& texToBind) {
	stbi_set_flip_vertically_on_load(true);
	if (!UCreateTexture(texFilename, texToBind))
	{
		cout << "Failed to load texture " << texFilename << endl;
		//system("PAUSE");
		return false;
	}
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
	return true;
}

void createObjects(objectHandler &items) {
	// 1. Scales the object
	glm::mat4 scale = glm::scale(glm::vec3(6.0f, 1.0f, 6.0f));
	// 2. Rotate the object
	glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 1.0f, 1.5f));
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;
	items.addObject(model, planeTex, "plane", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(180.0f, glm::vec3(0.8f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 2.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "torus", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 0.05f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 2.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, waxTex, "cylinder", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.3f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.5f, 1.9f, 5.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, wickTex, "cylinder", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.3f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.5f, 1.9f, 5.7f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, wickTex, "cylinder", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 1.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "cylinder", gProgramId);

	//Box
	//https://stackoverflow.com/questions/7351659/how-to-initialize-a-glmmat4-with-an-array
	float *location = new float[16]{ 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.250000, -0.000000, 0.000000, -0.000000, 0.000000, 1.000000, 0.000000, -1.000001, 1.224999, -2.999997, 1.000000 };
	model = glm::make_mat4(location);
	items.addObject(model, greyTex, "box", gProgramId);
	delete[] location;

	location = new float[16] { 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.070436, -0.000000, 0.000000, -0.000000, 0.000000, 1.000000, 0.000000, -1.000001, 1.065624, -2.999997, 1.000000 };
	model = glm::make_mat4(location);
	items.addObject(model, cyanTex, "box", gProgramId);
	delete[] location;

	location = new float[16] {1.000000, 0.000060, 0.000493, 0.000000, 0.000000, 0.021927, -0.037829, 0.000000, -0.000524, 0.115491, 0.940911, 0.000000, -1.000000, 1.455441, -3.068074, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, cyanTex, "box", gProgramId);
	delete[] location;

	location = new float[16] {1.000002, 0.000000, -0.000000, 0.000000, 0.000001, 0.391620, -0.000002, 0.000000, -0.000000, -0.000000, 0.063039, 0.000000, -1.000002, 1.223701, -3.528664, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, cyanTex, "box", gProgramId);
	delete[] location;

	// Jar
	location = new float[16] {0.634311, 0.000000, -0.000000, 0.000000, 0.000000, 2.000000, 0.000000, 0.000000, -0.000000, -0.000000, 0.634311, 0.000000, -2.666669, 1.000000, -3.333334, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, jarGlassTex, "cylinder", gProgramId);
	delete[] location;

	location = new float[16] {0.601270, 0.000000, 0.000000, 0.000000, 0.000000, -0.000000, -0.601270, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, -2.666669, 3.039999, -3.333334, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, jarGlassTex, "torus", gProgramId);
	delete[] location;

	location = new float[16] {0.606061, 0.000000, -0.000000, 0.000000, 0.000000, 0.062500, 0.000000, 0.000000, -0.000000, -0.000000, 0.606061, 0.000000, -2.666669, 3.052499, -3.333334, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, woodTex, "cylinder", gProgramId);
	delete[] location;


	//waxbox
	location = new float[16] {1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.289664, 0.000000, 0.000000, 0.000000, 0.000000, 1.331000, 0.000000, 4.299999, 1.239302, 5.500000, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, plasticTex, "box", gProgramId);
	delete[] location;

	location = new float[16] {0.564474, 0.000000, 0.000000, 0.000000, 0.000000, 0.289664, 0.000000, 0.000000, 0.000000, 0.000000, 0.909091, 0.000000, 4.299999, 1.239302, 5.318182, 1.000000};
	model = glm::make_mat4(location);
	items.addObject(model, whiteTex, "plane", gProgramId);
	delete[] location;




	prevTex = items.getObjectList()->at(0).texture;
}

void createTestObjects(objectHandler& items) {

	// 1. Scales the object
	glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	glm::mat4 rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	glm::mat4 translation = glm::translate(glm::vec3(5.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;
	items.addObject(model, glassTex, "Sphere", gProgramId);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(3.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "TaperedCylinder", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(2.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "cone", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(7.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "Prism", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(9.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "pyramid", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "pyramid3", gProgramId);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.0f, 5.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "box", gProgramId);
}