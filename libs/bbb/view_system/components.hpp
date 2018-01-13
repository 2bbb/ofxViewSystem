//
//  components.hpp
//
//  Created by ISHII 2bit on 2018/01/12.
//

#pragma once

#ifndef bbb_components_hpp
#define bbb_components_hpp

#include <map>
#include <string>
#include <memory>
#include <functional>

#include "ofEventUtils.h"
#include "ofVectorMath.h"
#include "ofImage.h"
#include "ofColor.h"

namespace bbb {
    namespace view_system {
        namespace components {
            struct view;
            static void click_default(view &, const ofPoint &p) {};
            
            struct view : public std::enable_shared_from_this<view> {
                using ref = std::shared_ptr<view>;
                
                inline static view::ref create() {
                    return create(0.0f, 0.0f, 0.0f, 0.0f);
                }
                
                inline static view::ref create(const ofRectangle &rect) {
                    return create(rect.x, rect.y, rect.width, rect.height);
                }
                
                inline static view::ref create(float x, float y, float width, float height) {
                    return std::make_shared<view>(x, y, width, height);
                };
                
                inline view() {};
                inline view(float x, float y, float width, float height)
                    : position(x, y)
                    , width(width)
                    , height(height)
                {};
                
                virtual ~view() {
                    ofRemoveListener(ofEvents().mousePressed, this, &view::mousePressed);
                    ofRemoveListener(ofEvents().mouseReleased, this, &view::mouseReleased);
                };
                
                inline void add(const std::string &name, view::ref v) {
                    if(v->parent.lock()) v->parent.lock()->remove(v);
                    remove(name);
                    
                    v->name = name;
                    v->parent = shared_from_this();
                    subviews.emplace_back(v);
                }
                
                inline void add(view::ref v) {
                    if(v->parent.lock()) v->parent.lock()->remove(v);
                    v->parent = shared_from_this();
                    subviews.emplace_back(v);
                }
                
                inline view::ref find(const std::string &name) const {
                    auto &&result = std::find_if(subviews.begin(), subviews.end(), [&](view::ref v) {
                        return v->name == name;
                    });
                    return *result ? *result : view::ref();
                }
                
                inline void remove(const std::string &name) {
                    auto &&eraser = std::remove_if(subviews.begin(), subviews.end(), [&](view::ref v) {
                        return v->name == name;
                    });
                    auto &&end = subviews.end();
                    std::for_each(eraser, end, [](view::ref v) {
                        v->parent.reset();
                    });
                    subviews.erase(eraser, end);
                }
                
                inline void remove(const view::ref &v) {
                    auto &&eraser = std::remove(subviews.begin(), subviews.end(), v);
                    auto &&end = subviews.end();
                    std::for_each(eraser, end, [](view::ref v) {
                        v->parent.reset();
                    });
                    subviews.erase(eraser, end);
                }
                
                inline void setOrigin(float x, float y) {
                    position.x = x;
                    position.y = y;
                }
                inline void setOrigin(const ofPoint &p) { setOrigin(p.x, p.y); }
                
                inline bool isShown() const { return isVisible; };
                inline void setVisible(bool isVisible) { this->isVisible = isVisible; };
                inline void show() { setVisible(true); };
                inline void hide() { setVisible(false); };
                
                inline void setEventTransparentness(bool isEventTransparent) {
                    isEventTransparent_ = isEventTransparent;
                }
                inline bool isEventTransparent() const { return isEventTransparent_; };

                inline bool isEnabledUserInteraction() const { return isEnabledUserInteraction_; };
                inline void enableUserInteraction() { isEnabledUserInteraction_ = true; };
                inline void disableUserInteraction() { isEnabledUserInteraction_ = false; };

                inline ofPoint convertToLocalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p -= v->position;
                    return p;
                }
                
                inline ofPoint convertToGlobalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p += v->position;
                    return p;
                }
                
                inline void onClickDown(std::function<void(view &, ofPoint)> callback) {
                    clickDownCallback = callback;
                }
                
