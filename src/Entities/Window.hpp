//
// Created by arroganz on 1/1/18.
//

#pragma once

# include "fengin-core/include/FenginCore.hpp"
# include "Components/Window.hpp"
# include "Components/Color.hpp"

namespace fengin::entities
{
    class Window : public Entity
    {
    public:
        Window() {
            attach<components::Window>();
            auto &bg = attach<components::Color>();
            bg.color = futils::Black;
        }

        Window(int width, int height, std::string const &title = "Untitled", futils::WStyle const &style = futils::WStyle::Default, futils::Color color = futils::Granite) {
            auto &win = attach<components::Window>();
            win.size.w = width;
            win.size.h = height;
            win.style = style;
            win.title = title;

            auto &colorCompo = attach<components::Color>();
            colorCompo.color = color;
        }

        ~Window()
        {
            detach<components::Window>();
        }

        void setSize(int width, int height) {
            auto &win = get<fengin::components::Window>();
            win.size.w = width;
            win.size.h = height;
        }

        void setStyle(futils::WStyle const &style) {
            auto &win = get<fengin::components::Window>();
            win.style = style;
        }

        void setVisible(bool visible) {
            auto &win = get<fengin::components::Window>();
            win.visible = visible;
        }

        void setTitle(std::string const &title) {
            auto &win = get<fengin::components::Window>();
            win.title = title;
        }

        template <typename WindowType>
        WindowType *getWindowAs() {
            auto &compo = get<components::Window>();
            return static_cast<WindowType *>(compo._actualWindow);
        }
    };
}
