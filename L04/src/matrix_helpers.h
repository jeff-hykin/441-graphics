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
#include "helpers.h"

using namespace std;
using namespace glm;
using namespace helpers;

#define DEFAULT_MATRIX_VALUE f32, defaultp

// 
// Vector wrapper
// 
template <int ROWS=1>
struct Vector
    {
        vec<ROWS, DEFAULT_MATRIX_VALUE> to_v;
        float& operator[] (long int position)
            {
                if (position > ROWS) 
                    {
                        cerr << "Tried to access position " << position << " on a vector with only " << ROWS << " rows ";
                        exit(1);
                    }
                return to_v[position - 1];
            }
    };
    template <int ROWS>
    ostream& operator<<(ostream& output_stream, Vector<ROWS> input)
        {
            for (int each : range(ROWS))
                {
                    output_stream << input[each] << ", ";
                }
            return output_stream;
        }



// 
// Matrix wrapper for accessing sub elements safely
//
// (made this for lab3, but its not really needed after that)
template <int ROWS=1, int COLUMNS=1>
struct Matrix
    {
        mat<ROWS, COLUMNS, DEFAULT_MATRIX_VALUE> to_m;
        Vector<COLUMNS>& operator[] (long int position)
            {
                return *(Vector<COLUMNS>*)(&to_m[position-1]);
            }
    };
    template <int ROWS, int COLUMNS>
    ostream& operator<<(ostream& output_stream, Matrix<ROWS, COLUMNS> input)
        {
            for (int each : range(ROWS))
                {
                    output_stream << input[each] << "\n";
                }
            return output_stream;
        }



// add output method for matricies
template <int ROWS, int COLUMNS>
ostream& operator<<(ostream& output_stream, mat<ROWS, COLUMNS, DEFAULT_MATRIX_VALUE> input)
    {
        Matrix<ROWS, COLUMNS> helper;
        helper.to_m = input;
        output_stream << helper;
        return output_stream;
    }


struct Window
    {
        GLFWwindow* glfw_window;
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
    };
extern Window window;
Window window;

