#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

#include "GearEngine.hpp"
#include "EditorCamera.hpp"

// --- Custom Minimal GL Loader ---
// Since standard Windows gl.h only provides OpenGL 1.1, and ImGui's built-in loader 
// doesn't support instanced rendering, we load the exact 3.3 functions we need.
typedef ptrdiff_t GLsizeiptr;
typedef char GLchar;

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#endif

typedef void (*PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (*PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (*PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (*PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (*PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (*PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (*PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
typedef GLuint (*PFNGLCREATESHADERPROC) (GLenum type);
typedef void (*PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (*PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (*PFNGLCREATEPROGRAMPROC) (void);
typedef void (*PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (*PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (*PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (*PFNGLUSEPROGRAMPROC) (GLuint program);
typedef GLint (*PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (*PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (*PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (*PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (*PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (*PFNGLDELETEPROGRAMPROC) (GLuint program);
// --------------------------------

namespace gear_engine {

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 aInstanceMatrix; // Takes locations 1,2,3,4
layout (location = 5) in vec4 aInstanceColor;

uniform mat4 view;
uniform mat4 projection;

out vec4 vertexColor;
out vec2 TexCoords;
out float GearRadius;

void main()
{
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
    vertexColor = aInstanceColor;
    TexCoords = aPos.xy;
    GearRadius = length(aInstanceMatrix[0].xyz);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec2 TexCoords;
in float GearRadius;

void main()
{
    float r = length(TexCoords);
    if (r > 1.5) discard;
    
    float theta = atan(TexCoords.y, TexCoords.x);
    
    // Constant tooth size mathematically driven
    float teeth_density = 4.0; 
    float teeth_count = max(4.0, round(GearRadius * teeth_density));
    
    float tooth_depth = 0.2 / GearRadius; 
    
    // Square wave
    float wave = sign(sin(teeth_count * theta));
    float boundary = 1.0 + tooth_depth * wave;
    
    if (r > boundary) {
        discard;
    }
    
    // Axle hole
    float axle_radius = 0.2 / GearRadius;
    if (r < axle_radius) {
        discard;
    }
    
    FragColor = vertexColor;
}
)";

const char* lineVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

const char* lineFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 0.2, 0.2, 1.0); // Red line
}
)";

class DebugRenderer {
public:
    DebugRenderer() {
        LoadGLFunctions();
        CompileShaders();
        SetupCircleGeometry();
        SetupLineGeometry();
        
        instance_matrices_.reserve(kMaxBodies);
        instance_colors_.reserve(kMaxBodies);
    }

    ~DebugRenderer() {
        if (glDeleteVertexArrays) glDeleteVertexArrays(1, &circle_vao_);
        if (glDeleteBuffers) glDeleteBuffers(1, &circle_vbo_);
        if (glDeleteBuffers) glDeleteBuffers(1, &instance_matrix_vbo_);
        if (glDeleteBuffers) glDeleteBuffers(1, &instance_color_vbo_);
        if (glDeleteProgram) glDeleteProgram(shader_program_);
        
        if (glDeleteVertexArrays) glDeleteVertexArrays(1, &line_vao_);
        if (glDeleteBuffers) glDeleteBuffers(1, &line_vbo_);
        if (glDeleteProgram) glDeleteProgram(line_shader_program_);
    }

    void Draw(const EngineState& state, const ConstraintArrays& constraints, const EditorCamera& camera, int width, int height, int highlighted_gear = -1) {
        if (width == 0 || height == 0 || !glDrawArraysInstanced) return;
        
        instance_matrices_.clear();
        instance_colors_.clear();

        const RigidBodySoA& bodies = state.GetBodies();
        uint32_t cap = state.GetCapacity();

        for (uint32_t i = 1; i < cap; ++i) {
            if (!state.IsIndexActive(i)) continue;

            glm::mat4 model = glm::translate(glm::mat4(1.0f), bodies.positions[i]);
            model = model * glm::mat4_cast(bodies.rotations[i]);
            model = glm::scale(model, glm::vec3(bodies.radii[i]));
            
            instance_matrices_.push_back(model);
            if ((int)i == highlighted_gear) {
                instance_colors_.push_back(glm::vec4(1.0f, 1.0f, 0.2f, 1.0f)); // Yellow highlight
            } else {
                instance_colors_.push_back(glm::vec4(0.26f, 0.70f, 0.98f, 1.0f)); // Blue theme
            }
        }

        if (instance_matrices_.empty()) return;

        glUseProgram(shader_program_);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(circle_vao_);

        glBindBuffer(GL_ARRAY_BUFFER, instance_matrix_vbo_);
        glBufferData(GL_ARRAY_BUFFER, instance_matrices_.size() * sizeof(glm::mat4), instance_matrices_.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, instance_color_vbo_);
        glBufferData(GL_ARRAY_BUFFER, instance_colors_.size() * sizeof(glm::vec4), instance_colors_.data(), GL_DYNAMIC_DRAW);

        glDrawArraysInstanced(GL_TRIANGLES, 0, circle_vertex_count_, (GLsizei)instance_matrices_.size());
        
        glBindVertexArray(0);
        glUseProgram(0);

        // --- Draw Constraints (Lines) ---
        if (!constraints.gears.empty()) {
            std::vector<glm::vec3> line_vertices;
            for (const auto& gear : constraints.gears) {
                if (!state.IsHandleValid(gear.body_a) || !state.IsHandleValid(gear.body_b)) continue;
                line_vertices.push_back(bodies.positions[gear.body_a.index]);
                line_vertices.push_back(bodies.positions[gear.body_b.index]);
            }
            if (!line_vertices.empty()) {
                glUseProgram(line_shader_program_);
                glUniformMatrix4fv(glGetUniformLocation(line_shader_program_, "view"), 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(glGetUniformLocation(line_shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                
                glBindVertexArray(line_vao_);
                glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);
                glBufferData(GL_ARRAY_BUFFER, line_vertices.size() * sizeof(glm::vec3), line_vertices.data(), GL_DYNAMIC_DRAW);
                glDrawArrays(GL_LINES, 0, (GLsizei)line_vertices.size());
                
                glBindVertexArray(0);
                glUseProgram(0);
            }
        }
    }

    void DrawPreviewGear(glm::vec3 position, float radius, bool is_snapped, const EditorCamera& camera, int width, int height) {
        if (width == 0 || height == 0 || !glUseProgram) return;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::scale(model, glm::vec3(radius));

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

        glUseProgram(shader_program_);
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(circle_vao_);

        // Just use instance data buffer for 1 instance
        glm::vec4 color = is_snapped ? glm::vec4(0.2f, 1.0f, 0.2f, 0.8f) : glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
        
        glBindBuffer(GL_ARRAY_BUFFER, instance_matrix_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), glm::value_ptr(model), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, instance_color_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), glm::value_ptr(color), GL_DYNAMIC_DRAW);

        glDrawArraysInstanced(GL_TRIANGLES, 0, circle_vertex_count_, 1);
        
        glBindVertexArray(0);
        glUseProgram(0);
    }

private:
    void LoadGLFunctions() {
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glfwGetProcAddress("glGenVertexArrays");
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glfwGetProcAddress("glBindVertexArray");
        glGenBuffers = (PFNGLGENBUFFERSPROC)glfwGetProcAddress("glGenBuffers");
        glBindBuffer = (PFNGLBINDBUFFERPROC)glfwGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)glfwGetProcAddress("glBufferData");
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glfwGetProcAddress("glEnableVertexAttribArray");
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glfwGetProcAddress("glVertexAttribPointer");
        glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)glfwGetProcAddress("glVertexAttribDivisor");
        glCreateShader = (PFNGLCREATESHADERPROC)glfwGetProcAddress("glCreateShader");
        glShaderSource = (PFNGLSHADERSOURCEPROC)glfwGetProcAddress("glShaderSource");
        glCompileShader = (PFNGLCOMPILESHADERPROC)glfwGetProcAddress("glCompileShader");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)glfwGetProcAddress("glCreateProgram");
        glAttachShader = (PFNGLATTACHSHADERPROC)glfwGetProcAddress("glAttachShader");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)glfwGetProcAddress("glLinkProgram");
        glDeleteShader = (PFNGLDELETESHADERPROC)glfwGetProcAddress("glDeleteShader");
        glUseProgram = (PFNGLUSEPROGRAMPROC)glfwGetProcAddress("glUseProgram");
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glfwGetProcAddress("glGetUniformLocation");
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glfwGetProcAddress("glUniformMatrix4fv");
        glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)glfwGetProcAddress("glDrawArraysInstanced");
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glfwGetProcAddress("glDeleteVertexArrays");
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glfwGetProcAddress("glDeleteBuffers");
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glfwGetProcAddress("glDeleteProgram");
    }

    void CompileShaders() {
        if (!glCreateShader) return; // Safeguard if loader failed
        
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        
        shader_program_ = glCreateProgram();
        glAttachShader(shader_program_, vertexShader);
        glAttachShader(shader_program_, fragmentShader);
        glLinkProgram(shader_program_);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void SetupCircleGeometry() {
        if (!glGenVertexArrays) return;

        std::vector<glm::vec3> quad_vertices = {
            {-1.5f, -1.5f, 0.0f},
            { 1.5f, -1.5f, 0.0f},
            {-1.5f,  1.5f, 0.0f},
            { 1.5f,  1.5f, 0.0f},
            {-1.5f,  1.5f, 0.0f},
            { 1.5f, -1.5f, 0.0f}
        };
        circle_vertex_count_ = 6;

        glGenVertexArrays(1, &circle_vao_);
        glGenBuffers(1, &circle_vbo_);
        glGenBuffers(1, &instance_matrix_vbo_);
        glGenBuffers(1, &instance_color_vbo_);

        glBindVertexArray(circle_vao_);

        glBindBuffer(GL_ARRAY_BUFFER, circle_vbo_);
        glBufferData(GL_ARRAY_BUFFER, quad_vertices.size() * sizeof(glm::vec3), quad_vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, instance_matrix_vbo_);
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(1 + i);
            glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(1 + i, 1);
        }

        glBindBuffer(GL_ARRAY_BUFFER, instance_color_vbo_);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
    }

    void SetupLineGeometry() {
        if (!glGenVertexArrays) return;
        glGenVertexArrays(1, &line_vao_);
        glGenBuffers(1, &line_vbo_);
        
        glBindVertexArray(line_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBindVertexArray(0);
        
        // Compile Line Shader
        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &lineVertexShaderSource, NULL);
        glCompileShader(vShader);
        
        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &lineFragmentShaderSource, NULL);
        glCompileShader(fShader);
        
        line_shader_program_ = glCreateProgram();
        glAttachShader(line_shader_program_, vShader);
        glAttachShader(line_shader_program_, fShader);
        glLinkProgram(line_shader_program_);
        
        glDeleteShader(vShader);
        glDeleteShader(fShader);
    }

    // Function pointers
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
    PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
    PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
    PFNGLBUFFERDATAPROC glBufferData = nullptr;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
    PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = nullptr;
    PFNGLCREATESHADERPROC glCreateShader = nullptr;
    PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
    PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
    PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
    PFNGLATTACHSHADERPROC glAttachShader = nullptr;
    PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
    PFNGLDELETESHADERPROC glDeleteShader = nullptr;
    PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
    PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
    PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = nullptr;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
    PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
    // Basic DrawArrays (included in OpenGL 1.1) is used for lines, no need to load it dynamically.

    GLuint shader_program_ = 0;
    GLuint circle_vao_ = 0, circle_vbo_ = 0;
    GLuint instance_matrix_vbo_ = 0, instance_color_vbo_ = 0;
    int circle_vertex_count_ = 0;
    
    GLuint line_shader_program_ = 0;
    GLuint line_vao_ = 0, line_vbo_ = 0;

    std::vector<glm::mat4> instance_matrices_;
    std::vector<glm::vec4> instance_colors_;
};

} // namespace gear_engine
