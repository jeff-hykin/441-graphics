#include <fcntl.h>               // functional progamming
#include <type_traits>           // functional progamming
#include <utility>               // functional progamming
#include <functional>            // functional progamming
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
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#define TINYOBJLOADER_IMPLEMENTATION
#include "GLSL.h"
#include "MatrixStack.h"
#include "tiny_obj_loader.h"
#include "matrix_helpers.h"

using namespace std;
using namespace glm;


// 
// Window
// 
// wrap up most of the gloabl objects and put them inside a singletion class for modulatrity
struct Window
    {
        // data
            GLFWwindow* glfw_window;
            GLuint              progID;
            map<string, GLint>  attrIDs;
            map<string, GLint>  unifIDs;
            map<string, GLuint> bufIDs;
            int                 indCount;
            bool                next_render_special = false;
            float               next_render_value = 0.5;
            MatrixStack MV;
        // methods
            void init(int width=640, int height=480, string name="", GLFWmonitor* monitor_ptr=NULL, GLFWwindow* window_ptr=NULL)
                {
                    glfw_window = glfwCreateWindow(width, height, name.c_str(), monitor_ptr, window_ptr);
                    if(!glfw_window)
                        {
                            glfwTerminate();
                            exit(0);
                        }
                    // Make the window's context current.
                    glfwMakeContextCurrent(glfw_window);
                }
            
            // draw needed to be on window since it has access to unifIDs
            void draw(mat4 a_matrix)
                {
                    if (next_render_special)
                        {
                            next_render_value = 0.5;
                        }
                    else
                        {
                            next_render_value = 1;
                        }
                    glUniform1fv(unifIDs["active"], 1, &next_render_value);
                    next_render_special = false;
                    glUniformMatrix4fv(unifIDs["MV"], 1, GL_FALSE, value_ptr(a_matrix));
                    glDrawArrays(GL_TRIANGLES, 0, indCount);
                }
    };
extern Window window;
Window window;


// 
// KeyMapperClass
// 
// this is for organizing and making key callbacks more readable/usable
struct KeyCallBack 
    {
        int which_key = -1;
    };
typedef KeyCallBack* KeyCallBackId;
struct KeyMapperClass
    {
        // sub classes
            struct Key
                {
                    bool is_pressed = false;
                    map<KeyCallBackId, function<void(void)>> on_press;
                    map<KeyCallBackId, function<void(void)>> on_release;
                };
        // data
            map<int, Key> keys;
            bool has_been_bound_already_for_this_frame = false;
            
        // constructors
            KeyMapperClass()
                {
                    // create all the keys (0-350)
                    for (int each : range(0,350))
                        {
                            keys[each] = Key();
                        }
                }
        // members
            // this is for brevity/convienience externally
            bool isPressed(int key_code)
                {
                    return keys[key_code].is_pressed;
                }
            KeyCallBackId onPress(int key_code, function<void(void)> a_function)
                {
                    // create new KeyCallBack
                    KeyCallBackId callback_identifier = new KeyCallBack();
                    callback_identifier->which_key = key_code;
                    // add the KeyCallBack to the callback map for that key
                    auto& key = keys[key_code];
                    key.on_press[callback_identifier] = a_function;
                    // return the identifier encase the user wants to detach/delete it
                    return callback_identifier;
                }
            KeyCallBackId onRelease(int key_code, function<void(void)> a_function)
                {
                    // create new KeyCallBack
                    KeyCallBackId callback_identifier = new KeyCallBack();
                    callback_identifier->which_key = key_code;
                    // add the KeyCallBack to the callback map for that key
                    auto& key = keys[key_code];
                    key.on_release[callback_identifier] = a_function;
                    // return the identifier encase the user wants to detach/delete it
                    return callback_identifier;
                }
            void deleteListener(KeyCallBackId callback_identifier)
                {
                    // un-bind the callback from the map of callbacks
                    auto& key = keys[callback_identifier->which_key];
                    // delete from onpress and onrelease (it has to be one of them)
                    try 
                        {
                            key.on_press.erase(callback_identifier);
                        }
                    catch (...) {}
                    try 
                        {
                            key.on_release.erase(callback_identifier);
                        }
                    catch (...) {}
                    // delete the reference to prevent memory leaks
                    delete callback_identifier;
                }
            void keepTrackOfKeyPresses(int& action, int& key_code)
                    {
                        auto& key = keys[key_code];
                        // keydown
                        if (action == GLFW_PRESS)
                            {
                                key.is_pressed = true;
                                for (auto& each_callback : key.on_press)
                                    {
                                        // run each of the callbacks
                                        each_callback.second();
                                    }
                            }
                        // keyup
                        else if (action == GLFW_RELEASE)
                            {
                                key.is_pressed = false;
                                for (auto& each_callback : key.on_release)
                                    {
                                        // run each of the callbacks
                                        each_callback.second();
                                    }
                            }
                    }
    };
