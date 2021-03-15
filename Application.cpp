/*This code uses Modern OpenGL to render geometry,
    and uses an abstraction of the VertexBuffer, VertexArray, and Shader.
  The code is based on The Cherno OpenGL tutorial in https://youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2
*/
#include <GL/glew.h>       
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>  
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // to create the vertex array
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 600, "Rendering a square", NULL, NULL);

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    // Define the viewport dimensions
    GLCall(glViewport(0, 0, screenWidth, screenHeight));

    // Setup OpenGL options
    GLCall(glEnable(GL_DEPTH_TEST));

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);  //to synchronize the animation

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;
    {
        // include the texture, positions to be mapped for the 6 faces of the cube
        float positions[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, //back bottom left vertex
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f, //back bottom right vertex
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //back top right vertex
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, //back top left vertex
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f
        };

        unsigned int indices[] = {
            //Back Face
            0, 1, 2, // First Triangle
            2, 3, 0,  //Second Triangle
            //Front Face
            4, 5, 6,
            6, 7, 4,
            //Left Face
            8, 9, 10,
            10, 4, 8,
            //Right Face
            11, 2, 12,
            12, 13, 11,
            //Bottom face
            10, 14, 5,
            5, 4, 10,
            //Top face
            3, 2, 11,
            11, 15, 3
        };

        //defining how openGL is going to blend alpha
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));  //src alpha = 0; dest = 1 - 0 = 0



        VertexArray va; 
        VertexBuffer vb(positions, 16 * 5 * sizeof(float)); // expand the buffer to 4 elements per vertex

        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(2);  // adding to more attributes
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 36);

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);

        Texture texture("res/textures/texture0.png");
        texture.Bind();
        shader.SetUniform1i("u_Texture", 0);  //the slot is 0


        va.Unbind();
        vb.Unbind();    //GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
        ib.Unbind();    //GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        shader.UnBind();  //GLCall(glUseProgram(0));

        Renderer renderer;

        // to create the animation that change the color 
        float r = 0.0f;
        float increment = 0.05f;

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            renderer.Clear();  //GLCall(glClear(GL_COLOR_BUFFER_BIT));

            // use the shader and bind the buffer and ibo each time in case that the buffer change
            shader.Bind();   //GLCall(glUseProgram(shader));
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);   //GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

             // Create transformations
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = glm::mat4(1.0f);
            model = glm::rotate(model, (GLfloat)glfwGetTime() * 1.0f, glm::vec3(0.5f, 1.0f, 0.0f));
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

            shader.SetUniformMat4fv("model", model);
            shader.SetUniformMat4fv("view", view);
            shader.SetUniformMat4fv("projection", projection);
            renderer.Draw(va, ib, shader);

           
            //GLCall(glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr));

            //part of the animation
            if (r > 1.0f)
                increment = -0.05f;
            else if (r < 0.0f)
                increment = 0.05f;

            r += increment;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

    }
    glfwTerminate();
    return 0;
}







