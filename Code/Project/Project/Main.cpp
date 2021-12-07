#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void animacion();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(-100.0f, 2.0f, -45.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
float range = 0.0f;


// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;


// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// Keyframes
float rightArm1, leftArm1;
//float rightArm2, leftArm2, rightLeg2, leftLeg2;

#define MAX_FRAMES 9
int i_max_steps = 190;
int i_curr_steps = 0;
typedef struct _frame
{
	float rightArm1;
	float incRightArm1;
	float leftArm1;
	float incLeftArm1;

	//float rightArm2;
	//float incRightArm2;
	//float leftArm2;
	//float incLeftArm2;
	//float rightLeg2;
	//float incRightLeg2;
	//float leftLeg2;
	//float incLeftLeg2;
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;
bool play = false;
int playIndex = 0;

//Trineo
bool sledTrayectory = false;
bool trayectory1 = true;
bool trayectory2;
bool trayectory3;
bool trayectory4;
float rotSled;
bool subir;
//Lego 1
float movx = 0;
float movy = 0;
float movz = -130;
//Lego 2
float movLegoX, movLegoY = -7.3, movLegoZ = -40.05;
float rotLego = 90;
float caminar;
//Santa
float movSantaX;
float movSantaY;
float movSantaZ = 120;

// Positions of the point lights
glm::vec3 houseLights[] = {
	glm::vec3(40, 7.85, -23.2),
	glm::vec3(20, 7.85, -23.2),
	glm::vec3(0, 7.85, -33),


	glm::vec3(-21, 7.85, -20),
	glm::vec3(-21, 7.85, 0),
	glm::vec3(-21, 7.85, 20),

	glm::vec3(40, 7.85, 46),
	glm::vec3(20, 7.85, 46),
	glm::vec3(0, 7.85, 35),

	glm::vec3(59, 7.85, -10),
	glm::vec3(59, 7.85, 10),
	glm::vec3(59, 7.85, 30),
};

glm::vec3 treeLights[] = {
	glm::vec3(100, 17, -90),
	glm::vec3(85, 30, -80),
	glm::vec3(90, 15, -70),
	glm::vec3(100, 0, -90),
	glm::vec3(85, 5, -80),
	glm::vec3(90, 10, -70),

	glm::vec3(-70, 30, 85),
	glm::vec3(-70, 25, 80),
	glm::vec3(-85, 10, 70),
	glm::vec3(-85, 15, 85),
	glm::vec3(-80, 18, 80),
	glm::vec3(-80, 5, 70),
};

glm::vec3 LightP1;
glm::vec3 LightP2;
glm::vec3 LightP3;




void saveFrame(void)
{

	printf("frameindex %d\n", FrameIndex);

	KeyFrame[0].rightArm1 = 0;
	KeyFrame[1].rightArm1 = 45;
	KeyFrame[2].rightArm1 = 0;
	KeyFrame[0].leftArm1 = 0;
	KeyFrame[1].leftArm1 = 45;
	KeyFrame[2].leftArm1 = 0;

	//KeyFrame[FrameIndex].rightArm2 = rightArm2;
	//KeyFrame[FrameIndex].leftArm2 = leftArm2;
	//KeyFrame[FrameIndex].rightLeg2 = rightLeg2;
	//KeyFrame[FrameIndex].leftLeg2 = leftLeg2;

	FrameIndex = 3;
}

void resetElements(void)
{
	rightArm1 = KeyFrame[0].rightArm1;
	leftArm1 = KeyFrame[0].leftArm1;

	//rightArm2 = KeyFrame[0].rightArm2;
	//leftArm2 = KeyFrame[0].leftArm2;
	//rightLeg2 = KeyFrame[0].rightLeg2;
	//leftLeg2 = KeyFrame[0].leftLeg2;
}

void interpolation(void)
{
	KeyFrame[playIndex].incRightArm1 = (KeyFrame[playIndex + 1].rightArm1 - KeyFrame[playIndex].rightArm1) / i_max_steps;
	KeyFrame[playIndex].incLeftArm1 = (KeyFrame[playIndex + 1].leftArm1 - KeyFrame[playIndex].leftArm1) / i_max_steps;

	//KeyFrame[playIndex].incRightArm2 = (KeyFrame[playIndex + 1].rightArm2 - KeyFrame[playIndex].rightArm2) / i_max_steps;
	//KeyFrame[playIndex].incLeftArm2 = (KeyFrame[playIndex + 1].leftArm2 - KeyFrame[playIndex].leftArm2) / i_max_steps;
	//KeyFrame[playIndex].incRightLeg2 = (KeyFrame[playIndex + 1].rightLeg2 - KeyFrame[playIndex].rightLeg2) / i_max_steps;
	//KeyFrame[playIndex].incLeftLeg2 = (KeyFrame[playIndex + 1].leftLeg2 - KeyFrame[playIndex].leftLeg2) / i_max_steps;
}

void iluminacion(void)
{
	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	// Use cooresponding shader when setting uniforms/drawing objects
	lightingShader.Use();
	GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
	glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
	// Set material properties
	glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 10.0f);
	// Directional light
	glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.4f, 0.4f, 0.4f);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.4f, 0.4f, 0.4f);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

	/**************************************************************************************************************************
	*													Iluminación	casa													  *
	**************************************************************************************************************************/
	glm::vec3 lightColor1;
	lightColor1.x = abs(sin(glfwGetTime() * LightP1.x));
	lightColor1.y = abs(sin(glfwGetTime() * LightP1.y));
	lightColor1.z = sin(glfwGetTime() * LightP1.z);
	glm::vec3 lightColor2;
	lightColor2.x = abs(sin(glfwGetTime() * LightP2.x));
	lightColor2.y = abs(sin(glfwGetTime() * LightP2.y));
	lightColor2.z = sin(glfwGetTime() * LightP2.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), houseLights[0].x, houseLights[0].y, houseLights[0].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.032f);
	// Point light 2
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), houseLights[1].x, houseLights[1].y, houseLights[1].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.032f);
	// Point light 3
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"), houseLights[2].x, houseLights[2].y, houseLights[2].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.032f);
	// Point light 4
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].position"), houseLights[3].x, houseLights[3].y, houseLights[3].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].diffuse"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[3].specular"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[3].quadratic"), 0.032f);
	// Point light 5
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].position"), houseLights[4].x, houseLights[4].y, houseLights[4].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].diffuse"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[4].specular"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[4].quadratic"), 0.032f);
	// Point light 6
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].position"), houseLights[5].x, houseLights[5].y, houseLights[5].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].diffuse"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[5].specular"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[5].quadratic"), 0.032f);
	// Point light 7
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].position"), houseLights[6].x, houseLights[6].y, houseLights[6].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].diffuse"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[6].specular"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[6].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[6].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[6].quadratic"), 0.032f);
	// Point light 8
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[7].position"), houseLights[7].x, houseLights[7].y, houseLights[7].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[7].diffuse"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[7].specular"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[7].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[7].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[7].quadratic"), 0.032f);
	// Point light 9
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[8].position"), houseLights[8].x, houseLights[8].y, houseLights[8].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[8].diffuse"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[8].specular"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[8].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[8].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[8].quadratic"), 0.032f);
	// Point light 10
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[9].position"), houseLights[9].x, houseLights[9].y, houseLights[9].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[9].diffuse"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[9].specular"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[9].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[9].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[9].quadratic"), 0.032f);
	// Point light 11
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[10].position"), houseLights[10].x, houseLights[10].y, houseLights[10].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[10].diffuse"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[10].specular"), lightColor1.x, lightColor1.y, lightColor1.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[10].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[10].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[10].quadratic"), 0.032f);
	// Point light 12
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[11].position"), houseLights[11].x, houseLights[11].y, houseLights[11].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[11].diffuse"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[11].specular"), lightColor2.x, lightColor2.y, lightColor2.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[11].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[11].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[11].quadratic"), 0.032f);
	/**************************************************************************************************************************
	*													Iluminación	árboles													  *
	**************************************************************************************************************************/
	glm::vec3 lightColor3;
	lightColor3.x = abs(sin(glfwGetTime() * LightP3.x));
	lightColor3.y = abs(sin(glfwGetTime() * LightP3.y));
	lightColor3.z = sin(glfwGetTime() * LightP3.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[12].position"), treeLights[0].x, treeLights[0].y, treeLights[0].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[12].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[12].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[12].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[12].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[12].quadratic"), 0.032f);
	// Point light 2
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[13].position"), treeLights[1].x, treeLights[1].y, treeLights[1].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[13].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[13].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[13].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[13].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[13].quadratic"), 0.032f);
	// Point light 3
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[14].position"), treeLights[2].x, treeLights[2].y, treeLights[2].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[14].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[14].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[14].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[14].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[14].quadratic"), 0.032f);
	// Point light 4
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[15].position"), treeLights[3].x, treeLights[3].y, treeLights[3].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[15].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[15].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[15].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[15].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[15].quadratic"), 0.032f);
	// Point light 5
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[15].position"), treeLights[4].x, treeLights[4].y, treeLights[4].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[15].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[15].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[15].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[15].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[15].quadratic"), 0.032f);
	// Point light 6
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[16].position"), treeLights[5].x, treeLights[5].y, treeLights[5].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[16].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[16].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[16].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[16].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[16].quadratic"), 0.032f);
	// Point light 7
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[17].position"), treeLights[6].x, treeLights[6].y, treeLights[6].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[17].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[17].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[17].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[17].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[17].quadratic"), 0.032f);
	// Point light 8
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[18].position"), treeLights[7].x, treeLights[7].y, treeLights[7].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[18].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[18].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[18].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[18].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[18].quadratic"), 0.032f);
	// Point light 9
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[19].position"), treeLights[8].x, treeLights[8].y, treeLights[8].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[19].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[19].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[19].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[19].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[19].quadratic"), 0.032f);
	// Point light 10
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[20].position"), treeLights[9].x, treeLights[9].y, treeLights[9].z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[20].diffuse"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[20].specular"), lightColor3.x, lightColor3.y, lightColor3.z);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[20].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[20].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[20].quadratic"), 0.032f);

	// SpotLight
	glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);

	// Set material properties
	glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
}


