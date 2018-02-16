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
        
        namespace layout {
            struct margin {
                margin(float top, float right, float bottom, float left)
                : top(top)
                , right(right)
                , bottom(bottom)
                , left(left) {};
                margin(float top, float h, float bottom)
                : margin(top, h, bottom, h) {};
                margin(float v, float h)
                : margin(v, h, v, h) {};
                margin(float m = 0.0f)
                : margin(m, m, m, m) {};
                
                inline void set(float margin) { set(margin, margin, margin, margin); };
                inline void set(float vertical, float horizontal) { set(vertical, horizontal, vertical, horizontal); };
                inline void set(float top, float h, float bottom) { set(top, h, bottom, h); };
                inline void set(float top, float right, float bottom, float left) {
                    this->top = top;
                    this->right = right;
                    this->bottom = bottom;
                    this->left = left;
                };

                float top;
                float right;
                float bottom;
                float left;
            };
        };
        
        namespace components {
            struct view;
            using view_ref = std::shared_ptr<view>;
            
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
            
            using click_down_callback_t = std::function<void(mouse_event_arg)>;
            using click_up_callback_t = std::function<void(mouse_event_arg)>;
            using mouse_over_callback_t = std::function<void(mouse_event_arg)>;
            using window_resized_callback_t = std::function<void(resized_event_arg)>;
            
            static void mouse_default(mouse_event_arg);
            static void resized_default(resized_event_arg arg);
            static void fitToParent(resized_event_arg arg);
            
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
                    setting(float x, float y, float width, float height, const layout::margin &margin = {})
                    : frame(x, y, width, height)
                    , margin(margin) {};
                    
                    setting(const ofRectangle &frame = {}, const layout::margin &margin = {})
                    : setting(frame.x, frame.y, frame.width, frame.height, margin) {};
                    
                    ofRectangle frame{0.0f, 0.0f, 0.0f, 0.0f};
                    layout::margin margin{0.0f};
                    
                    bool isVisible{true};
                    bool isEventTransparent{false};
                    bool isEnabledUserInteraction{true};
                    bool isResizable{false};
                    
                    ofFloatColor backgroundColor{0.0f, 0.0f, 0.0f, 0.0f};
                    float alpha{1.0f};
                };
                
                inline static view::ref create(const ofRectangle &rect, const layout::margin &margin = {0.0f}) {
                    return create({rect, margin});
                }
                
                inline static view::ref create(float x, float y, float width, float height, const layout::margin &margin = {}) {
                    return create({{x, y, width, height}, margin});
                };
                
                inline static view::ref create(const view::setting &setting_ = {}) {
                    return std::make_shared<view>(setting_);
                }
                
                inline view() {};
                inline view(const view::setting &setting_)
                : setting_(setting_)
                , position(setting_.frame.position)
                {
                    calculateLayout();
                };
                inline view(view::setting &&setting_)
                : setting_(std::move(setting_))
                , position(setting_.frame.position)
                {
                    calculateLayout();
                };

                inline view(float x, float y, float width, float height, const layout::margin &margin = {})
                : view({{x, y, width, height}, margin})
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
                    return (result == subviews.end()) ? view::ref() : *result;
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
                
                inline void removeFromParent() {
                    auto &&parent = getParent();
                    if(parent) parent->remove(shared_from_this());
                    else ofLogWarning() << "maybe this view [" << getName() << "] is root";
                }
                
                inline setting &getSetting() { return setting_; }
                inline const setting &getSetting() const { return setting_; }

                inline void setOrigin(float x, float y) {
                    position.x = x;
                    position.y = y;
                }
                inline void setOrigin(const ofPoint &p) { setOrigin(p.x, p.y); }
                
                inline bool isShown() const { return getSetting().isVisible; };
                inline void setVisible(bool isVisible) { getSetting().isVisible = isVisible; };
                inline void show() { setVisible(true); };
                inline void hide() { setVisible(false); };
                
                inline bool isResizable() const {
                    ofLogWarning() << "feature of resizing: not implemented";
                    return getSetting().isResizable;
                };
                inline bool setEnableResize(bool isResizable) {
                    ofLogWarning() << "feature of resizing: not implemented";
                    return getSetting().isResizable = isResizable;
                };

                inline void setEventTransparentness(bool isEventTransparent) {
                    getSetting().isEventTransparent = isEventTransparent;
                }
                inline bool isEventTransparent() const { return getSetting().isEventTransparent; };

                inline bool isEnabledUserInteraction() const { return getSetting().isEnabledUserInteraction; };
                inline void enableUserInteraction() { getSetting().isEnabledUserInteraction = true; };
                inline void disableUserInteraction() { getSetting().isEnabledUserInteraction = false; };

                inline bool isClickedNow() const { return isClickedNow_; };
                
                inline ofPoint convertToLocalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p -= v->position;
                    return p;
                }
                
                inline ofPoint convertToGlobalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p += v->position;
                    return p;
                }
                
                inline void onClickDown(std::function<void(mouse_event_arg)> callback) {
                    clickDownCallback = callback;
                }
                
                inline void onClickUp(std::function<void(mouse_event_arg)> callback) {
                    clickUpCallback = callback;
                }
                
                inline void onMouseOver(std::function<void(mouse_event_arg)> callback) {
                    mouseOverCallback = callback;
                }
                
                inline void onWindowResized(std::function<void(resized_event_arg)> callback) {
                    windowResizedCallback = callback;
                }
                
                inline bool isInside(const ofPoint &p) const {
                    return ofRectangle(topLeft(), width, height).inside(p);
                }
                
                template <typename integer_t>
                inline auto setBackgroundColor(integer_t r, integer_t g, integer_t b, integer_t a = 255)
                    -> typename std::enable_if<std::is_integral<integer_t>::value>::type
                {
                    getSetting().backgroundColor.set(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                }
                inline void setBackgroundColor(float r, float g, float b, float a = 1.0f) {
                    getSetting().backgroundColor.set(r, g, b, a);
                }
                inline void setBackgroundColor(const ofFloatColor &c) { setBackgroundColor(c.r, c.g, c.b, c.a); };
                inline void setBackgroundColor(const ofColor &c) { setBackgroundColor(c.r, c.g, c.b, c.a); };

                inline ofFloatColor &getBackgroundColor() { return getSetting().backgroundColor; };
                inline const ofFloatColor &getBackgroundColor() const { return getSetting().backgroundColor; };

                inline float getAlpha() const { return getParent().get() ? (getParent()->getAlpha() * getSetting().alpha) : getSetting().alpha; };
                inline void setAlpha(float alpha) { getSetting().alpha = alpha; };
                inline void setAlpha(std::uint8_t alpha) { getSetting().alpha = alpha / 255.0f; };
                
                inline const std::string &getName() const & { return name; };
                inline std::string &&getName() && { return std::move(name); };
                
                inline ofPoint &getPosition() { return position; };
                inline ofPoint getPosition() const { return position; };
                inline void setPosition(const ofPoint &p) { position = p; }
                inline void moveTo(const ofPoint &p) { position = p; }
                inline void moveTo(float x, float y) { position.set(x, y); }
                inline void move(const ofPoint &p) { position += p; }
                inline void move(float x, float y) { position.x += x, position.y += y; }
                
                inline float getWidth() const { return width; };
                inline void setWidth(float width) {
                    getSetting().frame.width = width;
                    calculateLayout();
                }
                
                inline float getHeight() const { return height; };
                inline void setHeight(float height) {
                    getSetting().frame.height = height;
                    calculateLayout();
                };

                inline float setSize(float width, float height) {
                    getSetting().frame.width = width;
                    getSetting().frame.height = width;
                    calculateLayout();
                }
                
                inline void calculateLayout() {
                    auto &margin = getSetting().margin;
                    width = getSetting().frame.width - margin.right - margin.left;
                    height = getSetting().frame.height - margin.top - margin.bottom;
                }
                
                inline void setMargin(float margin) { setMargin(margin, margin, margin, margin); };
                inline void setMargin(float vertical, float horizontal) { setMargin(vertical, horizontal, vertical, horizontal); };
                inline void setMargin(float top, float h, float bottom) { setMargin(top, h, bottom, h); };
                inline void setMargin(float top, float right, float bottom, float left) {
                    getSetting().margin.set(top, right, bottom, left);
                    calculateLayout();
                };
                
                inline view::ref getParent() { return parent.lock(); };
                inline view::const_ref getParent() const { return parent.lock(); };
                inline view::ref getSubview(const std::string &name) { return find(name); };
                
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
                    if(!isShown()) return;
                    pushState();
                    ofTranslate(position + ofPoint(getSetting().margin.left, getSetting().margin.top));
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
                    ofAddListener(events.windowResized, this, &view::windowResizedRoot, OF_EVENT_ORDER_BEFORE_APP);
                }
                
                void unregisterEvents() {
                    auto &&events = ofEvents();
                    ofRemoveListener(events.mousePressed, this, &view::mousePressed);
                    ofRemoveListener(events.mouseReleased, this, &view::mouseReleased);
                    ofRemoveListener(events.mouseMoved, this, &view::mouseMoved);
                    ofRemoveListener(events.mouseDragged, this, &view::mouseDragged);
                    ofRemoveListener(events.windowResized, this, &view::windowResizedRoot);
                }
            protected:
                setting setting_;
                bool isClickedNow_{false};
                
                void mousePressed(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    clickDown(ofPoint(arg.x, arg.y));
                }
                void mouseReleased(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    clickUp(ofPoint(arg.x, arg.y));
                }
                void mouseMoved(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    mouseOver(ofPoint(arg.x, arg.y));
                }
                void mouseDragged(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    mouseOver(ofPoint(arg.x, arg.y));
                }
                void windowResizedRoot(ofResizeEventArgs &arg) {
                    windowResized({shared_from_this(), {ofPoint(), (float)arg.width, (float)arg.height}});
                }
                
                inline bool clickDown(const ofPoint &p) {
                    if(!isShown()) return false;
                    for(auto &&subview : subviews) if(subview->clickDown(p)) return true;
                    if(isEnabledUserInteraction() && isInside(p)) {
                        isClickedNow_ = true;
                        clickDownCallback({shared_from_this(), p, false});
                        return !isEventTransparent();
                    }
                    return false;
                }
                
                inline bool clickUp(const ofPoint &p) {
                    if(!isShown()) return false;
                    for(auto &&subview : subviews) if(subview->clickUp(p)) return true;
                    if((isEnabledUserInteraction() && isInside(p)) || isClickedNow_) {
                        isClickedNow_ = false;
                        clickUpCallback({shared_from_this(), p, false});
                        return !isEventTransparent();
                    }
                    return false;
                }
                
                inline void mouseOver(const ofPoint &p) {
                    if(!isShown()) return;
                    for(auto &&subview : subviews) subview->mouseOver(p);
                    if((isEnabledUserInteraction() && isInside(p))) {
                        mouseOverCallback({shared_from_this(), p, false});
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
                    if(0.0f < getBackgroundColor().a * getAlpha()) {
                        auto &&c = ofFloatColor(getBackgroundColor().r,
                                                getBackgroundColor().g,
                                                getBackgroundColor().b,
                                                getBackgroundColor().a * getAlpha());
                        ofSetColor(c);
                        ofDrawRectangle(0.0f, 0.0f, width, height);
                    }
                }
                
                inline void drawSubviews() const {
                    for(auto &&v : subviews) v->draw();
                }
                
                virtual void drawSpecific() const {};
                
                inline void windowResized(resized_event_arg super_arg) {
                    windowResizeSpecific(super_arg);
                    for(auto &&subview : subviews) {
                        subview->windowResized({subview, {position, width, height}});
                    }
                }
                
                virtual void windowResizeSpecific(resized_event_arg arg) {
                    windowResizedCallback(arg);
                };
                
                ofPoint position;
                float width;
                float height;
                
                std::function<void(mouse_event_arg)> clickDownCallback{mouse_default};
                std::function<void(mouse_event_arg)> clickUpCallback{mouse_default};
                std::function<void(mouse_event_arg)> mouseOverCallback{mouse_default};
                std::function<void(mouse_event_arg)> draggedCallback{mouse_default};
                
                std::function<void(resized_event_arg)> windowResizedCallback{resized_default};
                
                std::string name{""};
                std::vector<view::ref> subviews;
                std::weak_ptr<view> parent{};
            };
            
            static void mouse_default(mouse_event_arg) {};
            static void resized_default(resized_event_arg arg) {};
            static void fitToParent(resized_event_arg arg) {
                arg.target->setSize(arg.rect.width, arg.rect.height);
            };

            struct image : public view {
                using ref = std::shared_ptr<image>;
                
                struct setting : public view::setting {
                    setting(const ofRectangle &rect = {}, const layout::margin &margin = {})
                    : view::setting(rect, margin) {};
                    setting(boost::filesystem::path imagePath, const ofRectangle &rect, const layout::margin &margin = {})
                    : view::setting(rect, margin)
                    , imagePath(imagePath) {};
                    setting(const view::setting &setting)
                    : view::setting(setting) {};
                    
                    boost::filesystem::path imagePath{"didn't load from file"};
                };
                
                inline static image::ref create(const setting &setting_) {
                    return create(setting_);
                }
                
                inline static image::ref create() {
                    return create(0.0f, 0.0f, 0.0f, 0.0f);
                }
                
                inline static image::ref create(const ofRectangle &rect) {
                    return create(setting(rect));
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
                    setWidth(image_.getWidth());
                    setHeight(image_.getHeight());
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
                
                struct setting : public view::setting {
                    setting(const ofRectangle &rect = {}, const layout::margin &margin = {})
                    : view::setting(rect, margin) {};
                    setting(const view::setting &setting)
                    : view::setting(setting) {};
                };

                inline static drawer::ref create(float x, float y, float width, float height) {
                    return create({ofRectangle(x, y, width, height)});
                }
                
                inline static drawer::ref create(const ofRectangle &rect) {
                    return create({rect});
                }
                
                inline static drawer::ref create(std::function<void(drawer::const_ref)> callback, float x, float y, float width, float height) {
                    return create(callback, {ofRectangle(x, y, width, height)});
                }

                inline static drawer::ref create(std::function<void(drawer::const_ref)> callback, const ofRectangle &rect) {
                    return create(callback, {rect});
                };
                
                inline static drawer::ref create(std::function<void(drawer::const_ref)> callback, const setting &setting_) {
                    return std::make_shared<drawer>(callback, setting_);
                };
                
                inline static drawer::ref create(const drawer::setting &setting_ = {}) {
                    return std::make_shared<drawer>(setting_);
                }
                
                inline drawer(std::function<void(drawer::const_ref)> callback, const setting &setting_ = {})
                    : view(setting_)
                    , drawCallback(callback)
                {};
                inline drawer(const setting &setting_ = {})
                    : view(setting_)
                {};
                
                void onDraw(std::function<void(drawer::const_ref)> callback) {
                    drawCallback = callback;
                }
                
                virtual void drawSpecific() const override {
                    drawCallback(std::dynamic_pointer_cast<const drawer>(shared_from_this()));
                }
                
            protected:
                std::function<void(drawer::const_ref)> drawCallback{[](drawer::const_ref){}};
                setting setting_;
            };
        }; // components
        using namespace components;
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_hpp */
