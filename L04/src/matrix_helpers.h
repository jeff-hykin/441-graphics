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


// output
template <int ROWS, int COLUMNS>
ostream& operator<<(ostream& output_stream, mat<ROWS, COLUMNS, DEFAULT_MATRIX_VALUE> input)
    {
        Matrix<ROWS, COLUMNS> helper;
        helper.to_m = input;
        output_stream << helper;
        return output_stream;
    }

template <int SIZE>
mat<SIZE, SIZE, DEFAULT_MATRIX_VALUE> createIdentityMatrix()
    {
        Matrix<SIZE, SIZE> output;
        for (int each : range(SIZE))
            {
                output[each][each] = 1;
            }
        return output.to_m;
    }

mat4 createTranslationMatrix(vec3 const& translation)
    {
        Matrix<4,4> output;
        output.to_m = createIdentityMatrix<4>();
        return translate(output.to_m, translation);
    }
    

#define drawTheLetterA                                                                  \
    {                                                                                   \
        MV.pushMatrix();                                                                \
        /* squish it */                                                                 \
        if (keys[GLFW_KEY_LEFT_BRACKET ] == true) { rotation.z += 0.1; }                \
        if (keys[GLFW_KEY_RIGHT_BRACKET] == true) { rotation.z -= 0.1; }                \
        MV.rotate(rotation.z, vec3(0,0,1));                                             \
        MV.scale(vec3(1, 0.2, 1));                                                      \
                                                                                        \
        glUniformMatrix4fv(unifIDs["MV"], 1, GL_FALSE, value_ptr(MV.topMatrix()));      \
        glDrawArrays(GL_TRIANGLES, 0, indCount);                                        \
        MV.popMatrix();                                                                 \
    }                                                                                   \




