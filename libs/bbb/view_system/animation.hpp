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
    namespace view_system {
        class animation {
            class manager {
                using animation_map = std::unordered_map<std::string, std::shared_ptr<animation>>;
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
                            it++;
                        }
                    }
                }
                
            public:
                static manager &get() {
                    static manager _;
                    return _;
                }
                
                inline void add(std::shared_ptr<animation> e, const std::string &label) {
                    animations.insert(std::make_pair(label, e));
                }
                
                inline void remove(const std::string &label) {
                    animations.erase(label);
                }
            };
            
            std::function<void(float perc)> animationCallback;
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
                , duration(duration)
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
                            float delay,
                            const std::string &label = "",
                            const std::function<void(const std::string &)> &callback = [](const std::string &){})
            {
                manager::get().add(std::shared_ptr<animation>(new animation(animationCallback, duration, delay, label, callback)), label);
            }
            
            inline static void remove(const std::string &label) {
                manager::get().remove(label);
            }
            
            bool update(float time) {
                if(time < startTime) return false;
                float progress = ofMap(time, startTime, endTime, 0.0f, 1.0f, true);
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