extern KeyMapperClass key_manager; // declare
KeyMapperClass key_manager; // init


// 
// Renderables 
// 
// This is for inheriting to other classes for anything that needs to bind itself to the render function
struct Renderable
    {
        static void renderStart();
        Renderable() {};
        virtual void onRenderStart() {};
        virtual void render() {};
        virtual void onRenderEnd() {};
    };
    
struct RenderManager
    {
        vector<shared_ptr<Renderable>> renderables;
        void add(shared_ptr<Renderable> a_renderable_ptr)
            {
                renderables.push_back(a_renderable_ptr);
            }
        void renderStart()
            {
                // run all the render start functions
                for (auto& each : renderables)
                    {
                        each->onRenderStart();
                    }
            }
        void renderMain()
            {
                // run all the render functions
                for (auto& each : renderables)
                    {
                        each->render();
                    }
            }
        void renderEnd()
            {
                // run all the render end functions in reverse order
                for(vector<shared_ptr<Renderable>>::reverse_iterator iterator_index = renderables.rbegin(); iterator_index != renderables.rend(); ++iterator_index)
                    {
                        (*iterator_index)->onRenderEnd();
                    }
            }
    };


// 
// Component
// 
// this is for creating renderable cubes/objects 
struct Component : public Renderable
    {
        // data
            // scale, rotation, translation are all stored together inside of the transforms matrix
            Matrix4 transforms; // persistant memory of the transformations
            function<void(void)> on_render;
            vector<shared_ptr<Component>> children;
            bool transform_was_applied = false;
        // constuctors
            Component()
                {
                    
                }
            Component(std::initializer_list<shared_ptr<Component>> input_children)
                {
                    children = input_children;
                }
        // methods
            void transform()
                {
                    window.MV.multMatrix(transforms);
                    transform_was_applied = true;
                }
            void render() override
                {
                    transform_was_applied = false;
                    window.MV.pushMatrix();
                    // run the render function
                    on_render();
                    if (not transform_was_applied)
                        {
                            transform();
                        }
                    window.draw(window.MV.topMatrix());
                    // run the render function of each of the children
                    for (auto& each : children)
                        {
                            each->render();
                        }
                    window.MV.popMatrix();
                }
    };
    // create a helper for conveinience
    #define newCubeoid(...) shared_ptr<Component>(new Component({__VA_ARGS__}))


// 
// VertexShader
//
// summary:
//     this is a wrapper to make the code cleaner and more safe
//     if you don't care about safety and only want speed: then add #define NO_RUNTIME_CHECKS before including this file
struct VertexShader : public Renderable
    {
        // data
            GLuint id;
            string file_location;
            vector<GLuint> attached_programs;
            map<string, GLint> attribute_location_ids;
            // conditially add safety checks
            #ifndef NO_RUNTIME_CHECKS
                map<string, bool> data_has_been_sent_for;
            #endif
        // constructor 
            VertexShader() {};
        // member functions
            void loadFromFile(string location_of_shader_file)
                {
                    file_location = location_of_shader_file;
                    // create the shader
                    id = glCreateShader(GL_VERTEX_SHADER);
                    // get shader from file
                    const char* vShaderText = GLSL::textFileRead(file_location.c_str());
                    glShaderSource(id, 1, &vShaderText, NULL);
                    // Compile the shader
                    int rc;
                    glCompileShader(id);
                    glGetShaderiv(id, GL_COMPILE_STATUS, &rc);
                    if(!rc)
                        {
                            GLSL::printShaderInfoLog(id);
                            cerr << "Error compiling vertex shader " << file_location << endl;
                            exit(0);
                        }
                }
            void attachTo(GLuint program_id)
                {
                    // attach the shader to the program
                    glAttachShader(program_id, id);
                    attached_programs.push_back(program_id);
                }
            // call this inside init
            void addAttribute(string attribute_name)
                {
                    for (auto each_program : attached_programs)
                        {
                            attribute_location_ids[attribute_name]  = glGetAttribLocation(each_program, attribute_name.c_str());
                        }
                }
            // at the top of render
            void onRenderStart()
                {
                    // for each attribute, enable it
                    for (auto& key_value_pair : attribute_location_ids) 
                        {
                            glEnableVertexAttribArray(key_value_pair.second);
                            
                            // conditionally run safety checks
                            #ifndef NO_RUNTIME_CHECKS
                                data_has_been_sent_for[key_value_pair.first] = false;
                            #endif
                        }
                }
            // in the middle of render
            template <class ANYTYPE> 
            void sendData(string attribute_name, ANYTYPE& data)
                {
                    // conditionally run safety checks
                    #ifndef NO_RUNTIME_CHECKS
                        if (data_has_been_sent_for[attribute_name] == true)
                            {
                                cerr << "Durning render, there was an vertex shader attribute: " << attribute_name << " that was sent data twice\nmeaning vertex_shader.sendData() was probably called twice on it. (To fix this make sure it is only called once)\n";
                                exit(0);
                            }
                        data_has_been_sent_for[attribute_name] = true;
                    #endif
                    // send the data to the GPU
                    glVertexAttribPointer(attribute_location_ids[attribute_name], sizeof(data)/sizeof(GL_FLOAT), GL_FLOAT, false, 0, &data);
                }
            // at the end of render
            void onRenderEnd()
                {
                    // for each attribute, disable it
                    for (auto& key_value_pair : attribute_location_ids) 
                        {
                            glDisableVertexAttribArray(key_value_pair.second);
                            
                            // conditionally run safety checks
                            #ifndef NO_RUNTIME_CHECKS
                                if (data_has_been_sent_for[key_value_pair.first] == false)
                                    {
                                        cerr << "Durning render, there was an vertex shader attribute: " << key_value_pair.first << " that didnt get any data\nmeaning vertex_shader.sendData() was probably never called. (To fix this make sure it is called once)\n";
                                        exit(0);
                                    }
                            #endif
                        }
                }
    };


