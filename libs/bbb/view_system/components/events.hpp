//
//  components/events.h
//
//  Created by ISHII 2bit on 2018/02/21.
//

#pragma once

#ifndef bbb_components_events_hpp
#define bbb_components_events_hpp

#include <functional>
#include "../opt_arg_function.hpp"
#include <memory>

#include "ofRectangle.h"
#include "ofPoint.h"

namespace bbb {
    namespace view_system {
        inline namespace components {
            using view_ref = std::shared_ptr<struct view>;
            
            struct event_arg {
                event_arg(view_ref target)
                : target(target) {};
                
                view_ref target;
            };
            struct mouse_event_arg : public event_arg {
                mouse_event_arg(view_ref target, ofPoint p, bool isInside)
                : event_arg(target)
                , p(p)
                , isInside(isInside) {};
                
                ofPoint p;
                bool isInside;
            };
            
            struct resized_event_arg : public event_arg {
                resized_event_arg(view_ref target, const ofRectangle &rect)
                : event_arg(target)
                , rect(rect) {};
                
                ofRectangle rect;
            };
            
            using click_down_callback_t = bbb::opt_arg_function<void(mouse_event_arg)>;
            using click_up_callback_t = bbb::opt_arg_function<void(mouse_event_arg)>;
            using mouse_over_callback_t = bbb::opt_arg_function<void(mouse_event_arg)>;
            using window_resized_callback_t = bbb::opt_arg_function<void(resized_event_arg)>;
        };
    };
};
#endif /* bbb_components_events_hpp */
