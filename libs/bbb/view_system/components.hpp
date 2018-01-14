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
            static void click_default(std::shared_ptr<view>, const ofPoint &p) {};
            
            struct view : public std::enable_shared_from_this<view> {
                using ref = std::shared_ptr<view>;
                using const_ref = std::shared_ptr<const view>;

                inline static ofPoint previouseMousePosition() {
                    return ofPoint(ofGetPreviousMouseX(), ofGetPreviousMouseY());
                }
                inline static ofPoint mousePosition() {
                    return ofPoint(ofGetMouseX(), ofGetMouseY());
                }
                inline static ofPoint mouseDelta() {
                    return mousePosition() - previouseMousePosition();
                }

                struct setting {
                    ofRectangle frame{0.0f, 0.0f, 0.0f, 0.0f};
                };
                
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
                    unregisterEvents();
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

                inline bool isClickedNow() const { return isClickedNow_; };
                
                inline ofPoint convertToLocalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p -= v->position;
                    return p;
                }
                
                inline ofPoint convertToGlobalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p += v->position;
                    return p;
                }
                
                inline void onClickDown(std::function<void(view::ref, ofPoint)> callback) {
                    clickDownCallback = callback;
                }
                
                inline void onClickUp(std::function<void(view::ref, ofPoint)> callback) {
                    clickUpCallback = callback;
                }
                
                inline void onMouseOver(std::function<void(view::ref, ofPoint)> callback) {
                    mouseOverCallback = callback;
                }
                
                inline bool isInside(const ofPoint &p) const {
                    return ofRectangle(topLeft(), width, height).inside(p);
                }
                
                template <typename integer_t>
                inline auto setBackgroundColor(integer_t r, integer_t g, integer_t b, integer_t a = 255)
                    -> typename std::enable_if<std::is_integral<integer_t>::value>::type
                {
                    backgroundColor.set(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                }
                inline void setBackgroundColor(float r, float g, float b, float a = 1.0f) {
                    backgroundColor.set(r, g, b, a);
                }
                inline void setBackgroundColor(const ofFloatColor &c) { setBackgroundColor(c.r, c.g, c.b, c.a); };
                inline void setBackgroundColor(const ofColor &c) { setBackgroundColor(c.r, c.g, c.b, c.a); };

                inline ofFloatColor &getBackgroundColor() { return backgroundColor; };
                inline const ofFloatColor &getBackgroundColor() const { return backgroundColor; };

                inline float getAlpha() const { return getParent().get() ? (getParent()->getAlpha() * alpha) : alpha; };
                inline void setAlpha(float alpha) { this->alpha = alpha; };
                inline void setAlpha(std::uint8_t alpha) { this->alpha = alpha / 255.0f; };
                
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
                
                inline view::ref getParent() { return parent.lock(); };
                inline view::const_ref getParent() const { return parent.lock(); };

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
                
                void registerEvents() {
                    auto &&events = ofEvents();
                    ofAddListener(events.mousePressed, this, &view::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
                    ofAddListener(events.mouseReleased, this, &view::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
                    ofAddListener(events.mouseMoved, this, &view::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
                    ofAddListener(events.mouseDragged, this, &view::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
                }
                
                void unregisterEvents() {
                    auto &&events = ofEvents();
                    ofRemoveListener(events.mousePressed, this, &view::mousePressed);
                    ofRemoveListener(events.mouseReleased, this, &view::mouseReleased);
                    ofRemoveListener(events.mouseMoved, this, &view::mouseMoved);
                    ofRemoveListener(events.mouseDragged, this, &view::mouseDragged);
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
                void mouseMoved(ofMouseEventArgs &arg) {
                    if(!isVisible) return;
                    mouseOver(ofPoint(arg.x, arg.y));
                }
                void mouseDragged(ofMouseEventArgs &arg) {
                    if(!isVisible) return;
                    mouseOver(ofPoint(arg.x, arg.y));
                }

                inline bool clickDown(const ofPoint &p) {
                    if(!isVisible) return false;
                    for(auto &&subview : subviews) if(subview->clickDown(p)) return true;
                    if(isEnabledUserInteraction_ && isInside(p)) {
                        isClickedNow_ = true;
                        clickDownCallback(shared_from_this(), p);
                        return !isEventTransparent();
                    }
                    return false;
                }
                
                inline bool clickUp(const ofPoint &p) {
                    if(!isVisible) return false;
                    for(auto &&subview : subviews) if(subview->clickUp(p)) return true;
                    if((isEnabledUserInteraction_ && isInside(p)) || isClickedNow_) {
                        isClickedNow_ = false;
                        clickUpCallback(shared_from_this(), p);
                        return !isEventTransparent();
                    }
                    return false;
                }
                
                inline void mouseOver(const ofPoint &p) {
                    if(!isVisible) return;
                    for(auto &&subview : subviews) subview->mouseOver(p);
                    if((isEnabledUserInteraction_ && isInside(p))) {
                        mouseOverCallback(shared_from_this(), p);
                    }
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
                    if(0.0f < backgroundColor.a * getAlpha()) {
                        auto &&c = ofFloatColor(backgroundColor.r,
                                                backgroundColor.g,
                                                backgroundColor.b,
                                                backgroundColor.a * getAlpha());
                        ofSetColor(c);
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
                
                ofFloatColor backgroundColor{0.0f, 0.0f, 0.0f, 0.0f};
                float alpha{1.0f};
                
                std::function<void(view::ref, ofPoint)> clickDownCallback{click_default};
                std::function<void(view::ref, ofPoint)> clickUpCallback{click_default};
                std::function<void(view::ref, ofPoint)> mouseOverCallback{click_default};
                std::function<void(view::ref, ofPoint)> draggedCallback{click_default};

                std::string name{""};
                std::vector<view::ref> subviews;
                std::weak_ptr<view> parent{};
                
                bool isVisible{true};
                bool isEventTransparent_{false};
                bool isEnabledUserInteraction_{true};
                bool isClickedNow_{false};
            };
            
            struct image : public view {
                using ref = std::shared_ptr<image>;
                
                struct setting : public view::setting {
                    boost::filesystem::path imagePath;
                };
                
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
                inline bool load(const boost::filesystem::path &path) {
                    return image_.load(path);
                }
                
            protected:
                ofImage image_;
            };
            
            struct drawer : public view {
                using ref = std::shared_ptr<drawer>;
                using const_ref = std::shared_ptr<const drawer>;

                inline static drawer::ref create() {
                    return create(0.0f, 0.0f, 0.0f, 0.0f);
                }
                
                inline static drawer::ref create(const ofRectangle &rect) {
                    return create(rect.x, rect.y, rect.width, rect.height);
                }
                
                inline static drawer::ref create(float x, float y, float width, float height) {
                    return std::make_shared<drawer>(x, y, width, height);
                }
                
                inline static drawer::ref create(std::function<void(drawer::const_ref)> callback, const ofRectangle &rect) {
                    return create(callback, rect.x, rect.y, rect.width, rect.height);
                };
                
                inline static drawer::ref create(std::function<void(drawer::const_ref)> callback, float x, float y, float width, float height) {
                    return std::make_shared<drawer>(callback, x, y, width, height);
                }

                inline drawer() {};
                inline drawer(std::function<void(drawer::const_ref)> callback, float x, float y, float width, float height)
                    : view(x, y, width, height)
                    , drawCallback(callback)
                {};
                inline drawer(float x, float y, float width, float height)
                    : view(x, y, width, height)
                {};
                
                void setDrawCallback(std::function<void(drawer::const_ref)> callback) {
                    drawCallback = callback;
                }
                
                virtual void drawSpecific() const override {
                    drawCallback(std::dynamic_pointer_cast<const drawer>(shared_from_this()));
                }
                
            protected:
                std::function<void(drawer::const_ref)> drawCallback{[](drawer::const_ref){}};
            };
        }; // components
        using namespace components;
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_hpp */
