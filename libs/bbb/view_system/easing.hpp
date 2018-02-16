//
//  easing.hpp
//
//  Created by ISHII 2bit on 2018/01/14.
//

#ifndef easing_hpp
#define easing_hpp

#include <cmath>

#ifndef _USE_MATH_DEFINES
#   define _USE_MATH_DEFINES
#endif
#include <math.h>

namespace bbb {
    namespace view_system {
        namespace easing {
            namespace quadratic {
                static inline float in(float t) {
                    return t * t;
                }
                static inline float out(float t) {
                    return t * (2.0f - t);
                }
                static inline float in_out(float t) {
                    t *= 2.0f;
                    if(t < 1.0f) return 0.5f * t * t;
                    t -= 1.0f;
                    return 0.5 * (1.0f + t * (2.0f - t));
                }
            };
            namespace quad = quadratic;
            
            namespace cubic {
                static inline float in(float t) {
                    return t * t * t;
                }
                static inline float out(float t) {
                    t -= 1.0f;
                    return t * t * t + 1.0f;
                }
                static inline float in_out(float t) {
                    t *= 2.0f;
                    if(t < 1.0f) return 0.5f * t * t * t;
                    t -= 2.0f;
                    return 0.5f * (t * t * t + 2.0f);
                }
            };
            
            namespace quartic {
                static inline float in(float t) {
                    return t * t * t * t;
                }
                static inline float out(float t) {
                    t -= 1.0f;
                    return 1.0f - t * t * t * t;
                }
                static inline float in_out(float t) {
                    t *= 2.0f;
                    if(t < 1.0f) return 0.5f * t * t * t * t;
                    t -= 2.0f;
                    return 1 - 0.5f * t * t * t * t;
                }
            };
            namespace quart = quartic;
            
            namespace quintic {
                static inline float in(float t) {
                    return t * t * t * t * t;
                }
                static inline float out(float t) {
                    t -= 1.0f;
                    return t * t * t * t * t + 1.0f;
                }
                static inline float in_out(float t,float totaltime,float max ,float min) {
                    t *= 2.0f;
                    if(t < 1.0f) return 0.5f * t * t * t * t * t;
                    t -= 2.0f;
                    return 0.5f * t * t * t * t * t + 1.0f;
                }
            };
            namespace quin = quintic;
            
            namespace sine {
                static inline float in(float t) {
                    return 1.0f - std::cos(0.5f * t * M_PI);
                }
                static inline float out(float t) {
                    return std::sin(0.5f * t * M_PI);
                }
                static inline float in_out(float t) {
                    return 0.5f - 0.5f * std::cos(t * M_PI);
                }
            };
            namespace sin = sine;
            
            namespace exponential {
                static inline float in(float t) {
                    return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
                }
                static inline float out(float t) {
                    return t == 1.0f ? 1.0f : (1.0f - std::pow(2.0f, -10.0f * t));
                }
                static inline float in_out(float t) {
                    if(t == 1.0f) return t;
                    t *= 2.0f;
                    if(t < 1.0f) return 0.5f * std::pow(2.0f, 10.0f * (t - 1.0f));
                    t -= 1.0f;
                    return 1.0f - 0.5f * std::pow(2.0f, -10.0f * t);
                }
            };
            namespace exp = exponential;
            
            namespace circular {
                static inline float in(float t) {
                    return 1.0f - std::sqrt(1.0f - t * t);
                }
                static inline float out(float t) {
                    t -= 1.0f;
                    return std::sqrt(1.0f - t * t);
                }
                static inline float in_out(float t) {
                    t *= 0.5f;
                    if(t < 1.0f) return -0.5f * (std::sqrt(1.0f - t * t) - 1.0f);
                    t -= 2.0f;
                    return 0.5f * (std::sqrt(1.0f - t * t) + 1.0f);
                }
            };
            namespace circ = circular;
        };
    };
    namespace vs = view_system;
};

#endif /* easing_hpp */
