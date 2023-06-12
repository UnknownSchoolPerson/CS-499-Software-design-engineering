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

#include <learnOpengl/camera.h> // Camera class

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
	glm::vec2 gUVScale(1.0f, 1.0f);
	GLint gTexWrapMode = GL_REPEAT;
	// Shader program
	GLuint gProgramId;

	// camera
	Camera gCamera(glm::vec3(0.0f, 3.0f, 10.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;
	bool gLockCamera = false;
	float gZoom = 2.0f;
	bool keyUp = true;

	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

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
void UProcessInput(GLFWwindow* window);
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


/* Vertex Shader Source Code*/
const GLchar * vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

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
const GLchar * fragmentShaderSource = GLSL(440,

	in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting*/
	float ambientStrength = 0.1f; // Set ambient or global lighting strength
	vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

	//Calculate Diffuse lighting*/
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
	vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse = impact * lightColor; // Generate diffuse light color

	//Calculate Specular lighting*/
	float specularIntensity = 1.0f; // Set specular light strength
	float highlightSize = 10.0f; // Set specular highlight size
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * lightColor;

	// Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

	// Calculate phong result
	vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

	fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
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
	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Putting this into unnamed namespace cause problems. IDK why. Here my fix.
	objectHandler items;
	createObjects(items);

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
		UProcessInput(gWindow);

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
void UProcessInput(GLFWwindow* window)
{
	float speed = gDeltaTime * speedOffset;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// My code
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
	{
		keyUp = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && keyUp)
	{
		keyUp = false;
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
	GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
	GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
	GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

	// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
	//glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightPositionLoc, 1.0f, 3.0f, 1.0f);
	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

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
	// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gPlaneMesh.vao);

	// bind textures on corresponding texture units
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, planeTex);

	// 1. Scales the object
	//scale = glm::scale(glm::vec3(6.0f, 1.0f, 6.0f));
	// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	//translation = glm::translate(glm::vec3(0.0f, 1.0f, 1.5f));
	// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;

	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);

	// Draws the triangles
	//glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	//glBindTexture(GL_TEXTURE_2D, 0);

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gBoxMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(-3.5f, 1.0f, -3.0f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.5f, 0.0f, 1.0f);

	//// Draws the triangles
	//glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gPyramid3Mesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(-2.0f, 1.0f, -3.0f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.5f, 0.5f, 1.0f);

	//// Draws the triangles
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid3Mesh.nVertices);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gPyramid4Mesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(0.0f, 1.0f, -3.0f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.0f, 0.5f, 1.0f);

	//// Draws the triangles
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gPrismMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(2.0f, 1.0f, -3.0f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.0f, 0.5f, 1.0f);

	//// Draws the triangles
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPrismMesh.nVertices);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gConeMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(4.0f, 0.5f, -3.0f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 1.0f, 1.0f);

	//// Draws the triangles
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	//glDrawArrays(GL_TRIANGLE_STRIP, 36, 108);	//sides

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	/*glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 0.05f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 2.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//https://www.tug.org/pracjourn/2007-4/walden/color.pdf
	//glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 1.0f, 0.9f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waxTex);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);*/

	// Activate the VBOs contained within the mesh's VAO
	/*glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.3f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.5f, 1.9f, 5.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//https://www.tug.org/pracjourn/2007-4/walden/color.pdf
	glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.6f, 0.0f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wickTex);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.025f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 1.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//https://www.tug.org/pracjourn/2007-4/walden/color.pdf
	glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 1.0f, 0.9f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glassTex);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.3f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.5f, 1.9f, 5.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//https://www.tug.org/pracjourn/2007-4/walden/color.pdf
	glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.6f, 0.0f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wickTex);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.3f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.5f, 1.9f, 5.7f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//https://www.tug.org/pracjourn/2007-4/walden/color.pdf
	glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.6f, 0.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Deactivate the Vertex Array Object
	glBindVertexArray(0); */

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(-1.2f, 1.0f, 5.5f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 1.0f, 1.0f, 1.0f);

	//// Draws the triangles
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	//glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	//glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	//// Activate the VBOs contained within the mesh's VAO
	/*glBindVertexArray(meshes.gTorusMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// 2. Rotate the object
	rotation = glm::rotate(180.0f, glm::vec3(0.8f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 2.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glassTex);

	glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.0f, 1.0f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	*/

	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gSphereMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(4.0f, 1.0f, 4.3f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

	//// Draws the triangles
	//glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
/*void UCreateMesh(GLMesh &mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions    // Colors (r,g,b,a)
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f, // Top Right Vertex 0
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f, // Bottom Right Vertex 1
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f, // Bottom Left Vertex 2
		-0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 1.0f, 1.0f, // Top Left Vertex 3

		 0.5f, -0.5f, -1.0f,  0.5f, 0.5f, 1.0f, 1.0f, // 4 br  right
		 0.5f,  0.5f, -1.0f,  1.0f, 1.0f, 0.5f, 1.0f, //  5 tl  right
		-0.5f,  0.5f, -1.0f,  0.2f, 0.2f, 0.5f, 1.0f, //  6 tl  top
		-0.5f, -0.5f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f  //  7 bl back
	};

	// Index data to share position data
	GLushort indices[] = {
		0, 1, 3,  // Triangle 1
		1, 2, 3,   // Triangle 2
		0, 1, 4,  // Triangle 3
		0, 4, 5,  // Triangle 4
		0, 5, 6, // Triangle 5
		0, 3, 6,  // Triangle 6
		4, 5, 6, // Triangle 7
		4, 6, 7, // Triangle 8
		2, 3, 6, // Triangle 9
		2, 6, 7, // Triangle 10
		1, 4, 7, // Triangle 11
		1, 2, 7 // Triangle 12
	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerColor = 4;

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(2, mesh.vbos);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);
}*/


//void UDestroyMesh(GLMesh &mesh)
//{
//	glDeleteVertexArrays(1, &mesh.vao);
//	glDeleteBuffers(2, mesh.vbos);
//}


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
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 1.0f, 5.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	items.addObject(model, glassTex, "cylinder", gProgramId);

}