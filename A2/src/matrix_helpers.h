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

using namespace std;
using namespace glm;



// 
// Helpers for looping through code (I use these on many C++ projects)
// 
        vector<int> range(int lower, int increment, int upper)
            {
                vector<int> result;
                if (increment > 0)
                    {
                        for (int i = lower; i <= upper; i += increment)
                            {
                                result.push_back(i);
                            }
                    }
                else
                    {
                        for (int i = lower; i >= upper; i += increment)
                            {
                                result.push_back(i);
                            }
                    }
                return result;
            }
        vector<int> range(int lower, int upper)
            {
                return range(lower, 1, upper);
            }
        vector<int> range(int upper)
            {
                return range(1, 1, upper);
            }

// 
// Matrix wrapper
//
// this helped me debug things using mat4's and do Lab3, and helped with conversions
const vector<float> SPECIAL_IDENTITY_ROW1 = {1,0,0,0};
const vector<float> SPECIAL_IDENTITY_ROW2 = {0,1,0,0};
const vector<float> SPECIAL_IDENTITY_ROW3 = {0,0,1,0};
const vector<float> SPECIAL_IDENTITY_ROW4 = {0,0,0,1};
struct Matrix4
    {
        // data
            vector<vector<float>> data = {   SPECIAL_IDENTITY_ROW1,   SPECIAL_IDENTITY_ROW2,   SPECIAL_IDENTITY_ROW3,   SPECIAL_IDENTITY_ROW4  };
        // constructors
            Matrix4()
                {}
            Matrix4(mat4 input)
                {
                    data.at(0).at(0) = *(float*)&input[0][0];
                    data.at(0).at(1) = *(float*)&input[0][1];
                    data.at(0).at(2) = *(float*)&input[0][2];
                    data.at(0).at(3) = *(float*)&input[0][3];
                    
                    data.at(1).at(0) = *(float*)&input[1][0];
                    data.at(1).at(1) = *(float*)&input[1][1];
                    data.at(1).at(2) = *(float*)&input[1][2];
                    data.at(1).at(3) = *(float*)&input[1][3];
                    
                    data.at(2).at(0) = *(float*)&input[2][0];
                    data.at(2).at(1) = *(float*)&input[2][1];
                    data.at(2).at(2) = *(float*)&input[2][2];
                    data.at(2).at(3) = *(float*)&input[2][3];
                    
                    data.at(3).at(0) = *(float*)&input[3][0];
                    data.at(3).at(1) = *(float*)&input[3][1];
                    data.at(3).at(2) = *(float*)&input[3][2];
                    data.at(3).at(3) = *(float*)&input[3][3];
                }
        // methods
            mat4 toMat4()
                {
                    mat4 result = mat4(              data.at(0).at(0), data.at(0).at(1), data.at(0).at(2), data.at(0).at(3),              
                                                     data.at(1).at(0), data.at(1).at(1), data.at(1).at(2), data.at(1).at(3),
                                                     data.at(2).at(0), data.at(2).at(1), data.at(2).at(2), data.at(2).at(3),              
                                                     data.at(3).at(0), data.at(3).at(1), data.at(3).at(2), data.at(3).at(3)         );
                    return result;
                }
        // overloads
            operator glm::mat4()
                {
                    return toMat4();
                }
            vector<float>& operator[] (long int position)
                {
                    if (position < 0 or position >= 4)
                        {
                            cerr << "Trying to access " << position << " on a Matrix4\n";
                            exit(0);
                        }
                    return data.at(position);
                }
    };
    ostream& operator<<(ostream& output_stream, Matrix4 input)
        {
            for (auto& each_row : input.data)
                {
                    for (auto& each_cell : each_row)
                        {
                            output_stream << each_cell << ", ";
                        }
                    output_stream << "\n";
                }
            return output_stream;
        }

// add output method for mat4
ostream& operator<<(ostream& output_stream, mat4 input)
    {
        Matrix4 helper(input);
        output_stream << helper;
        return output_stream;
    }