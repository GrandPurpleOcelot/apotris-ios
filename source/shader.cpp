#ifdef GL
#include "shader.h"

#include <GL/glew.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int width = 0;
int height = 0;
float scale = 0;

uint vbo, ebo, vao, texture, shaderProgram;

float vertices[] = {
    1.0f,  1.0f,  0.0f, 1.0f, 0.0f, // top right
    1.0f,  -1.0f, 0.0f, 1.0f, 1.0f, // bottom right
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // bottom left
    -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, // top left
};
uint indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

uint loadShaderFromFile(std::string path, int type) {
    std::string add = "#version 330 core\n\n";
    if (type == GL_VERTEX_SHADER) {
        add += "#define VERTEX\n\n";
    } else if (type == GL_FRAGMENT_SHADER) {
        add += "#define FRAGMENT\n\n";
    }

    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();

    std::string str = add + buffer.str();

    // std::cout << str << std::endl;

    uint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char* const*)&str, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    return shader;
}

uint getShader() {
    std::string shaderPath = "assets/shaders/crt/shaders/crt-lottes.glsl";
    uint vertexShader = loadShaderFromFile(shaderPath, GL_VERTEX_SHADER);
    uint fragmentShader = loadShaderFromFile(shaderPath, GL_FRAGMENT_SHADER);

    uint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
    }
    return program;
}

void initShaders(SDL_Window* window) {
    SDL_GLContext context = SDL_GL_CreateContext(window);

    glewInit();

    glViewport(0, 0, width, height);

    shaderProgram = getShader();

    int res_out = glGetUniformLocation(shaderProgram, "OutputSize");
    glUniform2f(res_out, (float)width, (float)height);

    int res_in = glGetUniformLocation(shaderProgram, "InputSize");
    glUniform2f(res_in, (float)width, (float)height);

    int res_tex = glGetUniformLocation(shaderProgram, "TextureSize");
    glUniform2f(res_tex, (float)512, (float)512);

    // glUseProgram(shaderProgram);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    int position = glGetAttribLocation(shaderProgram, "VertexCoord");
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)nullptr);
    glEnableVertexAttribArray(position);

    int texCoord = glGetAttribLocation(shaderProgram, "TexCoord");
    glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(texCoord);

    glm::mat4 def = glm::mat4();
    unsigned int transformLoc =
        glGetUniformLocation(shaderProgram, "MVPMatrix");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(def));

    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void drawWithShaders(SDL_Window* window, SDL_Surface* img,
                     bool shadersEnabled) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, img->pixels);

    if (shadersEnabled) {
        glUseProgram(shaderProgram);
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    SDL_GL_SwapWindow(window);
}

void freeShaders() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &texture);
}

void refreshShaderResolution(int w, int h, float s) {
    width = w;
    height = h;
    scale = s;

    glViewport(0, 0, width, height);

    int res_out = glGetUniformLocation(shaderProgram, "OutputSize");
    glUniform2f(res_out, (float)width, (float)height);

    int res_in = glGetUniformLocation(shaderProgram, "InputSize");
    glUniform2f(res_in, (float)512, (float)512);

    int res_tex = glGetUniformLocation(shaderProgram, "TextureSize");
    glUniform2f(res_tex, (float)512, (float)512);

    float offsetX = (1.0 - ((width / scale) / 512)) / 2;
    float offsetY = (1.0 - ((height / scale) / 512)) / 2;

    vertices[0 * 5 + 3] = 1.0 - offsetX; // top right
    vertices[0 * 5 + 4] = offsetY;

    vertices[1 * 5 + 3] = 1.0 - offsetX; // bottom right
    vertices[1 * 5 + 4] = 1.0 - offsetY;

    vertices[2 * 5 + 3] = offsetX; // bottom left
    vertices[2 * 5 + 4] = 1.0 - offsetY;

    vertices[3 * 5 + 3] = offsetX; // top left
    vertices[3 * 5 + 4] = offsetY;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}
#endif