struct KeyMapperClass
    {
        // data
            map<int, bool> keys;
            bool has_been_bound_already_for_this_frame = false;
        // constructor
            KeyMapperClass()
                {
                    // set all the keys (0-350) to be un-pressed by default
                    for (int each : range(0,350))
                        {
                            keys[each] = false;
                        }
                }
        // members
        void keepTrackOfKeyPresses(int& action, int& key)
                {
                    // keydown
                    if (action == GLFW_PRESS)
                        {
                            keys[key] = true;
                        }
                    // keyup
                    else if (action == GLFW_RELEASE)
                        {
                            keys[key] = false;
                        }
                }
            void bindKeysTo(vec3& rotation, vec3& translation)
                {
                    if (keys[GLFW_KEY_UP           ] == true) { rotation.x    += 0.05; }
                    if (keys[GLFW_KEY_DOWN         ] == true) { rotation.x    -= 0.05; }
                    if (keys[GLFW_KEY_LEFT         ] == true) { rotation.y    += 0.05; }
                    if (keys[GLFW_KEY_RIGHT        ] == true) { rotation.y    -= 0.05; }
                    if (keys[GLFW_KEY_LEFT_BRACKET ] == true) { rotation.z    += 0.05; }
                    if (keys[GLFW_KEY_RIGHT_BRACKET] == true) { rotation.z    -= 0.05; }
                    if (keys[GLFW_KEY_A            ] == true) { translation.x += 0.05; }
                    if (keys[GLFW_KEY_D            ] == true) { translation.x -= 0.05; }
                    if (keys[GLFW_KEY_E            ] == true) { translation.y += 0.05; }
                    if (keys[GLFW_KEY_Z            ] == true) { translation.y -= 0.05; }
                    if (keys[GLFW_KEY_W            ] == true) { translation.z += 0.05; }
                    if (keys[GLFW_KEY_S            ] == true) { translation.z -= 0.05; }
                    // keep track of if the keys already bound
                    has_been_bound_already_for_this_frame = true;
                }
            // apply incremental changes to a matrix
            mat4 transformFromKeyPresses(const mat4& a_matrix)
                {
                    vec3 rotation;
                    vec3 translation;
                    if (keys[GLFW_KEY_UP           ] == true) { rotation.x    += 0.05; }
                    if (keys[GLFW_KEY_DOWN         ] == true) { rotation.x    -= 0.05; }
                    if (keys[GLFW_KEY_LEFT         ] == true) { rotation.y    += 0.05; }
                    if (keys[GLFW_KEY_RIGHT        ] == true) { rotation.y    -= 0.05; }
                    if (keys[GLFW_KEY_LEFT_BRACKET ] == true) { rotation.z    += 0.05; }
                    if (keys[GLFW_KEY_RIGHT_BRACKET] == true) { rotation.z    -= 0.05; }
                    if (keys[GLFW_KEY_A            ] == true) { translation.x += 0.05; }
                    if (keys[GLFW_KEY_D            ] == true) { translation.x -= 0.05; }
                    if (keys[GLFW_KEY_E            ] == true) { translation.y += 0.05; }
                    if (keys[GLFW_KEY_Z            ] == true) { translation.y -= 0.05; }
                    if (keys[GLFW_KEY_W            ] == true) { translation.z += 0.05; }
                    if (keys[GLFW_KEY_S            ] == true) { translation.z -= 0.05; }
                    
                    mat4 copy_of_matrix = a_matrix;
                    // apply translation
                    copy_of_matrix *= translate(mat4(1.0f), translation);
                    // apply rotation
                    copy_of_matrix *= rotate(mat4(1.0f), rotation.x, vec3(1,0,0));
                    copy_of_matrix *= rotate(mat4(1.0f), rotation.y, vec3(0,1,0));
                    copy_of_matrix *= rotate(mat4(1.0f), rotation.z, vec3(0,0,1));
                    
                    // keep track of if the keys already bound
                    has_been_bound_already_for_this_frame = true;
                    
                    // return the transformation matrix
                    return copy_of_matrix;
                }
    };
extern KeyMapperClass key_mapper; // declare
KeyMapperClass key_mapper; // init



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

extern MatrixStack MV;
MatrixStack MV;

struct Cubeiod : public Renderable
    {
        // data 
            mat4 transforms; // persistant memory of the transformations
            function<void(void)> on_render;
            vector<Cubeiod> children;
        // constuctors
            Cubeiod(function<void(void)> input_on_render)
                {
                    on_render = input_on_render;
                }
        // methods
            void render() override
                {
                    MV.pushMatrix();
                    // run the render function
                    on_render();
                    // run the render function of each of the children
                    for (auto& each : children)
                        {
                            each.render();
                        }
                    MV.popMatrix();
                }
    };
    // create a helper for making cubeoids
    #define Cubeoid(FUNC) shared_ptr<Cubeiod>(new Cubeiod([&]() FUNC ))


#define drawTheLetterA                                                                  \
    {                                                                                   \
        vec3 rotation_offset(0, -0.05, 0.95);                                           \
        vec3 translation_offset(0, 0, 0.0999987);                                       \
        MV.pushMatrix();                                                                \
                                                                                        \
        /* rotate it first */                                                           \
        MV.rotate(global_rotation.x + rotation_offset.x, vec3(1,0,0));                  \
        MV.rotate(global_rotation.y + rotation_offset.y, vec3(0,1,0));                  \
        MV.rotate(global_rotation.z + rotation_offset.z, vec3(0,0,1));                  \
        /* squish it */                                                                 \
        MV.scale(vec3(1, 0.2, 1));                                                      \
        /* translate it */                                                              \
        vec3 new_translation = global_translation + translation_offset;                 \
        MV.translate(new_translation);                                                  \
        /* draw it */                                                                   \
        draw(MV.topMatrix());                                                            \
        MV.popMatrix();                                                                 \
    }                                                                                   \






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

