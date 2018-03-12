//
//  layout.hpp
//
//  Created by ISHII 2bit on 2018/02/21.
//

#pragma once

#ifndef bbb_layout_hpp
#define bbb_layout_hpp

namespace bbb {
    namespace view_system {
        namespace layout {
            struct margin {
                inline margin(const margin &) = default;
                inline margin(float top, float right, float bottom, float left)
                : top(top)
                , right(right)
                , bottom(bottom)
                , left(left) {};
                inline margin(float top, float h, float bottom)
                : margin(top, h, bottom, h) {};
                inline margin(float v, float h)
                : margin(v, h, v, h) {};
                inline margin(float m = 0.0f)
                : margin(m, m, m, m) {};
                
                inline margin &operator=(const margin &) = default;
                inline margin &operator=(float m) { return set(m); };
                
                inline margin &set(const margin &m) { return operator=(m); };
                inline margin &set(float margin) { return set(margin, margin, margin, margin); };
                inline margin &set(float vertical, float horizontal) { return set(vertical, horizontal, vertical, horizontal); };
                inline margin &set(float top, float h, float bottom) { return set(top, h, bottom, h); };
                inline margin &set(float top, float right, float bottom, float left) {
                    this->top = top;
                    this->right = right;
                    this->bottom = bottom;
                    this->left = left;
                    return *this;
                };
                
                float top;
                float right;
                float bottom;
                float left;
            };
        };
    };
};

#endif /* bbb_layout_hpp */
