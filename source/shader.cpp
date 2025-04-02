#ifdef GL
#include "shader.h"

#ifdef PC
#elif defined(PORTMASTER)
#include <SDL_opengles2.h>
#include <glad/gles2.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logging.h"
#include <cstdio>
#include <dirent.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int width = 0;
int height = 0;
float scale = 0;

uint vbo, ebo, vao, shaderTexture, shaderProgram;

SDL_GLContext shaderContext;

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

std::string translateLine(std::string in) {

    std::size_t pos = in.find("out vec4");

    if (pos != std::string::npos)
        in.replace(pos, 8, "out highp vec4");

    // pos = in.find("attribute");

    // if(pos != std::string::npos)
    //     in.replace(pos, 9, "in");

    return in;
}

uint loadShaderFromFile(std::string path, int type) {
    std::string add = "";

#if defined(PORTMASTER)
    add += "#version 300 es\n\n";
#endif

    if (type == GL_VERTEX_SHADER) {
        add += "#define VERTEX\n\n";
    } else if (type == GL_FRAGMENT_SHADER) {
        add += "#define FRAGMENT\n\n";
    }

    std::ifstream t(path);
    std::stringstream buffer_in;

    buffer_in << t.rdbuf();

    std::string buffer_out;

    std::string line;
    while (getline(buffer_in, line)) {
        if (line.find("pragma") == std::string::npos) {
            buffer_out += translateLine(line) + "\n";
        }
    }

    std::string str = add + buffer_out;

    uint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char* const*)&str, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    return shader;
}

int getShader(int index) {
    std::vector<std::string> shaders = findShaders();

    if (shaders.size() == 0)
        return -2;

    if (index <= 0 || index > (int)shaders.size()) {
        return -1;
    }

    std::string shaderFile = shaders.at(index - 1);

    log("loading shader: " + shaderFile);

    uint vertexShader = loadShaderFromFile(shaderFile, GL_VERTEX_SHADER);
    uint fragmentShader = loadShaderFromFile(shaderFile, GL_FRAGMENT_SHADER);

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
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
    }
    return program;
}

void initShaders(SDL_Window* window, int index) {
    shaderContext = SDL_GL_CreateContext(window);

    if (shaderStatus == ShaderStatus::NOT_INITED) {
#ifdef PC
        int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
#else
        int version = gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress);
#endif

        printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version),
               GLAD_VERSION_MINOR(version));
        ;

        if (version == 0) {
            shaderStatus = ShaderStatus::NO_GL;
            return;
        }
    }

    glViewport(0, 0, width, height);

    shaderProgram = getShader(index);

    if (shaderProgram < 0) {
        shaderStatus = ShaderStatus::NO_SHADER_FILES;
        return;
    }

    int res_out = glGetUniformLocation(shaderProgram, "OutputSize");
    if (res_out == -1) {
        log("ERROR: OutputSize");
        shaderStatus = ShaderStatus::SHADER_FILE_ERROR;
    }

    glUniform2f(res_out, (float)width, (float)height);

    int res_in = glGetUniformLocation(shaderProgram, "InputSize");
    if (res_in == -1) {

        log("ERROR: InputSize");
        shaderStatus = ShaderStatus::SHADER_FILE_ERROR;
    }

    glUniform2f(res_in, (float)width, (float)height);

    int res_tex = glGetUniformLocation(shaderProgram, "TextureSize");
    if (res_tex == -1) {
        log("ERROR: TextureSize");
        shaderStatus = ShaderStatus::SHADER_FILE_ERROR;
    }

    glUniform2f(res_tex, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

    glUseProgram(shaderProgram);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    glGenTextures(1, &shaderTexture);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    int position = glGetAttribLocation(shaderProgram, "VertexCoord");
    if (position == -1) {
        log("ERROR: position");
        shaderStatus = ShaderStatus::SHADER_FILE_ERROR;
    }

    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(position);

    int texCoord = glGetAttribLocation(shaderProgram, "TexCoord");
    if (texCoord == -1) {
        log("ERROR: texcoord");
        shaderStatus = ShaderStatus::SHADER_FILE_ERROR;
    }

    glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(texCoord);

    glm::mat4 def = glm::mat4();
    unsigned int transformLoc =
        glGetUniformLocation(shaderProgram, "MVPMatrix");
    if (transformLoc == -1) {
        log("ERROR: MVP");
        shaderStatus = ShaderStatus::SHADER_FILE_ERROR;
    }

    // if (shaderStatus == ShaderStatus::SHADER_FILE_ERROR)
    //     return;

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(def));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shaderTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    shaderStatus = ShaderStatus::OK;
}

void drawWithShaders(SDL_Window* window, SDL_Surface* img,
                     bool shadersEnabled) {

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shaderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

    if (shadersEnabled) {
        glUseProgram(shaderProgram);
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);
}

void freeShaders() {
    if (shaderStatus == ShaderStatus::NOT_INITED)
        return;
    SDL_GL_DeleteContext(shaderContext);

    shaderStatus = ShaderStatus::NOT_INITED;
    if ((int)shaderStatus <= (int)ShaderStatus::NO_SHADER_FILES) {
        return;
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &shaderTexture);
    glDeleteProgram(shaderProgram);
}

void refreshShaderResolution(int w, int h, float s) {
    width = w;
    height = h;
    scale = s;

    glViewport(0, 0, width, height);

    int res_out = glGetUniformLocation(shaderProgram, "OutputSize");
    glUniform2f(res_out, (float)width, (float)height);

    int res_in = glGetUniformLocation(shaderProgram, "InputSize");
    glUniform2f(res_in, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

    int res_tex = glGetUniformLocation(shaderProgram, "TextureSize");
    glUniform2f(res_tex, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

    float offsetX = (1.0 - ((width / scale) / SCREEN_WIDTH)) / 2;
    float offsetY = (1.0 - ((height / scale) / SCREEN_HEIGHT)) / 2;

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

static bool hasSuffix(const std::string& s, const std::string& suffix) {
    return (s.size() >= suffix.size()) &&
           equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}

std::vector<std::string> findShaders() {
    std::string shaderPath = "assets/shaders";

    DIR* dir = opendir(shaderPath.c_str());
    if (!dir) {
        log("couldn't open dir");
        return {};
    }

    std::vector<std::string> shaderPaths;

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (hasSuffix(entry->d_name, ".glsl")) {
            shaderPaths.push_back(shaderPath + "/" + entry->d_name);
        }
    }

    closedir(dir);

    return shaderPaths;
}
#endif
