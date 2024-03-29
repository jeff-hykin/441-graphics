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

#include "classes.h"

using namespace std;
using namespace glm;



// 
// init
// 
    string      RESOURCE_DIR = "./"; // Where the resources are loaded from
    auto                vertex_shader = shared_ptr<VertexShader>(new VertexShader());
    FragmentShader      fragment_shader;
    RenderManager       render_manager;

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
            key_manager.keepTrackOfKeyPresses(action, key);
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
                // Create the program
                window.progID = glCreateProgram();
                // load the shaders
                vertex_shader->loadFromFile(RESOURCE_DIR + "vert.glsl");
                fragment_shader.loadFromFile(RESOURCE_DIR + "frag.glsl");
                // attach shaders
                vertex_shader->attachTo(window.progID);
                fragment_shader.attachTo(window.progID);
                int rc;
                glLinkProgram(window.progID);
                glGetProgramiv(window.progID, GL_LINK_STATUS, &rc);
                if(!rc)
                    {
                        GLSL::printProgramInfoLog(window.progID);
                        cout << "Error linking shaders " << vertex_shader->file_location << " and " << fragment_shader.file_location << endl;
                        return;
                    }
                
                // Get vertex attribute IDs
                window.attrIDs["aPos"] = glGetAttribLocation(window.progID, "aPos");
                window.attrIDs["aNor"] = glGetAttribLocation(window.progID, "aNor");

                // Get uniform IDs
                window.unifIDs["P"]  = glGetUniformLocation(window.progID, "P");
                window.unifIDs["MV"] = glGetUniformLocation(window.progID, "MV");
                window.unifIDs["active"] = glGetUniformLocation(window.progID, "active");

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
                window.indCount = posBuf.size() / 3; // number of indices to be rendered

                // Generate 2 buffer IDs and put them in the window.bufIDs map.
                GLuint tmp[2];
                glGenBuffers(2, tmp);
                window.bufIDs["bPos"] = tmp[0];
                window.bufIDs["bNor"] = tmp[1];

                glBindBuffer(GL_ARRAY_BUFFER, window.bufIDs["bPos"]);
                glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_STATIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, window.bufIDs["bNor"]);
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
                glfwGetFramebufferSize(window.glfw_window, &width, &height);
                float aspect = width / (float)height;

                // Set up projection matrix (camera intrinsics)
                mat4 P = perspective((float)(45.0 * M_PI / 180.0), aspect, 0.01f, 100.0f);

                // Tell OpenGL which GLSL program to use
                glUseProgram(window.progID);
                // Pass in the current projection matrix
                glUniformMatrix4fv(window.unifIDs["P"], 1, GL_FALSE, value_ptr(P));
                // Enable the attribute
                glEnableVertexAttribArray(window.attrIDs["aPos"]);
                // Enable the attribute
                glEnableVertexAttribArray(window.attrIDs["aNor"]);
                // Bind the position buffer object to make it the currently active buffer
                glBindBuffer(GL_ARRAY_BUFFER, window.bufIDs["bPos"]);
                // Set the pointer -- the data is already on the GPU
                glVertexAttribPointer(window.attrIDs["aPos"], 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                // Bind the color buffer object to make it the currently active buffer
                glBindBuffer(GL_ARRAY_BUFFER, window.bufIDs["bNor"]);
                // Set the pointer -- the data is already on the GPU
                glVertexAttribPointer(window.attrIDs["aNor"], 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                // setup the shader
                render_manager.renderStart();
                
                window.MV.pushMatrix();
                // reset the keybinding each frame
                key_manager.has_been_bound_already_for_this_frame = false;
            //
            // DO STUFF HERE
            //
                render_manager.renderMain();
            // 
            // END DO STUFF
            // 
                window.MV.popMatrix();

                // Unbind the buffer object
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // Disable the attribute
                glDisableVertexAttribArray(window.attrIDs["aNor"]);
                // Disable the attribute
                glDisableVertexAttribArray(window.attrIDs["aPos"]);
                // close the shader bindings
                render_manager.renderEnd();
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
        // 
        // Parse arguments
        // 
            if(argc < 2)
                {
                    cout << "Please specify the resource directory." << endl;
                    return 0;
                }
            RESOURCE_DIR = argv[1] + string("/");

        // 
        // Keybindings setup
        // 
            int active_element = 0;
            key_manager.onPress(GLFW_KEY_PERIOD, [&](){
                    active_element++;
                });
            key_manager.onPress(GLFW_KEY_COMMA, [&](){
                    if (active_element > 0)
                        {
                            active_element--;
                        }
                });
            auto map_keybindings_on_active_element = [&](shared_ptr<Component> object, int element_number){
                // when this is the active element then listen to the keybindings
                if (active_element == element_number)
                    {
                        bool shift_is_pressed = key_manager.isPressed(GLFW_KEY_LEFT_SHIFT) or key_manager.isPressed(GLFW_KEY_RIGHT_SHIFT);
                        int negative_if_shift = shift_is_pressed ? -1 : 1 ;
                        // scale
                        // x,y,z rotation
                        if (key_manager.isPressed(GLFW_KEY_X)) { object->transforms = rotate(object->transforms.toMat4(), negative_if_shift * 0.02f, vec3(1, 0, 0)); }
                        if (key_manager.isPressed(GLFW_KEY_Y)) { object->transforms = rotate(object->transforms.toMat4(), negative_if_shift * 0.02f, vec3(0, 1, 0)); }
                        if (key_manager.isPressed(GLFW_KEY_Z)) { object->transforms = rotate(object->transforms.toMat4(), negative_if_shift * 0.02f, vec3(0, 0, 1)); }
                        // translate
                        if (key_manager.isPressed(GLFW_KEY_UP          ))  { object->transforms = translate(object->transforms.toMat4(), vec3(    0,     0,  0.05));  }
                        if (key_manager.isPressed(GLFW_KEY_DOWN        ))  { object->transforms = translate(object->transforms.toMat4(), vec3(    0,     0, -0.05));  }
                        if (key_manager.isPressed(GLFW_KEY_RIGHT       ))  { object->transforms = translate(object->transforms.toMat4(), vec3( 0.05,     0,     0));  }
                        if (key_manager.isPressed(GLFW_KEY_LEFT        ))  { object->transforms = translate(object->transforms.toMat4(), vec3(-0.05,     0,     0));  }
                        window.next_render_special = true;
                    }
                // tell the object to update itself
                object->transform();
            };
        // 
        // Create Renderable Object heiracy
        // 
            shared_ptr<Component> torso, head, left_upper_arm, left_lower_arm, right_upper_arm, right_lower_arm, left_upper_leg, left_lower_leg, right_upper_leg, right_lower_leg;
            torso = newCubeoid(
                head = newCubeoid(),
                left_upper_arm = newCubeoid(
                    left_lower_arm = newCubeoid()
                ),
                right_upper_arm = newCubeoid(
                    right_lower_arm = newCubeoid()
                ),
                left_upper_leg = newCubeoid(
                    left_lower_leg = newCubeoid()
                ),
                right_upper_leg = newCubeoid(
                    right_lower_leg = newCubeoid()
                )
            );
        // 
        // Render functions
        // 
            torso->on_render = [&]()
                {
                    window.MV.translate(0,1,-5.5);
                    map_keybindings_on_active_element(torso, 0);
                };
                head->on_render = [&]()
                    {
                        window.MV.translate(0,0.5,0);
                        map_keybindings_on_active_element(head, 1);
                        window.MV.scale(0.5, 0.5, 0.5);
                        window.MV.translate(0,0.5,0);
                    };
                vec3 arm_scale           = vec3(0.3, 0.8, 0.3);
                vec3 inverse_arm_scale   = vec3(1/arm_scale.x, 1/arm_scale.y, 1/arm_scale.z);
                vec3 arm_offset          = vec3(0.65 , -0.5, 0);
                vec3 lower_arm_translate = vec3(0, -0.55 * arm_scale.y, 0 );
                left_upper_arm->on_render = [&]()
                    {
                        window.MV.translate(-0.5, 0.5,0);
                        map_keybindings_on_active_element(left_upper_arm, 2);
                        window.MV.scale(arm_scale);
                        window.MV.translate(-arm_offset.x, arm_offset.y, arm_offset.z);
                    };
                    left_lower_arm->on_render = [&]()
                        {
                            window.MV.scale(inverse_arm_scale);
                            window.MV.translate(lower_arm_translate);
                            map_keybindings_on_active_element(left_lower_arm, 3);
                            window.MV.scale(arm_scale);
                            window.MV.translate(lower_arm_translate);
                        };
                right_upper_arm->on_render = [&]()
                    {
                        window.MV.translate(0.5, 0.5,0);
                        map_keybindings_on_active_element(right_upper_arm, 4);
                        window.MV.scale(arm_scale);
                        window.MV.translate(arm_offset.x, arm_offset.y, arm_offset.z);
                    };
                    right_lower_arm->on_render = [&]()
                        {
                            window.MV.scale(inverse_arm_scale);
                            window.MV.translate(lower_arm_translate);
                            map_keybindings_on_active_element(right_lower_arm, 5);
                            window.MV.scale(arm_scale);
                            window.MV.translate(lower_arm_translate);
                        };
                vec3 leg_scale           = vec3(0.4, 1, 0.4);
                vec3 inverse_leg_scale   = vec3(1/leg_scale.x, 1/leg_scale.y, 1/leg_scale.z);
                vec3 leg_rotate_axis     = vec3(0.25, -0.5, 0);
                vec3 leg_offset          = vec3(0, -0.5, 0);
                vec3 lower_leg_translate = vec3(0, -0.5 * leg_scale.y, 0 );
                left_upper_leg->on_render = [&]()
                    {
                        window.MV.translate(-leg_rotate_axis.x, leg_rotate_axis.y, leg_rotate_axis.z);
                        map_keybindings_on_active_element(left_upper_leg, 6);
                        window.MV.scale(leg_scale);
                        window.MV.translate(leg_offset);
                    };
                    left_lower_leg->on_render = [&]()
                        {
                            window.MV.scale(inverse_leg_scale);
                            window.MV.translate(lower_leg_translate);
                            map_keybindings_on_active_element(left_lower_leg, 7);
                            window.MV.scale(leg_scale);
                            window.MV.translate(lower_leg_translate);
                        };
                right_upper_leg->on_render = [&]()
                    {
                        window.MV.translate(leg_rotate_axis.x, leg_rotate_axis.y, leg_rotate_axis.z);
                        map_keybindings_on_active_element(right_upper_leg, 8);
                        window.MV.scale(leg_scale);
                        window.MV.translate(leg_offset);
                    };
                    right_lower_leg->on_render = [&]()
                        {
                            window.MV.scale(inverse_leg_scale);
                            window.MV.translate(lower_leg_translate);
                            map_keybindings_on_active_element(right_lower_leg, 9);
                            window.MV.scale(leg_scale);
                            window.MV.translate(lower_leg_translate);
                        };
        // 
        // Attach all the Renderables
        // 
            render_manager.add(vertex_shader);
            render_manager.add(torso);
        // 
        // OpenGL window setup/run/close
        // 
            // Set error callback.
            glfwSetErrorCallback(error_callback);
            // Initialize the library.
            if(!glfwInit())
                {
                    return -1;
                }
            // Create a windowed mode window and its OpenGL context.
            window.glfw_window = glfwCreateWindow(640, 480, "Jeff Hykin", NULL, NULL);
            if(!window.glfw_window)
                {
                    glfwTerminate();
                    return -1;
                }
            // Make the window's context current.
            glfwMakeContextCurrent(window.glfw_window);
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
            glfwSetKeyCallback(window.glfw_window, key_callback);
            // Set the mouse call back.
            glfwSetMouseButtonCallback(window.glfw_window, mouse_callback);
            // Set the window resize call back.
            glfwSetFramebufferSizeCallback(window.glfw_window, resize_callback);
            // Initialize scene.
            init();
            // Loop until the user closes the window.
            while(!glfwWindowShouldClose(window.glfw_window))
                {
                    // Render scene.
                    render();
                    // Swap front and back buffers.
                    glfwSwapBuffers(window.glfw_window);
                    // Poll for and process events.
                    glfwPollEvents();
                }
            // Quit program.
            glfwDestroyWindow(window.glfw_window);
            glfwTerminate();
            return 0;
    }
