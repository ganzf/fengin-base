//
// Created by arroganz on 1/12/18.
//


#pragma once

#include <Entities/GameObject.hpp>
# include "fengin-core/include/FenginCore.hpp"

namespace fengin::components {
    struct ListView : Component {
        std::string name;
        int offset{0};
        int size{0};
        bool reversed;
        bool fit{false};
        bool expand{false};
        bool shrink{false};
        float padding;
        futils::Align align;
        futils::VAlign valign;
        futils::Ordering order;
        std::vector<entities::GameObject *> content;
    };
}