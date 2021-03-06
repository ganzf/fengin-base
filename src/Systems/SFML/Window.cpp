//
// Created by arroganz on 1/1/18.
//

#include "Components/Window.hpp"
#include "Entities/Camera.hpp"
#include "Camera/Camera.hpp"
#include "Window.hpp"

namespace
{
    std::unordered_map<futils::WStyle, decltype(sf::Style::None)> styleToSfStyle =
            {
                    {futils::WStyle::None, sf::Style::None},
                    {futils::WStyle::Default, sf::Style::Default},
                    {futils::WStyle::Fullscreen, sf::Style::Fullscreen},
            };
}

namespace fengin::systems::SFMLSystems
{
    void Window::requireEvents()
    {
        addReaction<ClearWindow>([this](futils::IMediatorPacket &pkg) {
            auto &packet = EventManager::rebuild<ClearWindow>(pkg);
            auto &entity = packet.camera;
            auto &cam = entity->get<components::Camera>();
            auto window = cam.window;
            auto &winCompo = window->get<components::Window>();
            auto realWindow = _windows.at(&winCompo).win;
//            sf::Color color;
//            if (window->has<components::Color>()) {
//                color << window->get<components::Color>().color;
//                realWindow->clear(color);
//            }
        });
        addReaction<RequestWindow>([this](futils::IMediatorPacket &pkg){
            auto &request = EventManager::rebuild<RequestWindow>(pkg);
            ResponseWindow response;
            if (request.camera == nullptr) {
                response.camera = nullptr;
                response.window = _windows.begin()->second.win;
                events->send<ResponseWindow>(response);
                return;
            }
            auto &entity = request.camera;
            auto &cam = entity->get<components::Camera>();
            auto window = cam.window;
            auto &winCompo = window->get<components::Window>();
            response.camera = entity;
            response.window = _windows.at(&winCompo).win;
            events->send<ResponseWindow>(response);
        });
        addReaction<fengin::ComponentAttached<Component>>([this](futils::IMediatorPacket &pkg){
            auto components = entityManager->get<Component>();
            auto &packet = EventManager::rebuild<fengin::ComponentAttached<Component>>(pkg);
            for (auto &win: components)
            {
                if (win->title == packet.compo.title)
                    onNewWindow(*win);
            }
        });
        addReaction<fengin::ComponentDeleted<Component>>([this](futils::IMediatorPacket &pkg){
            auto components = entityManager->get<Component>();
            auto &packet = EventManager::rebuild<fengin::ComponentDeleted<Component>>(pkg);
            Component *toRemove = nullptr;
            for (auto pair: _windows)
            {
                auto win = pair.first;
                if (win->title == packet.compo.title) {
                    onWindowDestroyed(*win);
                    toRemove = win;
                }
            }
            _windows.erase(toRemove);
            std::cout << "Notification complete." << std::endl;
        });
    }

    void Window::onWindowDestroyed(Component &win)
    {
        if (_windows.find(&win) == _windows.end())
            return ;
        auto &real = _windows.at(&win);
        close(real);
        std::cout << " Closed window " << real.data->title << std::endl;
    }

    void Window::onNewWindow(Component &win)
    {
        auto &real = _windows[&win];
        real.data = &win;
        auto mode = sf::VideoMode::getDesktopMode();
        win.screenSize.w = mode.width;
        win.screenSize.h = mode.height;
    }

    void Window::init()
    {
        __init();
        phase = RUN;
        requireEvents();
    }

    void Window::open(RealWindow &real)
    {
        if (real.win != nullptr)
            return ;
        auto &data = *real.data;
        if (real.win == nullptr)
        {
            sf::ContextSettings settings;
            settings.depthBits = 24;
            settings.stencilBits = 8;
            settings.antialiasingLevel = 4;
            settings.attributeFlags = sf::ContextSettings::Core;
            settings.majorVersion = 4;
            settings.minorVersion = 5;
            real.win = new sf::Window(sf::VideoMode(data.size.w, data.size.h, 32), data.title, styleToSfStyle[real.data->style], settings);
            real.win->setActive();
            real.win->setFramerateLimit(60);
            if (real.win->isOpen()) {
                data.isOpen = true;
                data.isClose = false;
            }
            real.copy = *real.data;
        }
    }

    void Window::close(RealWindow &real)
    {
        if (real.win == nullptr)
            return ;
        auto &data = *real.data;
        if (real.win != nullptr)
        {
            real.win->close();
            data.isOpen = false;
            data.isClose = true;
        }
    }

    void Window::pollEvents(RealWindow &real)
    {
        if (real.win == nullptr)
            return ;

        sf::Event event;
        int count{0};

        while (real.win->pollEvent(event)) {
            events->send<sf::Event>(event);
            count++;
        }
    }

    void Window::updateTitle(RealWindow &real)
    {
        if (real.win == nullptr)
            return ;

        if (real.copy.title != real.data->title)
        {
            real.win->setTitle(real.data->title);
            real.copy.title = real.data->title;
        }
    }

    void Window::run(float)
    {
        switch (phase)
        {
            case INIT: return init();
            case RUN:
                for (auto &pair: _windows)
                {
                    auto &real = pair.second;
                    if (real.data->visible)
                        open(real);
                    else
                        close(real);
                    updateTitle(real);
                    pollEvents(real);
                }
                return ;
        }
    }
}