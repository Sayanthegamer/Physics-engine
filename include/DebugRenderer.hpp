#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "GearEngine.hpp"
#include "EditorCamera.hpp"
#include "GearMeshGenerator.hpp"

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

const char* meshVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* meshFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec4 objectColor;

void main()
{
    // Simple directional lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(norm, lightDir), 0.2); // ambient 0.2
    vec3 result = diff * objectColor.rgb;
    FragColor = vec4(result, objectColor.a);
}
)";

const char* gridVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 nearPoint;
out vec3 farPoint;

// Unproject screen coordinates to world coordinates
vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
    vec3 p = aPos;
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, view, projection);
    farPoint = UnprojectPoint(p.x, p.y, 1.0, view, projection);
    gl_Position = vec4(p, 1.0);
}
)";

const char* gridFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 nearPoint;
in vec3 farPoint;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    if (t < 0.0) discard;
    
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    
    // Grid pattern
    float scale = 1.0;
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    
    // Fade out in distance
    float linearDepth = length(fragPos3D - inverse(view)[3].xyz);
    float fading = max(0.0, (100.0 - linearDepth) / 100.0);
    
    vec4 color = vec4(0.8, 0.8, 0.8, 1.0 - min(line, 1.0));
    color.a *= fading * 0.5; // Overall transparency
    
    if (color.a < 0.01) discard;
    
    FragColor = color;
    
    // Calculate exact depth so it interacts with 3D objects properly
    vec4 clip_space_pos = projection * view * vec4(fragPos3D, 1.0);
    gl_FragDepth = (clip_space_pos.z / clip_space_pos.w) * 0.5 + 0.5;
}
)";

class DebugRenderer {
public:
    DebugRenderer() {
        LoadGLFunctions();
        CompileShaders();
        SetupCircleGeometry();
        SetupLineGeometry();
        SetupGridGeometry();
        
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
        
        if (glDeleteVertexArrays) glDeleteVertexArrays(1, &grid_vao_);
        if (glDeleteBuffers) glDeleteBuffers(1, &grid_vbo_);
        if (glDeleteProgram) glDeleteProgram(grid_shader_program_);
        
        if (glDeleteProgram) glDeleteProgram(mesh_shader_program_);
    }