                inline void onClickUp(std::function<void(view &, ofPoint)> callback) {
                    clickUpCallback = callback;
                }
                
                inline bool isInside(const ofPoint &p) const {
                    return ofRectangle(topLeft(), width, height).inside(p);
                }
                
                inline void setBackgroundColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
                    backgroundColor.set(r, g, b, a);
                }
                inline void setBackgroundColor(const ofColor &c) { setBackgroundColor(c.r, c.g, c.b, c.a); }
                
                inline ofColor &getBackgroundColor() { return backgroundColor; };
                inline const ofColor &getBackgroundColor() const { return backgroundColor; };

                inline ofPoint &getPosition() { return position; };
                inline ofPoint getPosition() const { return position; };
                inline void setPosition(const ofPoint &p) { position = p; }
                inline void moveTo(const ofPoint &p) { position = p; }
                inline void moveTo(float x, float y) { position.set(x, y); }
                inline void move(const ofPoint &p) { position += p; }
                inline void move(float x, float y) { position.x += x, position.y += y; }
                
                inline const std::string &getName() const & { return name; };
                inline std::string &&getName() && { return std::move(name); };
                
                inline float &getWidth() { return width; };
                inline float getWidth() const { return width; };
                
                inline float &getHeight() { return height; };
                inline float getHeight() const { return height; };
                
                inline std::weak_ptr<view> getParent() { return parent; };
                inline const std::weak_ptr<view> getParent() const { return parent; };
                
                inline float left() const { return convertToGlobalCoordinate().x; };
                inline float right() const { return left() + width; };
                inline float top() const { return convertToGlobalCoordinate().y; };
                inline float bottom() const { return top() + height; };

                inline float horizontalCenter() const { return left() + width * 0.5f; };
                inline float verticalCenter() const { return top() + height * 0.5f; };

                inline ofPoint topLeft() const { return convertToGlobalCoordinate(); };
                inline ofPoint topRight() const { return topLeft() + ofPoint(width, 0); };
                inline ofPoint bottomLeft() const { return topLeft() + ofPoint(0, height); };
                inline ofPoint bottomRight() const { return topLeft() + ofPoint(width, height); };
                inline ofPoint center() const { return topLeft() + ofPoint(width * 0.5f, height * 0.5f); };
                
                void draw() const {
                    if(!isVisible) return;
                    pushState();
                    ofTranslate(position);
                    drawBackground();
                    drawSpecific();
                    drawSubviews();
                    popState();
                }
                
