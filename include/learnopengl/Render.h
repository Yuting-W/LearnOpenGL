#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <texture/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include <iostream>
/*---------global value---------------*/
// settings
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
// camera
class Camera;
extern Camera camera;
extern float lastX;
extern float lastY;
extern bool firstMouse;

// timing
extern float deltaTime;
extern float lastFrame;


extern unsigned int cubeVAO;
extern unsigned int planeVAO;
/*--------function------------*/

extern unsigned int TextureFromFile(const char* path, const std::string& directory);
extern unsigned int LoadHDRTextureFromFile(const char* path, const std::string& directory);

extern unsigned int SetCubemapToFrame(unsigned int captureWidth, unsigned int capturesHeight);
extern void SetFramebuffer(unsigned int& captureFBO, unsigned int& captureRBO, unsigned int captureWidth, unsigned int capturesHeight);
extern void HDR2CubeAndIrradianceMap(const char* path, unsigned int& cubemap, unsigned int& irradianceMap);
//shadowing
extern void ShadowingSet(unsigned int captureWidth, unsigned int capturesHeight, unsigned int& depthMap, unsigned int& depthMapFBO);


extern void renderCube();
extern void renderPlane();
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
extern void processInput(GLFWwindow* window);

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
extern void framebuffer_size_callback(GLFWwindow* window, int width, int height);


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
extern void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
extern void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);