//
// Created by arroganz on 8/6/18.
//

#include <Systems/SFML/Window.hpp>
#include <utils/inputKeys.hpp>
#include <Components/Transform.hpp>
//#include <Systems/SFML/AssetLoader.hpp>
#include <SOIL.h>
#include "OpenGL.hpp"
#include "Camera.hpp"
#include "Entities/Camera.hpp"
#include <gtc/matrix_transform.hpp>
#include <Systems/SFML/Utils/models.hpp>

float _x = 4.0f;
float _y = 2.0f;
float _z = 0.0f;

namespace fengin::systems::SFMLSystems {
    void OpenGL::init() {
        __init();
        addReaction<futils::Keys>([&_x, &_y, &_z](futils::IMediatorPacket &pkg){
            auto &key = EventManager::rebuild<futils::Keys>(pkg);
            if (key == futils::Keys::E) {
                _x -= 1;
            }
            if (key == futils::Keys::R) {
                _x += 1;
            }
            if (key == futils::Keys::D) {
                _y -= 1;
            }
            if (key == futils::Keys::F) {
                _y += 1;
            }
            if (key == futils::Keys::C) {
                _z -= 1;
            }
            if (key == futils::Keys::V) {
                _z += 1;
            }
        });
        addReaction<ResponseWindow>([this](futils::IMediatorPacket &pkg){
            auto &packet = EventManager::rebuild<ResponseWindow>(pkg);
            if (!packet.window) {
                std::cerr << "No window received" << std::endl;
                return;
            }
            win = (packet.window);
            win->setActive();
            glewExperimental = GL_TRUE;
            if (glewInit() != GLEW_OK) {
                events->send<events::Shutdown>();
                std::cerr << "GlewInit Failed." << std::endl;
            } else {
                std::cout << "GlewInit ok" << std::endl;
            }
            phase++;
            sf::ContextSettings settings = win->getSettings();
            std::cout << "depth bits:\t" << settings.depthBits << std::endl;
//            std::cout << "stencil bits:\t" << settings.stencilBits << std::endl;
//            std::cout << "antialiasing level:\t" << settings.antialiasingLevel << std::endl;
//            std::cout << "version:\t" << settings.majorVersion << "." << settings.minorVersion << std::endl;
//            std::cout << "GL VERSION:\t" << glGetString(GL_VERSION) << std::endl;
//            std::cout << "GLSL VERSION:\t" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
//            std::cout << "GL EXTENSIONS:\t" << glGetString(GL_EXTENSIONS) << std::endl;
            this->loadAndCompileShaders();
            this->setupOpenGlVertices();
        });
//        addReaction<AssetsLoaded>([this](futils::IMediatorPacket &pkg){
//            const auto &packet = EventManager::rebuild<AssetsLoaded>(pkg);
//            for (auto &texture: *packet.textures) {
//                GLuint textureId = SOIL_load_OGL_texture(
//                        std::string("./resources/" + texture.first).c_str(),
//                                SOIL_LOAD_AUTO,
//                                SOIL_CREATE_NEW_ID,
//                                SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
//                        );
//
//                /* check for an error during the load process */
//                if( 0 == textureId )
//                {
//                    std::cerr << "SOIL loading error: " << SOIL_last_result() << std::endl;
//                } else {
//                    events->send<std::string>("Texture " + texture.first + " loaded into OpenGL with soil and id " + std::to_string(textureId));
//                    this->loadedGlTextures[texture.first] = textureId;
////                    Allocate 1 name for texture textureId
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                }
//                std::cout << "Received texture " << texture.first << " in openGL " << std::endl;
//            }
//        });
        events->send<RequestWindow>();
//        events->send<RequestAssets>();
    }

    void OpenGL::loadAndCompileShaders() {
        events->send<std::string>("Preparing shaders...");
        shaderProgram.loadShadersFromDir("./Shaders/vertexShaders/", utils::Shader::ShaderType::Vertex);
        shaderProgram.loadShadersFromDir("./Shaders/fragmentShaders/", utils::Shader::ShaderType::Fragment);
        if (shaderProgram.compile()) {
            events->send<std::string>("Compiled shaders successfully");
        }
        shaderProgram.use();
    }

    void OpenGL::setupOpenGlVertices() {
        win->setVerticalSyncEnabled(true);
        vao.gen();
        vbo.gen();
        auto model = utils::makeCube();
        vbo.set(model);
        auto colors = utils::makeCubeColors();
        vbo.setColors(colors);
    }

    void OpenGL::renderTile(vec3f pos, vec3f size, vec3f rot, const components::Box &box) {
        auto &camPos = cam->get<components::Transform>().position;
        const auto &camC = cam->get<fengin::components::Camera>();
        auto const zoom = camC.zoom;

        // Normal
//         glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) win->getSize().x / (float) win->getSize().y, 0.1f, 100.0f);
        // Ortho
        glm::mat4 Projection = glm::ortho(-_x, _x, -_y, _y, 0.0f, 100.0f); // In world coordinates
        glm::mat4 View = glm::lookAt(
                glm::vec3(camPos.x + 10, 10, camPos.y + 10), // Camera is at (4,3,3), in World Space
                glm::vec3(camPos.x, camPos.z, camPos.y), // and looks at the origin
                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
        glm::mat4 Model = glm::mat4(1.0f);
        Model = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.z, size.y));
        Model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x * 2 - camPos.x, pos.z * 2 - camPos.z, pos.y * 2 - camPos.y));
        glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
        GLuint MatrixID = glGetUniformLocation(shaderProgram.getId(), "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
        if (box.wireframe) {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        } else {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
        vbo.draw();

    }

    void OpenGL::render(float elapsed) {
        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LESS);
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        static auto timer = 0.0f;
        timer += elapsed;
        if (!this->win || !this->cam) {
            auto cams = entityManager->get<components::Camera>();
            if (cams.size() > 0) {
                cam = dynamic_cast<Camera *>(&cams[0]->getEntity());
            }
            return;
        }
        auto go = entityManager->get<fengin::components::GameObject>();
        for (auto &obj: go) {
            if (!obj->visible)
                continue;
            auto &tr = obj->getEntity().get<components::Transform>();
            if (obj->getEntity().has<components::Box>())
                renderTile(tr.position, tr.size, tr.rotation, obj->getEntity().get<components::Box>());
        }
        win->display();
    }
}