//
// FragmentShader
//
// summary:
//     this is a wrapper to make the code cleaner and more safe
//     if you don't care about safety and only want speed: then add #define NO_RUNTIME_CHECKS before including this file
struct FragmentShader
    {
        // data
            GLuint id;
            string file_location;
            vector<GLuint> attached_programs;
            map<string, GLint> attribute_location_ids;
            // conditially add safety checks
            #ifndef NO_RUNTIME_CHECKS
                map<string, bool> data_has_been_sent_for;
            #endif
        // member functions
            void loadFromFile(string location_of_shader_file)
                {
                    file_location = location_of_shader_file;
                    // Create shader handles
                    id = glCreateShader(GL_FRAGMENT_SHADER);

                    // Read shader sources
                    const char* fShaderText = GLSL::textFileRead(file_location.c_str());
                    glShaderSource(id, 1, &fShaderText, NULL);

                    // Compile fragment shader
                    int rc;
                    glCompileShader(id);
                    glGetShaderiv(id, GL_COMPILE_STATUS, &rc);
                    if(!rc)
                        {
                            GLSL::printShaderInfoLog(id);
                            cout << "Error compiling fragment shader " << file_location << endl;
                            return;
                        }
                }
            void attachTo(GLuint program_id)
                {
                    // attach the shader to the program
                    glAttachShader(program_id, id);
                    attached_programs.push_back(program_id);
                }
            // call this inside init
            void addAttribute(string attribute_name)
                {
                    for (auto each_program : attached_programs)
                        {
                            attribute_location_ids[attribute_name]  = glGetUniformLocation(each_program, attribute_name.c_str());
                        }
                }
            // at the top of render
            void onRenderStart()
                {
                    // for each attribute, enable it
                    for (auto& key_value_pair : attribute_location_ids) 
                        {
                            glEnableVertexAttribArray(key_value_pair.second);
                            
                            // conditionally run safety checks
                            #ifndef NO_RUNTIME_CHECKS
                                data_has_been_sent_for[key_value_pair.first] = false;
                            #endif
                        }
                }
            // in the middle of render
            template <class ANYTYPE> 
            void sendData(string attribute_name, ANYTYPE& data)
                {
                    // conditionally run safety checks
                    #ifndef NO_RUNTIME_CHECKS
                        if (data_has_been_sent_for[attribute_name] == true)
                            {
                                cerr << "Durning render, there was an vertex shader attribute: " << attribute_name << " that was sent data twice\nmeaning vertex_shader.sendData() was probably called twice on it. (To fix this make sure it is only called once)\n";
                                exit(0);
                            }
                        data_has_been_sent_for[attribute_name] = true;
                    #endif
                    // send the data to the GPU
                    glVertexAttribPointer(attribute_location_ids[attribute_name], sizeof(data)/sizeof(GL_FLOAT), GL_FLOAT, false, 0, &data);
                }
            // at the end of render
            void onRenderEnd()
                {
                    // for each attribute, disable it
                    for (auto& key_value_pair : attribute_location_ids) 
                        {
                            glDisableVertexAttribArray(key_value_pair.second);
                            
                            // conditionally run safety checks
                            #ifndef NO_RUNTIME_CHECKS
                                if (data_has_been_sent_for[key_value_pair.first] == false)
                                    {
                                        cerr << "Durning render, there was an vertex shader attribute: " << key_value_pair.first << " that didnt get any data\nmeaning vertex_shader.sendData() was probably never called. (To fix this make sure it is called once)\n";
                                        exit(0);
                                    }
                            #endif
                        }
                }
    };

