#include <memory>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <cassert>
#include <cstring>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "GLSL.h"
#include "MatrixStack.h"
#include "tiny_obj_loader.h"

#include "matrix_helpers.h"

using namespace std;
using namespace glm;

// Modelview matrix: camera and world
MatrixStack MV;


// 
// init
// 

    GLFWwindow* window;              // Main application window
    string      RESOURCE_DIR = "./"; // Where the resources are loaded from

    GLuint              progID;
    map<string, GLint>  attrIDs;
    map<string, GLint>  unifIDs;
    map<string, GLuint> bufIDs;
    int                 indCount;
    vec3 global_rotation;
    vec3 global_translation(0,0,-2.5);

// 
// 
// Callbacks
// 
// 
    // This function is called when a GLFW error occurs
    static void error_callback(int error, const char* description)
        {
            cerr << description << endl;
        }

    // This function is called when a key is pressed
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            cout << "action = " << action << ", key = " << key << ", x = " << global_rotation.x << ", y = " << global_rotation.y << ", z = " << global_rotation.z << ", x = " << global_translation.x << ", y = " << global_translation.y << ", z = " << global_translation.z << "\n"; 
            key_mapper.keepTrackOfKeyPresses(action, key);
            // close on escape
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                {
                    glfwSetWindowShouldClose(window, GL_TRUE);
                }
        }

    // This function is called when the mouse is clicked
    static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
        {
            // Do nothing
        }

    // If the window is resized, capture the new size and reset the viewport
    static void resize_callback(GLFWwindow* window, int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    // This function is called once to initialize the scene and OpenGL
    static void init()
        {
            //
            // General setup
            //

                // Initialize time.
                glfwSetTime(0.0);

                // Set background color.
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                // Enable z-buffer test.
                glEnable(GL_DEPTH_TEST);

            //
            // GLSL program setup
            //

                // Create shader handles
                GLuint vShaderID = glCreateShader(GL_VERTEX_SHADER);
                GLuint fShaderID = glCreateShader(GL_FRAGMENT_SHADER);

                // Read shader sources
                string      vShaderName = RESOURCE_DIR + "vert.glsl";
                string      fShaderName = RESOURCE_DIR + "frag.glsl";
                const char* vShaderText = GLSL::textFileRead(vShaderName.c_str());
                const char* fShaderText = GLSL::textFileRead(fShaderName.c_str());
                glShaderSource(vShaderID, 1, &vShaderText, NULL);
                glShaderSource(fShaderID, 1, &fShaderText, NULL);

                // Compile vertex shader
                int rc;
                glCompileShader(vShaderID);
                glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &rc);
                if(!rc)
                    {
                        GLSL::printShaderInfoLog(vShaderID);
                        cout << "Error compiling vertex shader " << vShaderName << endl;
                        return;
                    }

                // Compile fragment shader
                glCompileShader(fShaderID);
                glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &rc);
                if(!rc)
                    {
                        GLSL::printShaderInfoLog(fShaderID);
                        cout << "Error compiling fragment shader " << fShaderName << endl;
                        return;
                    }

                // Create the program and link
                progID = glCreateProgram();
                glAttachShader(progID, vShaderID);
                glAttachShader(progID, fShaderID);
                glLinkProgram(progID);
                glGetProgramiv(progID, GL_LINK_STATUS, &rc);
                if(!rc)
                    {
                        GLSL::printProgramInfoLog(progID);
                        cout << "Error linking shaders " << vShaderName << " and " << fShaderName << endl;
                        return;
                    }
                // initilize the vertex shader
                vertex_shader.init(progID);
                
                // Get vertex attribute IDs
                attrIDs["aPos"] = glGetAttribLocation(progID, "aPos");
                attrIDs["aNor"] = glGetAttribLocation(progID, "aNor");

                // Get uniform IDs
                unifIDs["P"]  = glGetUniformLocation(progID, "P");
                unifIDs["MV"] = glGetUniformLocation(progID, "MV");

            //
            // Vertex buffer setup
            //

                // Load OBJ geometry
                vector<float> posBuf;
                vector<float> norBuf;
                // Some obj files contain material information.
                // We'll ignore them for this assignment.
                string                           meshName = RESOURCE_DIR + "cube.obj";
                tinyobj::attrib_t                attrib;
                std::vector<tinyobj::shape_t>    shapes;
                std::vector<tinyobj::material_t> materials;
                string                           errStr;
                rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
                if(!rc)
                    {
                        cerr << errStr << endl;
                    }
                else
                    {
                        // Some OBJ files have different indices for vertex positions, normals,
                        // and texture coordinates. For example, a cube corner vertex may have
                        // three different normals. Here, we are going to duplicate all such
                        // vertices.
                        // Loop over shapes
                        for(size_t s = 0; s < shapes.size(); s++)
                            {
                                // Loop over faces (polygons)
                                size_t index_offset = 0;
                                for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
                                    {
                                        size_t fv = shapes[s].mesh.num_face_vertices[f];
                                        // Loop over vertices in the face.
                                        for(size_t v = 0; v < fv; v++)
                                            {
                                                // access to vertex
                                                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                                                posBuf.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                                                posBuf.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                                                posBuf.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                                                if(!attrib.normals.empty())
                                                    {
                                                        norBuf.push_back(attrib.normals[3 * idx.normal_index + 0]);
                                                        norBuf.push_back(attrib.normals[3 * idx.normal_index + 1]);
                                                        norBuf.push_back(attrib.normals[3 * idx.normal_index + 2]);
                                                    }
                                            }
                                        index_offset += fv;
                                        // per-face material (IGNORE)
                                        shapes[s].mesh.material_ids[f];
                                    }
                            }
                    }
                indCount = posBuf.size() / 3; // number of indices to be rendered

                // Generate 2 buffer IDs and put them in the bufIDs map.
                GLuint tmp[2];
                glGenBuffers(2, tmp);
                bufIDs["bPos"] = tmp[0];
                bufIDs["bNor"] = tmp[1];

                glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
                glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_STATIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
                glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_STATIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, 0);

                assert(norBuf.size() == posBuf.size());

                GLSL::checkError(GET_FILE_LINE);
        }

    // This function is called every frame to draw the scene.
    static void render()
        {
            // 
            // Frame setup
            // 
                // Clear framebuffer.
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Get current frame buffer size.
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                float aspect = width / (float)height;

                // Set up projection matrix (camera intrinsics)
                mat4 P = perspective((float)(45.0 * M_PI / 180.0), aspect, 0.01f, 100.0f);

                // Tell OpenGL which GLSL program to use
                glUseProgram(progID);
                // Pass in the current projection matrix
                glUniformMatrix4fv(unifIDs["P"], 1, GL_FALSE, value_ptr(P));
                // Enable the attribute
                glEnableVertexAttribArray(attrIDs["aPos"]);
                // Enable the attribute
                glEnableVertexAttribArray(attrIDs["aNor"]);
                // Bind the position buffer object to make it the currently active buffer
                glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
                // Set the pointer -- the data is already on the GPU
                glVertexAttribPointer(attrIDs["aPos"], 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                // Bind the color buffer object to make it the currently active buffer
                glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
                // Set the pointer -- the data is already on the GPU
                glVertexAttribPointer(attrIDs["aNor"], 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                // setup the shader
                vertex_shader.onRenderStart();
                
                MV.pushMatrix();
                // reset the keybinding each frame
                key_mapper.has_been_bound_already_for_this_frame = false;
            //
            // DO STUFF HERE
            //
            
            
            
            
            
            // move the global object if  when keys are pressed down 
            if (not key_mapper.has_been_bound_already_for_this_frame)
                {
                    MV.topMatrix() *= key_mapper.transformFromKeyPresses(MV.topMatrix());
                }
            
            drawTheLetterA 
            
            // 
            // END DO STUFF
            // 
                MV.popMatrix();

                // Unbind the buffer object
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // Disable the attribute
                glDisableVertexAttribArray(attrIDs["aNor"]);
                // Disable the attribute
                glDisableVertexAttribArray(attrIDs["aPos"]);
                // close the shader bindings
                vertex_shader.onRenderEnd();
                // Unbind our GLSL program
                glUseProgram(0);

                GLSL::checkError(GET_FILE_LINE);
        }


// 
// 
// Main
// 
// 
int main(int argc, char** argv)
    {
        if(argc < 2)
            {
                cout << "Please specify the resource directory." << endl;
                return 0;
            }
        RESOURCE_DIR = argv[1] + string("/");

        // Set error callback.
        glfwSetErrorCallback(error_callback);
        // Initialize the library.
        if(!glfwInit())
            {
                return -1;
            }
        // Create a windowed mode window and its OpenGL context.
        window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
        if(!window)
            {
                glfwTerminate();
                return -1;
            }
        // Make the window's context current.
        glfwMakeContextCurrent(window);
        // Initialize GLEW.
        glewExperimental = true;
        if(glewInit() != GLEW_OK)
            {
                cerr << "Failed to initialize GLEW" << endl;
                return -1;
            }
        glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
        cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
        cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
        GLSL::checkVersion();
        // Set vsync.
        glfwSwapInterval(1);
        // Set keyboard callback.
        glfwSetKeyCallback(window, key_callback);
        // Set the mouse call back.
        glfwSetMouseButtonCallback(window, mouse_callback);
        // Set the window resize call back.
        glfwSetFramebufferSizeCallback(window, resize_callback);
        // Initialize scene.
        init();
        // Loop until the user closes the window.
        while(!glfwWindowShouldClose(window))
            {
                // Render scene.
                render();
                // Swap front and back buffers.
                glfwSwapBuffers(window);
                // Poll for and process events.
                glfwPollEvents();
            }
        // Quit program.
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