    void Draw(const EngineState& state, const ConstraintArrays& constraints, const EditorCamera& camera, int width, int height, int highlighted_gear = -1) {
        if (width == 0 || height == 0 || !glUseProgram) return;

        const RigidBodySoA& bodies = state.GetBodies();
        uint32_t cap = state.GetCapacity();

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

        // Draw grid first (with depth testing enabled but writing usually doesn't matter for transparent grid, 
        // though our grid shader writes gl_FragDepth so it works anywhere).
        // Actually best to draw opaque objects first, then grid for transparency.
        
        for (uint32_t i = 1; i < cap; ++i) {
            if (!state.IsIndexActive(i)) continue;

            glm::mat4 model = glm::translate(glm::mat4(1.0f), bodies.positions[i]);
            model = model * glm::mat4_cast(bodies.rotations[i]);
            // Do scale by radius, GearMeshGenerator meshes are cached at radius 1.0f.
            model = glm::scale(model, glm::vec3(bodies.radii[i]));
            
            glm::vec4 color = ((int)i == highlighted_gear) ? glm::vec4(1.0f, 1.0f, 0.2f, 1.0f) : glm::vec4(0.26f, 0.70f, 0.98f, 1.0f);
            
            int teeth_count = std::max(4, (int)std::round(bodies.radii[i] * 4.0f));
            
            DrawCachedMesh(teeth_count, model, color, camera, width, height);
        }

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

    void DrawGrid(const EditorCamera& camera, int width, int height) {
        if (width == 0 || height == 0 || !glUseProgram) return;
        
        // We need depth testing but disable depth writing for the transparent grid
        // to blend properly with the opaque geometry behind it (which we already drew).
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Disable writing to depth buffer

        glUseProgram(grid_shader_program_);
        
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

        glUniformMatrix4fv(glGetUniformLocation(grid_shader_program_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(grid_shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(grid_vao_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);
        
        glDepthMask(GL_TRUE); // Re-enable depth writing
        glDisable(GL_BLEND);
    }

    void DrawFloor(const EditorCamera& camera, int width, int height) {
        if (width == 0 || height == 0 || !glUseProgram) return;

        glUseProgram(mesh_shader_program_);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

        // Floor at y = -0.5 to not perfectly overlap with z-fighting on grid (which is at y=0)
        // Wait, gears are at y=0. So floor should be slightly below, e.g. -0.05
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.05f, 0.0f));
        model = glm::scale(model, glm::vec3(50.0f)); // 50x50 floor

        glUniformMatrix4fv(glGetUniformLocation(mesh_shader_program_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(mesh_shader_program_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mesh_shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::vec4 floorColor(0.2f, 0.2f, 0.25f, 1.0f); // Dark slate floor
        glUniform4fv(glGetUniformLocation(mesh_shader_program_, "objectColor"), 1, glm::value_ptr(floorColor));

        glBindVertexArray(grid_vao_); // Reuse the quad geometry
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glUseProgram(0);
    }

    void DrawPreviewGear(glm::vec3 position, float radius, bool is_snapped, const EditorCamera& camera, int width, int height) {
        if (width == 0 || height == 0 || !glUseProgram) return;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::scale(model, glm::vec3(radius));
        
        glm::vec4 color = is_snapped ? glm::vec4(0.2f, 1.0f, 0.2f, 0.8f) : glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
        int teeth_count = std::max(4, (int)std::round(radius * 4.0f));
        
        DrawCachedMesh(teeth_count, model, color, camera, width, height);
    }

    void DrawCachedMesh(int teeth_count, const glm::mat4& model, const glm::vec4& color, const EditorCamera& camera, int width, int height) {
        if (width == 0 || height == 0 || !glUseProgram) return;

        auto it = mesh_cache_.find(teeth_count);
        if (it == mesh_cache_.end()) {
            GearMeshGenerator gen;
            MeshData mesh = gen.Generate(1.0f, teeth_count);
            
            CachedMesh gl_mesh;
            glGenVertexArrays(1, &gl_mesh.vao);
            glGenBuffers(1, &gl_mesh.vbo);
            glGenBuffers(1, &gl_mesh.ebo);

            glBindVertexArray(gl_mesh.vao);

            glBindBuffer(GL_ARRAY_BUFFER, gl_mesh.vbo);
            glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(0x8893, gl_mesh.ebo);
            glBufferData(0x8893, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

            glBindVertexArray(0);
            
            gl_mesh.index_count = (GLsizei)mesh.indices.size();
            mesh_cache_[teeth_count] = gl_mesh;
            it = mesh_cache_.find(teeth_count);
        }

        const CachedMesh& gl_mesh = it->second;

        glUseProgram(mesh_shader_program_);
        
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = camera.GetProjectionMatrix((float)width / (float)height);

        glUniformMatrix4fv(glGetUniformLocation(mesh_shader_program_, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(mesh_shader_program_, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mesh_shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glUniform4fv(glGetUniformLocation(mesh_shader_program_, "objectColor"), 1, glm::value_ptr(color));

        glBindVertexArray(gl_mesh.vao);
        glDrawElements(GL_TRIANGLES, gl_mesh.index_count, GL_UNSIGNED_INT, 0);

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
        glUniform4fv = (PFNGLUNIFORM4FVPROC)glfwGetProcAddress("glUniform4fv");
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
        
        // Compile Grid Shader
        GLuint gvShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(gvShader, 1, &gridVertexShaderSource, NULL);
        glCompileShader(gvShader);
        
        GLuint gfShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(gfShader, 1, &gridFragmentShaderSource, NULL);
        glCompileShader(gfShader);
        
        grid_shader_program_ = glCreateProgram();
        glAttachShader(grid_shader_program_, gvShader);
        glAttachShader(grid_shader_program_, gfShader);
        glLinkProgram(grid_shader_program_);
        
        glDeleteShader(gvShader);
        glDeleteShader(gfShader);
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
        
        // Compile Mesh Shader
        GLuint mvShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(mvShader, 1, &meshVertexShaderSource, NULL);
        glCompileShader(mvShader);
        
        GLuint mfShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(mfShader, 1, &meshFragmentShaderSource, NULL);
        glCompileShader(mfShader);
        
        mesh_shader_program_ = glCreateProgram();
        glAttachShader(mesh_shader_program_, mvShader);
        glAttachShader(mesh_shader_program_, mfShader);
        glLinkProgram(mesh_shader_program_);
        
        glDeleteShader(mvShader);
        glDeleteShader(mfShader);
    }

    void SetupGridGeometry() {
        if (!glGenVertexArrays) return;
        
        // Full screen quad
        std::vector<glm::vec3> grid_vertices = {
            {-1.0f, 0.0f, -1.0f},
            { 1.0f, 0.0f, -1.0f},
            {-1.0f, 0.0f,  1.0f},
            { 1.0f, 0.0f,  1.0f},
            {-1.0f, 0.0f,  1.0f},
            { 1.0f, 0.0f, -1.0f}
        };

        glGenVertexArrays(1, &grid_vao_);
        glGenBuffers(1, &grid_vbo_);
        
        glBindVertexArray(grid_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, grid_vbo_);
        glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(glm::vec3), grid_vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        
        // Setup normals for the floor quad to be pointing UP
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // We'll just disable normal array when drawing grid/floor, or provide dummy data. 
        // Actually, mesh shader uses location 1 for normals. Since we reuse grid_vao_ for floor (which uses mesh shader),
        // we should provide normal data.
        
        std::vector<glm::vec3> grid_normals = {
            {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
        };
        
        GLuint grid_normal_vbo;
        glGenBuffers(1, &grid_normal_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, grid_normal_vbo);
        glBufferData(GL_ARRAY_BUFFER, grid_normals.size() * sizeof(glm::vec3), grid_normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        
        glBindVertexArray(0);
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
    typedef void (*PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
    PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = nullptr;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
    PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
    // Basic DrawArrays (included in OpenGL 1.1) is used for lines, no need to load it dynamically.

    GLuint shader_program_ = 0;
    GLuint mesh_shader_program_ = 0;
    GLuint circle_vao_ = 0, circle_vbo_ = 0;
    GLuint instance_matrix_vbo_ = 0, instance_color_vbo_ = 0;
    int circle_vertex_count_ = 0;
    
    GLuint grid_shader_program_ = 0;
    GLuint grid_vao_ = 0, grid_vbo_ = 0;
    
    GLuint line_shader_program_ = 0;
    GLuint line_vao_ = 0, line_vbo_ = 0;

    std::vector<glm::mat4> instance_matrices_;
    std::vector<glm::vec4> instance_colors_;
    
    struct CachedMesh {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLsizei index_count = 0;
    };
    std::unordered_map<int, CachedMesh> mesh_cache_;
};

} // namespace gear_engine
