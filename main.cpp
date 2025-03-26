#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <texture/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/shader_s.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include<learnopengl/Render.h>


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.
    // build and compile our shader zprogram
    // ------------------------------------
    Shader ModelShader("./shader/6.1.PHR.vs", "./shader/6.1.PHR.fs");
    
    Shader backgroundShader("./shader/6.2.background.vs", "./shader/6.2.background.fs");
    Shader planeShader("./shader/5.3.1.shadowed.vs", "./shader/5.3.1.shadowed.fs");

    

    ModelShader.use();
    
    ModelShader.setInt("texture_diffuse1", 0);
    ModelShader.setInt("texture_specular1", 1);
    ModelShader.setInt("irradianceMap", 2);
    ModelShader.setInt("shadowMap", 3);
    //ModelShader.setFloat("metallic", 0.5f);
    //ModelShader.setFloat("roughness", 0.5f);
    ModelShader.setFloat("ao", 1.0f);

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);

    planeShader.use();
    planeShader.setInt("diffuseTexture", 0);
    planeShader.setInt("shadowMap", 1);




    Model ourModel("./resources/objects/nanosuit/nanosuit.obj");
    // lights
    // ------
    // lighting
    //glm::vec3 lightPos(70,100,-15);
    glm::vec3 lightPos(100.f, 100.f, 50.f);
    glm::vec3 lightPositions[] = {
        glm::vec3(10.0f, 10.0f, 13.0f),
        glm::vec3(-10.0f, 10.0f, 13.0f),
        lightPos,
        lightPos,
    };
    glm::vec3 lightColors[] = {
        glm::vec3(20.0f,0.0f, 0.0f),
        glm::vec3(0.0f, 20.0f, 0.0f),
        glm::vec3(9000.0f, 9000.0f, 9000.0f),
        glm::vec3(0.0f, 0.0f, 0.0f)
    };
    float lightArea = 200.0f;
    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;
    unsigned int irradianceMap;
    unsigned int envCubemap;
    HDR2CubeAndIrradianceMap("./resources/hdrs/zwartkops_straight_afternoon_2k.hdr", envCubemap, irradianceMap);
    
    //HDR2CubeAndIrradianceMap("./resources/hdrs/meadow_4k.hdr", envCubemap, irradianceMap);
    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    //shadowing
    // --------------------------------------
    
    
    //shadowing set
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    Shader simpleDepthShader("./shader/5.3.1.shadow_mapping_depth.vs", "./shader/5.3.1.shadow_mapping_depth.fs");
    unsigned int depthMap, depthMapFBO;
    ShadowingSet(SHADOW_WIDTH, SHADOW_HEIGHT, depthMap, depthMapFBO);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 100.0f, far_plane = 200.5f;
        lightProjection = glm::ortho(-10.0f, 20.0f, -10.0f, 20.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::scale(model, glm::vec3(3.0, 3.0, 3.0));
        model = glm::translate(model, glm::vec3(0.0, 0.0, -10.0));
        model = glm::rotate(model, glm::radians(-0.f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(6.0, 0.0, -9.0));
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        simpleDepthShader.setMat4("model", model);
        ourModel.Draw(simpleDepthShader);
        //simpleDepthShader.setMat4("model", model2);
        //ourModel.Draw(simpleDepthShader);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
       
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //render model
        //-------------------------------------------------------
        ModelShader.use();
        //vs
        
        ModelShader.setMat4("model", model);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ModelShader.setMat4("projection", projection);
        ModelShader.setMat4("view", view);
        glm::mat3 NormalMat = glm::transpose(glm::inverse(glm::mat3(model)));
        ModelShader.setMat3("modelInvTrans", NormalMat);
        ModelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        //fs
        // bind pre-computed IBL data
        ModelShader.setVec3("DirectLightPos", lightPos);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        ModelShader.setVec3("camPos", camera.Position);
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]);  i++)
        {
            ModelShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            ModelShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }

        ourModel.Draw(ModelShader);
        //ModelShader.setMat4("model", model2);
        //ourModel.Draw(ModelShader);
        // render plane 
        //--
        planeShader.use();
        model = glm::mat4(1.0);

        float ftemp = 30.f;
        model = glm::translate(model, glm::vec3(5.0f, ftemp, 0.0f));
        model = glm::rotate(model,-20.f, glm::vec3(0.f,  1.f,  0.f));
        model = glm::scale(model, glm::vec3(ftemp));
        planeShader.setMat4("model", model);
        planeShader.setMat4("projection", projection);
        planeShader.setMat4("view", view);
        // set light uniforms
        planeShader.setVec3("viewPos", camera.Position);
        planeShader.setVec3("lightPos", lightPos);
        planeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        planeShader.setFloat("lightArea", lightArea);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderPlane();
        // render skybox (render as last to prevent overdraw)
        //----------------------------------------------------------
        backgroundShader.use();
        backgroundShader.setMat4("view", view);
        backgroundShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        
        renderCube();






        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

   
    }

   

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

