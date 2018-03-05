//
//  animation.hpp
//
//  Created by ISHII 2bit on 2018/01/13.
//

#pragma once

#ifndef bbb_animation_hpp
#define bbb_animation_hpp

#include <functional>
#include <memory>
#include <unordered_map>

#include "ofEventUtils.h"

namespace bbb {
    template <typename t1, typename t2, typename t3, typename t4, typename t5>
    auto pmap(t1 v, t2 imin, t3 imax, t4 omin, t5 omax)
    -> decltype((v - imin) * (omax - omin) / (imax - imin) + omin)
    {
        if(omin == omax) return omin;
        else if(imin == imax) return 0.5 * (omin + omax);
        return (v - imin) * (omax - omin) / (imax - imin) + omin;
    }
    template <typename t1, typename t2, typename t3>
    auto pmap(t1 v, t2 omin, t3 omax)
    -> decltype(v * (omax - omin) + omin)
    { return (omin == omax) ? omin : (v * (omax - omin) + omin); };
    
    namespace view_system {
        class animation {
            using ref = std::shared_ptr<animation>;
            using const_ref = std::shared_ptr<const animation>;
            class manager {
                using animation_map = std::unordered_map<std::string, animation::ref>;
                animation_map animations;
                manager() {
                    ofAddListener(ofEvents().update, this, &manager::update, OF_EVENT_ORDER_BEFORE_APP);
                }
                ~manager() {
                    ofRemoveListener(ofEvents().update, this, &manager::update);
                }
                void update(ofEventArgs &) {
                    const float currentTime = ofGetElapsedTimef();
                    for(auto it = animations.begin(); it != animations.end();) {
                        if(it->second->update(currentTime)) {
                            it->second->finish();
                            it = animations.erase(it);
                        } else {
                            ++it;
                        }
                    }
                }
                
            public:
                static manager &get() {
                    static manager _;
                    return _;
                }
                
                inline void add(animation::ref e, const std::string &label) {
                    animations.insert(std::make_pair(label, e));
                }
                
                inline void remove(const std::string &label) {
                    animations.erase(label);
                }
                
                inline animation::ref find(const std::string &label) const {
                    auto it = std::find_if(animations.begin(), animations.end(), [&label](const animation_map::value_type &pair) {
                        return pair.first == label;
                    });
                    return (it == animations.end()) ? animation::ref() : it->second;
                }
            };
            friend class manager;

            std::function<void(float progress)> animationCallback;
            float duration, delay;
            std::string label;
            std::function<void(const std::string &)> callback;
            float startTime, endTime;
            animation(std::function<void(float progress)> animationCallback,
                      float duration,
                      float delay,
                      const std::string &label,
                      const std::function<void(const std::string &)> &callback)
                : animationCallback(animationCallback)
                , duration(duration < 0.0f ? 0.0f : duration)
                , delay(delay)
                , callback(callback)
                , label(label)
            {
                startTime = delay + ofGetElapsedTimef();
                endTime = startTime + duration;
            }
            
        public:
            inline static void add(std::function<void(float)> animationCallback,
                                   float duration,
                                   float delay = 0.0f,
                                   const std::string &label = "",
                                   const std::function<void(const std::string &)> &callback = [](const std::string &){})
            {
                manager::get().add(animation::ref(new animation(animationCallback, duration, delay, label, callback)), label);
            }
            inline static void add(std::function<void(float)> animationCallback,
                                   float duration,
                                   const std::string &label,
                                   const std::function<void(const std::string &)> &callback = [](const std::string &){})
            {
                add(animationCallback, duration, 0.0f, "", callback);
            }
            inline static void add(std::function<void(float)> animationCallback,
                                   float duration,
                                   float delay,
                                   const std::function<void(const std::string &)> &callback)
            {
                add(animationCallback, duration, delay, "", callback);
            }
            inline static void add(std::function<void(float)> animationCallback,
                                   float duration,
                                   const std::function<void(const std::string &)> &callback)
            {
                add(animationCallback, duration, 0.0f, "", callback);
            }
            
            inline static void remove(const std::string &label) {
                manager::get().remove(label);
            }
            
            bool update(float time) {
                if(time < startTime) return false;
                float progress = (startTime == endTime) ? 1.0f : ofMap(time, startTime, endTime, 0.0f, 1.0f, true);
                animationCallback(progress);
                return 1.0f <= progress;
            }
            
            void finish() {
                callback(label);
            }
        };
    };
    namespace vs = view_system;
};

#endif /* bbb_animation_hpp */