                void registerEvent() {
                    ofAddListener(ofEvents().mousePressed, this, &view::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
                    ofAddListener(ofEvents().mouseReleased, this, &view::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
                }
                
            protected:
                void mousePressed(ofMouseEventArgs &arg) {
                    if(!isVisible) return;
                    clickDown(ofPoint(arg.x, arg.y));
                }
                void mouseReleased(ofMouseEventArgs &arg) {
                    if(!isVisible) return;
                    clickUp(ofPoint(arg.x, arg.y));
                }

                inline bool clickDown(const ofPoint &p) {
                    if(!isVisible) return false;
                    for(auto &&subview : subviews) if(subview->clickDown(p)) return true;
                    if(isEnabledUserInteraction_ && isInside(p)) return clickDownCallback(*this, p), !isEventTransparent();
                    return false;
                }
                
                inline bool clickUp(const ofPoint &p) {
                    if(!isVisible) return false;
                    for(auto &&subview : subviews) if(subview->clickUp(p)) return true;
                    if(isEnabledUserInteraction_ && isInside(p)) return clickUpCallback(*this, p), !isEventTransparent();
                    return false;
                }

                inline void pushState() const {
                    ofPushMatrix();
                    ofPushStyle();
                }
                
                inline void popState() const {
                    ofPopStyle();
                    ofPopMatrix();
                }
                
                inline void drawBackground() const {
                    if(backgroundColor.a) {
                        ofSetColor(backgroundColor);
                        ofDrawRectangle(0.0f, 0.0f, width, height);
                    }
                }
                
                inline void drawSubviews() const {
                    for(auto &&v : subviews) v->draw();
                }
                
                virtual void drawSpecific() const {};
                
                ofPoint position;
                float width;
                float height;
                
                ofColor backgroundColor{0, 0, 0, 0};
                
                std::function<void(view &, ofPoint)> clickDownCallback{click_default};
                std::function<void(view &, ofPoint)> clickUpCallback{click_default};
                
                std::string name{""};
                std::vector<view::ref> subviews;
                std::weak_ptr<view> parent{};
                
                bool isVisible{true};
                bool isEventTransparent_{false};
                bool isEnabledUserInteraction_{true};
            };
            
            struct image : public view {
                using ref = std::shared_ptr<image>;
                
                inline static image::ref create() {
                    return create(0.0f, 0.0f, 0.0f, 0.0f);
                }
                
                inline static image::ref create(const ofRectangle &rect) {
                    return create(rect.x, rect.y, rect.width, rect.height);
                }
                
                inline static image::ref create(float x, float y, float width, float height) {
                    return std::make_shared<image>(x, y, width, height);
                }
                
                inline static image::ref create(ofImage image_, const ofRectangle &rect) {
                    return create(image_, rect.x, rect.y, rect.width, rect.height);
                };
                
                inline static image::ref create(ofImage image_, float x, float y, float width, float height) {
                    return std::make_shared<image>(image_, x, y, width, height);
                }
                
                inline image() {};
                inline image(ofImage image_, float x, float y, float width, float height)
                    : view(x, y, width, height)
                    , image_(image_)
                {};
                inline image(float x, float y, float width, float height)
                    : view(x, y, width, height)
                {};
                
                virtual ~image() {};
                
                inline void fitToImage() {
                    width = image_.getWidth();
                    height = image_.getHeight();
                }
                
                virtual void drawSpecific() const override {
                    if(image_.isAllocated()) {
                        ofSetColor(ofColor::white);
                        image_.draw(0.0f, 0.0f, width, height);
                    }
                }
                
                inline const ofImage &getImage() const & { return image_; };
                inline ofImage &&getImage() && { return std::move(image_); };
                inline operator const ofImage &() const & { return image_; };
                inline operator ofImage &&() && { return std::move(image_); };
                
            protected:
                ofImage image_;
            };
            
            struct drawer : public view {
                using ref = std::shared_ptr<drawer>;
                
                inline static drawer::ref create() {
                    return create(0.0f, 0.0f, 0.0f, 0.0f);
                }
                
                inline static drawer::ref create(const ofRectangle &rect) {
                    return create(rect.x, rect.y, rect.width, rect.height);
                }
                
                inline static drawer::ref create(float x, float y, float width, float height) {
                    return std::make_shared<drawer>(x, y, width, height);
                }
                
                inline static drawer::ref create(std::function<void(const drawer &)> callback, const ofRectangle &rect) {
                    return create(callback, rect.x, rect.y, rect.width, rect.height);
                };
                
                inline static drawer::ref create(std::function<void(const drawer &)> callback, float x, float y, float width, float height) {
                    return std::make_shared<drawer>(callback, x, y, width, height);
                }

                inline drawer() {};
                inline drawer(std::function<void(const drawer &)> callback, float x, float y, float width, float height)
                    : view(x, y, width, height)
                    , drawCallback(callback)
                {};
                inline drawer(float x, float y, float width, float height)
                    : view(x, y, width, height)
                {};
                
                void setDrawCallback(std::function<void(const drawer &)> callback) {
                    drawCallback = callback;
                }
                
                virtual void drawSpecific() const override {
                    drawCallback(*this);
                }
                
            protected:
                std::function<void(const drawer &)> drawCallback{[](const drawer &){}};
            };
        }; // components
        using namespace components;
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_hpp */