int main()
{
	// Init GLFW
	glfwInit();

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	printf("%f", glfwGetTime());

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
	Shader SkyBoxshader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");

	//Model Trees((char*)"Models/Trees/trees.obj");
	Model House((char*)"Models/House/house.obj");
	//Model LivingRoom((char*)"Models/LivingRoom/livingRoom.obj");
	//Model LivingRoom2((char*)"Models/LivingRoom/livingRoom2.obj");
	//Model Kitchen((char*)"Models/Kitchen/kitchen.obj");
	//Model Bathroom((char*)"Models/Bathroom/bathroom.obj");
	//Model Room((char*)"Models/Room/room.obj");
	Model Present1((char*)"Models/Presents/present1.obj");
	//Model Present2((char*)"Models/Presents/present2.obj");
	Model Windows((char*)"Models/Windows/windows_.obj");
	//Model MailBox((char*)"Models/Mailbox/mailbox.obj");
	Model Sled((char*)"Models/Sled/sled.obj");
	Model hSanta((char*)"Models/Santa/headSanta.obj");
	Model bSanta((char*)"Models/Santa/bodySanta.obj");
	Model raSanta((char*)"Models/Santa/rightArmSanta.obj");
	Model laSanta((char*)"Models/Santa/leftArmSanta.obj");
	Model rlSanta((char*)"Models/Santa/rightLegSanta.obj");
	Model llSanta((char*)"Models/Santa/leftLegSanta.obj");
	//Model Sign((char*)"Models/Sign/sign.obj");
	//Model Bag((char*)"Models/Bag/sac.obj");
	//Model body((char*)"Models/Lego/body.obj");
	//Model head((char*)"Models/Lego/head.obj");
	//Model RightArm((char*)"Models/Lego/rightArm.obj");
	//Model LeftArm((char*)"Models/Lego/leftArm.obj");
	//Model RightLeg((char*)"Models/Lego/rightLeg.obj");
	//Model LeftLeg((char*)"Models/Lego/leftLeg.obj");


	//Inicializaci�n de KeyFrames

	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].rightArm1 = 0;
		KeyFrame[i].incRightArm1 = 0;
		KeyFrame[i].leftArm1 = 0;
		KeyFrame[i].incLeftArm1 = 0;

		//KeyFrame[i].rightArm2 = 0;
		//KeyFrame[i].incRightArm2 = 0;
		//KeyFrame[i].leftArm2 = 0;
		//KeyFrame[i].incLeftArm2 = 0;
		//KeyFrame[i].incRightLeg2 = 0;
		//KeyFrame[i].incRightLeg2 = 0;
		//KeyFrame[i].leftLeg2 = 0;
		//KeyFrame[i].incLeftLeg2 = 0;
	}



	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] =
	{
		// Positions            // Normals              // Texture Coords
		-0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,  	1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  0.0f,

		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  1.0f
	};

	GLuint indices[] =
	{  // Note that we start from 0!
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15,
		16,17,18,19,
		20,21,22,23,
		24,25,26,27,
		28,29,30,31,
		32,33,34,35
	};



	GLfloat skyboxVertices[] = {
		// Positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};


	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Texture Coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Set the vertex attributes (only position data for the lamp))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


	//SkyBox
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	// Load textures
	vector<const GLchar*> faces;
	faces.push_back("SkyBox/Winter/winter-skyboxes/IceLake/posx.jpg");
	faces.push_back("SkyBox/Winter/winter-skyboxes/IceLake/negx.jpg");
	faces.push_back("SkyBox/Winter/winter-skyboxes/IceLake/posy.jpg");
	faces.push_back("SkyBox/Winter/winter-skyboxes/IceLake/negy.jpg");
	faces.push_back("SkyBox/Winter/winter-skyboxes/IceLake/posz.jpg");
	faces.push_back("SkyBox/Winter/winter-skyboxes/IceLake/negz.jpg");


	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 1000.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		animacion();


		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		iluminacion(); //Función que se encarga de toda la iluminación


		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();
		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);


		glm::mat4 model(1);
		//Carga de modelo 
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		House.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Kitchen.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Room.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Bathroom.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//LivingRoom.Draw(lightingShader);


		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//LivingRoom2.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Presents.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Trees.Draw(lightingShader);


		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Sign.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//MailBox.Draw(lightingShader);

		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Bag.Draw(lightingShader);


		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(movx, movy, movz));
		//model = rotate(model, glm::radians(rotSled), glm::vec3(0,1,0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Sled.Draw(lightingShader);


		////Lego character1
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1+movx, 23.45+movy, movz));
		//model = glm::rotate(model, glm::radians(rotSled+90), glm::vec3(0, 1, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//body.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1 + movx, 23 + movy, movz));
		//model = glm::rotate(model, glm::radians(rotSled + 90), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0, 1.8, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//head.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1 + movx, 23 + movy, movz));
		//model = glm::rotate(model, glm::radians(rotSled + 90), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(-0.93, 1, -0.1));
		//model = glm::rotate(model, glm::radians(rightArm1), glm::vec3(0, 0, 1));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//RightArm.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1 + movx, 23 + movy, movz));
		//model = glm::rotate(model, glm::radians(rotSled + 90), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0.85, 1, -0.2));
		//model = glm::rotate(model, glm::radians(-70.0f), glm::vec3(1, 0, 0));
		//model = glm::rotate(model, glm::radians(leftArm1), glm::vec3(0, 1, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//LeftArm.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1 + movx, 23 + movy, movz));
		//model = glm::rotate(model, glm::radians(rotSled + 90), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(-0.1, -1.22, -0.09));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//RightLeg.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1 + movx, 23 + movy, movz));
		//model = glm::rotate(model, glm::radians(rotSled + 90), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0.1, -1.22, -0.05));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//LeftLeg.Draw(lightingShader);

		////Lego character2
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//body.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0.0, 1.3, 0.0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//head.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(-0.85, +0.8, -0.15));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//RightArm.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0.85, +0.6, -0.15));
		//model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1, 0, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//LeftArm.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(-0.15, -1.6, -0.15));
		//model = glm::rotate(model, glm::radians(caminar), glm::vec3(1, 0, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//RightLeg.Draw(lightingShader);

		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0.15, -1.6, -0.15));
		//model = glm::rotate(model, glm::radians(-caminar), glm::vec3(1, 0, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//LeftLeg.Draw(lightingShader);


		//model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(movLegoX, movLegoY, movLegoZ));
		//model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		//model = glm::translate(model, glm::vec3(0, 3.7, 0));
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Present1.Draw(lightingShader);


		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movSantaX, movSantaY, movSantaZ));
		model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(0.0, -1.3, 0.28));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		hSanta.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movSantaX, movSantaY, movSantaZ));
		model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(0.0, -4, 0.0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		bSanta.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movSantaX, movSantaY, movSantaZ));
		model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(-1.5, -2.73, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		laSanta.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movSantaX, movSantaY, movSantaZ));
		model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(1.5, -2.73, 0));
		model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1, 0, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		raSanta.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movSantaX, movSantaY, movSantaZ));
		model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(-0.8, -6.2, 0));
		model = glm::rotate(model, glm::radians(caminar), glm::vec3(1, 0, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		llSanta.Draw(lightingShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(movSantaX, movSantaY, movSantaZ));
		model = glm::rotate(model, glm::radians(rotLego), glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(0.8, -6.2, 0));
		model = glm::rotate(model, glm::radians(-caminar), glm::vec3(1, 0, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		rlSanta.Draw(lightingShader);

		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//model = glm::mat4(1);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//Windows.Draw(lightingShader);
		//glDisable(GL_BLEND);





		glBindVertexArray(0);


		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		glBindVertexArray(lightVAO);
		for (GLuint i = 0; i < 12; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, houseLights[i]);
			model = glm::scale(model, glm::vec3(0.3f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);

			model = glm::mat4(1);
			model = glm::translate(model, treeLights[i]);
			model = glm::scale(model, glm::vec3(0.3f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);


		// Draw skybox as last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		SkyBoxshader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
		glDisable(GL_BLEND);



		// Swap the screen buffers
		glfwSwapBuffers(window);
	}




	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();




	return 0;
}


void animacion()
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex >= FrameIndex)	//end of total animation?
			{
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			rightArm1 += KeyFrame[playIndex].incRightArm1*2;
			leftArm1 += KeyFrame[playIndex].incLeftArm1*2;

			//rightArm2 += KeyFrame[playIndex].incRightArm2;
			//leftArm2 += KeyFrame[playIndex].incLeftArm2;
			//rightLeg2 += KeyFrame[playIndex].incRightLeg2;
			//leftLeg2 += KeyFrame[playIndex].incLeftLeg2;
			i_curr_steps+=2;
		}
	}
	if (sledTrayectory)
	{
		if (movy < -20.0f)
			subir = true;
		else if (movy > 12.0f)
			subir = false;
		if (subir)
			movy += 0.1;
		else
			movy -= 0.1;
		caminar = sin(glfwGetTime() * 4) * 70;

		if (trayectory1)
		{
			if (movx == 160.0f)
			{
				trayectory1 = false;
				trayectory2 = true;
				rotSled = -90.0f;
				rotLego = 0.0f;
			}
			else
			{
				movx += 0.5f;
				movLegoX += 0.3f;
			}
		}

		if (trayectory2)
		{
			if (movz == 130)
			{
				trayectory2 = false;
				trayectory3 = true;
				rotSled = 180.0f;
				rotLego = -90.0f;
			}
			else
			{
				movz += 0.5f;
				movLegoZ += 0.3f;
			}
		}

		if (trayectory3)
		{
			if (movx == -140.0f)
			{
				trayectory3 = false;
				trayectory4 = true;
				rotSled = 90.0;
				rotLego = 180.0f;
			}
			else
			{
				movx -= 0.5f;
				movLegoX -= 0.3;
			}
		}

		if (trayectory4)
		{
			if (movz < -130.0f)
			{
				trayectory4 = false;
				trayectory1 = true;
				rotSled = 0.0f;
				rotLego = 0.0f;
			}
			else
			{
				movz -= 0.5f;
				movLegoZ -= 0.3f;
			}
		}
	} //Animación trineo.
}


// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (keys[GLFW_KEY_L])
	{
		saveFrame();

		if (play == false)// && (FrameIndex > 1))
		{

			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
		}
	}

	if (keys[GLFW_KEY_F])
		sledTrayectory = !sledTrayectory;

	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE]) //Activación luces.
	{
		active = !active;
		if (active)
		{
			LightP1 = glm::vec3(1.0f, 0.0f, 0.0f);
			LightP2 = glm::vec3(0.0f, 1.0f, 0.0f);
			LightP3 = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		else
		{
			LightP1 = glm::vec3(0.0f, 0.0f, 0.0f);
			LightP2 = glm::vec3(0.0f, 0.0f, 0.0f);
			LightP3 = glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{

	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime + 1);
	}
	if (keys[GLFW_KEY_S])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime + 1);
	}
	if (keys[GLFW_KEY_A])
	{
		camera.ProcessKeyboard(LEFT, deltaTime + 1);
	}
	if (keys[GLFW_KEY_D])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime + 1);
	}


	if (keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime + 0.05);
	}
	if (keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime + 0.05);
	}
	if (keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime + 0.05);
	}
	if (keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime + 0.05);
	}
}