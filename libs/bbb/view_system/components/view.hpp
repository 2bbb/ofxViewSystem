//
//  components/view.hpp
//
//  Created by ISHII 2bit on 2018/02/21.
//

#pragma once

#ifndef bbb_components_view_hpp
#define bbb_components_view_hpp

#include <map>
#include <string>
#include <memory>
#include <functional>

#include "./events.hpp"
#include "./type_utils.hpp"
#include "../layout.hpp"
#include "../animation.hpp"

#include "../opt_arg_function.hpp"

#include "ofEventUtils.h"
#include "ofVectorMath.h"
#include "ofImage.h"
#include "ofColor.h"

namespace bbb {
    namespace view_system {
        inline namespace components {
            template <typename view_type>
            static inline std::shared_ptr<view_type> create(const typename view_type::setting &setting) {
                return std::make_shared<view_type>(setting);
            }
            
            namespace { // make static
                void mouse_default(mouse_event_arg);
                void resized_default(resized_event_arg arg);
                void fitToParent(resized_event_arg arg);
            }

            struct view : public std::enable_shared_from_this<view> {
                using ref = std::shared_ptr<view>;
                using const_ref = std::shared_ptr<const view>;
                
                inline static ofPoint previouseMousePosition()
                { return ofPoint(ofGetPreviousMouseX(), ofGetPreviousMouseY()); };
                inline static ofPoint mousePosition()
                { return ofPoint(ofGetMouseX(), ofGetMouseY()); };
                inline static ofPoint mouseDelta()
                { return mousePosition() - previouseMousePosition(); };
                
                template <typename type>
                struct setting_base {
                    template <typename _>
                    struct is_family : std::false_type {};
                    template <typename _>
                    struct is_family<setting_base<_>> : std::true_type {};

                    using self_type = type_utils::return_type_t<type, setting_base>;
                    inline self_type &self() { return reinterpret_cast<self_type &>(*this); };
                    
                    template <typename _>
                    operator setting_base<_>&()
                    { return reinterpret_cast<setting_base<_> &>(*this); };
                    
                    template <typename _>
                    operator const setting_base<_>&() const
                    { return reinterpret_cast<const setting_base<_> &>(*this); };

                    inline setting_base(float x, float y,
                                        float width, float height,
                                        const layout::margin &margin = {})
                    : frame(x, y, width, height)
                    , margin(margin) {};
                    
                    inline setting_base(const ofRectangle &frame = {}, const layout::margin &margin = {})
                    : setting_base(frame.x, frame.y, frame.width, frame.height, margin) {};
                    
                    inline setting_base(const setting_base &) = default;
                    inline setting_base(setting_base &&) = default;
                    template <typename _>
                    inline setting_base(const setting_base<_> &setting_)
                    : setting_base(setting_.operator const setting_base &()) {}
                    
                    inline setting_base &operator=(const setting_base &) = default;
                    inline setting_base &operator=(setting_base &&) = default;
                    
                    template <typename ... arguments>
                    inline self_type &setFrame(arguments && ... args) {
                        frame.set(std::forward<arguments>(args) ...);
                        return self();
                    }
                    
                    template <typename ... arguments>
                    inline self_type &setMargin(arguments && ... args) {
                        margin.set(std::forward<arguments>(args) ...);
                        return self();
                    }
                    
                    template <typename ... arguments>
                    inline self_type &setBackgroundColor(arguments && ... vals) {
                        backgroundColor.set(std::forward<arguments>(vals) ...);
                        return self();
                    }
                    
                    inline self_type &setVisible(bool isVisible) {
                        this->isVisible = isVisible;
                        return self();
                    }
                    inline self_type &setEventTransparent(bool isEventTransparent) {
                        this->isEventTransparent = isEventTransparent;
                        return self();
                    }
                    inline self_type &setEnableUserInteraction(bool isEnabledUserInteraction) {
                        this->isEnabledUserInteraction = isEnabledUserInteraction;
                        return self();
                    }
                    inline self_type &setResizable(bool isResizable) {
                        this->isResizable = isResizable;
                        return self();
                    }
                    
                    ofRectangle frame{0.0f, 0.0f, 0.0f, 0.0f};
                    layout::margin margin{0.0f};
                    ofFloatColor backgroundColor{0.0f, 0.0f, 0.0f, 0.0f};
                    float alpha{1.0f};
                    
                    bool isVisible{true};
                    bool isEventTransparent{false};
                    bool isEnabledUserInteraction{true};
                    bool isResizable{false};
                };
                struct traits {
                    template <typename type>
                    struct is_shared_ptr
                    : std::false_type {};
                    template <typename type>
                    struct is_shared_ptr<std::shared_ptr<type>>
                    : std::true_type {};
                    
                    template <typename type_>
                    struct remove_shared_ptr
                    { using type = type_; };
                    template <typename type_>
                    struct remove_shared_ptr<std::shared_ptr<type_>>
                    { using type = type_; };
                    
                    template <typename type>
                    using remove_shared_ptr_t = typename remove_shared_ptr<type>::type;
                };
                
                template <typename inherited_view>
                auto as(const inherited_view &)
                    -> typename enable_if<
                        !traits::is_shared_ptr<inherited_view>::value,
                        std::shared_ptr<inherited_view>
                    >::type
                {
                    return dynamic_pointer_cast<inherited_view>(shared_from_this());
                };
                template <typename inherited_view>
                auto as()
                    -> typename enable_if<
                        !traits::is_shared_ptr<inherited_view>::value,
                        std::shared_ptr<inherited_view>
                    >::type
                {
                    return dynamic_pointer_cast<inherited_view>(shared_from_this());
                };
                template <typename inherited_view_ref>
                auto as(inherited_view_ref = {})
                    -> typename enable_if<
                        traits::is_shared_ptr<inherited_view_ref>::value,
                        inherited_view_ref
                    >::type
                {
                    using type = traits::remove_shared_ptr_t<inherited_view_ref>;
                    return dynamic_pointer_cast<type>(shared_from_this());
                };

                using setting = setting_base<void>;
                
                static view::ref create(float x, float y, float width, float height) {
                    return create({ofRectangle(x, y, width, height)});
                };
                
                static view::ref create(const ofRectangle &rect) {
                    return create(setting{rect});
                }
                
                template <typename _>
                static view::ref create(const setting_base<_> &setting_ = {}) {
                    return std::make_shared<view>(setting_);
                }

                inline view() = default;
                
                inline view(const setting &setting_)
                : setting_(setting_)
                , position(setting_.frame.position)
                , name("view_" + std::to_string(rand()))
                { calculateLayout(); };
                
                inline view(setting &&setting_)
                : setting_(std::move(setting_))
                , position(setting_.frame.position)
                , name("view_" + std::to_string(rand()))
                { calculateLayout(); };
                
                virtual ~view() {
                    unregisterEvents();
                };
                
                inline setting &getSetting() { return setting_; }
                inline const setting &getSetting() const { return setting_; }
                
                inline void setSetting(const setting &setting_) {
                    this->position = setting_.frame.position;
                    this->setting_ = setting_;
                    calculateLayout();
                };
                
                template <typename setting_t>
                inline view &operator=(setting_t &&setting_) {
                    setSetting(std::move(setting_));
                    return *this;
                };
                
#pragma mark operation about subviews
                
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
                
                inline void insert_view_to_front_of(const std::string &name, view::ref v, view::ref target) {
                    if(v->parent.lock()) v->parent.lock()->remove(v);
                    remove(name);
                    
                    v->name = name;
                    v->parent = shared_from_this();
                    
                    auto end = subviews.end();
                    auto it = std::find(subviews.begin(), end, target);
                    if(it == end) {
                        ofLogWarning() << "can't find target from " << subviews.size() << "subview(s)";
                        subviews.emplace_back(v);
                    } else {
                        subviews.insert(it + 1, v);
                    }
                }
                inline void insert_view_to_front_of(const std::string &name, view::ref v, const std::string &target_name)
                { insert_view_to_front_of(name, v, find(target_name)); };
                
                inline void insert_view_to_front_of(view::ref v, view::ref target) {
                    if(v->parent.lock()) v->parent.lock()->remove(v);
                    v->parent = shared_from_this();
                    
                    auto end = subviews.end();
                    auto it = std::find(subviews.begin(), end, target);
                    if(it == end) {
                        ofLogWarning() << "can't find target from " << subviews.size() << "subview(s)";
                        subviews.emplace_back(v);
                    } else {
                        subviews.insert(it + 1, v);
                    }
                }
                inline void insert_view_to_front_of(view::ref v, const std::string &target_name)
                { insert_view_to_front_of(v, find(target_name)); };

                inline void insert_view_to_rear_of(const std::string &name, view::ref v, view::ref target) {
                    if(v->parent.lock()) v->parent.lock()->remove(v);
                    remove(name);
                    
                    v->name = name;
                    v->parent = shared_from_this();
                    
                    auto end = subviews.end();
                    auto it = std::find(subviews.begin(), end, target);
                    if(it == end) {
                        ofLogWarning() << "can't find target from " << subviews.size() << "subview(s)";
                        subviews.emplace_back(v);
                    } else {
                        subviews.insert(it, v);
                    }
                }
                inline void insert_view_to_rear_of(const std::string &name, view::ref v, const std::string &target_name)
                { insert_view_to_rear_of(name, v, find(target_name)); };
                
                inline void insert_view_to_rear_of(view::ref v, view::ref target) {
                    if(v->parent.lock()) v->parent.lock()->remove(v);
                    v->parent = shared_from_this();
                    
                    auto end = subviews.end();
                    auto it = std::find(subviews.begin(), end, target);
                    if(it == end) {
                        ofLogWarning() << "can't find target from " << subviews.size() << "subview(s)";
                        subviews.emplace_back(v);
                    } else {
                        subviews.insert(it, v);
                    }
                }
                
                inline void insert_view_to_rear_of(view::ref v, const std::string &target_name)
                { insert_view_to_rear_of(v, find(target_name)); };
                
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
                        if(v->parent.lock()) v->parent.reset();
                    });
                    subviews.erase(eraser, end);
                }
                
                inline void remove(const view::ref &v) {
                    auto &&eraser = std::remove(subviews.begin(), subviews.end(), v);
                    auto &&end = subviews.end();
//                    std::for_each(eraser, end, [](view::ref v) {
//                        if(v && v->parent.lock()) v->parent.reset();
//                    });
                    subviews.erase(eraser, end);
                }
                
                inline void removeFromParent() {
                    auto &&parent = getParent();
                    if(parent) parent->remove(shared_from_this());
                    else ofLogWarning() << "maybe this view [" << getName() << "] is root";
                }
                
                inline void setOrigin(float x, float y) {
                    position.x = x;
                    position.y = y;
                }
                inline void setOrigin(const ofPoint &p) { setOrigin(p.x, p.y); }
                
                inline bool isShown() const { return getSetting().isVisible; };
                inline void setVisible(bool isVisible) { getSetting().isVisible = isVisible; };
                inline void fadeTo(float alpha,
                                   float duration = 0.3f,
                                   bbb::opt_arg_function<void(const std::string &)> finish = [](const std::string &) {})
                {
                    const std::string animation_name = getName() + "::fade_animation";
                    const float current_alpha = getAlpha();
                    animation::remove(animation_name);
                    animation::add([=](float p) {
                        setAlpha(bbb::pmap(p, current_alpha, alpha));
                    }, duration, animation_name, [=](const std::string &label) {
                        finish(label);
                    });
                };
                inline void show() { setVisible(true); };
                inline void fadeIn(float duration, bbb::opt_arg_function<void(const std::string &)> finish = [](const std::string &) {}) {
                    show();
                    fadeTo(1.0f, duration, finish);
                };
                inline void hide() { setVisible(false); };
                inline void fadeOut(float duration, bbb::opt_arg_function<void(const std::string &)> finish = [](const std::string &) {}) {
                    fadeTo(0.0f, duration, [=](const std::string &label) {
                        finish(label);
                        hide();
                    });
                };
                
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
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p -= v->position - ofPoint(v->getSetting().margin.left, v->getSetting().margin.top);
                    return p;
                }
                
                inline ofPoint convertToGlobalCoordinate(ofPoint p = ofPoint()) const {
                    for(auto v = this; v != nullptr; v = v->parent.lock().get()) p += v->position + ofPoint(v->getSetting().margin.left, v->getSetting().margin.top);
                    return p;
                }
                
                inline void onClickDown(bbb::opt_arg_function<void(mouse_event_arg)> callback) {
                    clickDownCallback = callback;
                }
                
                inline void onClickUp(bbb::opt_arg_function<void(mouse_event_arg)> callback) {
                    clickUpCallback = callback;
                }
                
                inline void onMouseOver(bbb::opt_arg_function<void(mouse_event_arg)> callback) {
                    mouseOverCallback = callback;
                }
                
                inline void onWindowResized(bbb::opt_arg_function<void(resized_event_arg)> callback) {
                    windowResizedCallback = callback;
                }
                
                inline bool isInside(const ofPoint &p) const {
                    auto &margin = getSetting().margin;
                    return ofRectangle(topLeft() + ofPoint(margin.left, margin.top), width - (margin.left + margin.right), height + (margin.top - margin.bottom)).inside(p);
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
                
                inline float getParentAlpha() const { return getParent().get() ? getParent()->getAlpha() : 1.0f; };
                inline float getAlpha() const { return getParentAlpha() * getSetting().alpha; };
                inline float calcAlpha(float alpha = 1.0f) const { return getParentAlpha() * alpha; };
                template <typename float_t>
                inline auto setAlpha(float_t alpha)
                -> typename std::enable_if<std::is_floating_point<float_t>::value>::type
                { getSetting().alpha = alpha; };
                template <typename int_t>
                inline auto setAlpha(int_t alpha)
                -> typename std::enable_if<std::is_integral<int_t>::value>::type
                { getSetting().alpha = alpha / 255.0f; };

                inline const std::string &getName() const & { return name; };
                inline std::string &&getName() && { return std::move(name); };
                
                inline ofPoint &getPosition() { return position; };
                inline ofPoint getPosition() const { return position; };
                inline void setPosition(float x, float y) { position.set(x, y); }
                inline void setPosition(const ofPoint &p) { position = p; }
                inline void moveTo(const ofPoint &p) { position = p; }
                inline void moveTo(float x, float y) { position.set(x, y); }
                inline void move(const ofPoint &p) { position += p; }
                inline void move(float x, float y) { position.x += x, position.y += y; }
                
                inline const ofRectangle &getFrame() const { return setting_.frame; };
                inline ofRectangle getBounds() const { return {0, 0, width, height}; };
                
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
                
                inline void setSize(float width, float height) {
                    getSetting().frame.width = width;
                    getSetting().frame.height = height;
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
                template <typename type>
                inline auto getSubviewAs(const std::string &name)
                    -> decltype(find(name)->as<type>())
                { return find(name)->as<type>(); };

                inline float left() const { return convertToGlobalCoordinate().x; };
                inline float right() const { return left() + width; };
                inline float top() const { return convertToGlobalCoordinate().y; };
                inline float bottom() const { return top() + height; };
                
                inline void setLeft(float left) { getPosition().x = left; };
                inline void setRight(float right) { getPosition().x = right - getWidth(); };
                inline void setTop(float top) { getPosition().y = top; };
                inline void setBottom(float bottom) { getPosition().y = bottom - getHeight(); };

                inline void setLeftStretch(float left) {
                    float l = getPosition().x;
                    setLeft(left);
                    setWidth(getWidth() + l - left);
                };
                inline void setRightStretch(float right) {
                    float r = getPosition().x + getWidth();
                    setRight(right);
                    setWidth(getWidth() + right - r);
                };
                inline void setTopStretch(float top) {
                    float t = getPosition().y;
                    setTop(top);
                    setHeight(getHeight() + t - top);
                };
                inline void setBottomStretch(float bottom) {
                    float b = getPosition().y + getHeight();
                    setBottom(bottom);
                    setHeight(getHeight() + bottom - b);
                };
                
                inline float horizontalCenter() const { return left() + width * 0.5f; };
                inline float verticalCenter() const { return top() + height * 0.5f; };
                
                inline ofPoint topLeft() const { return convertToGlobalCoordinate(); };
                inline ofPoint topRight() const { return topLeft() + ofPoint(width, 0); };
                inline ofPoint bottomLeft() const { return topLeft() + ofPoint(0, height); };
                inline ofPoint bottomRight() const { return topLeft() + ofPoint(width, height); };
                inline ofPoint center() const { return topLeft() + ofPoint(width * 0.5f, height * 0.5f); };
                
                virtual void draw() {
                    if(!isShown()) return;
                    pushState();
                    ofTranslate(position + ofPoint(getSetting().margin.left, getSetting().margin.top));
                    drawBackground();
                    drawInternal();
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
                
                void setForegroundColor(int r, int g, int b, int a = 255) {
                    ofSetColor(r, g, b, getAlpha() * a);
                }
                void setForegroundColor(int gray, int a = 255) {
                    ofSetColor(gray, gray, gray, getAlpha() * a);
                }

            protected:
                setting setting_;
                bool isClickedNow_{false};
                std::shared_ptr<ofPoint> clickedPoint_;
                
                inline void mousePressed(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    clickDown(ofPoint(arg.x, arg.y));
                }
                inline void mouseReleased(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    clickUp(ofPoint(arg.x, arg.y));
                }
                inline void mouseMoved(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    mouseOver(ofPoint(arg.x, arg.y));
                }
                inline void mouseDragged(ofMouseEventArgs &arg) {
                    if(!isShown()) return;
                    mouseOver(ofPoint(arg.x, arg.y));
                }
                inline void windowResizedRoot(ofResizeEventArgs &arg) {
                    windowResized({shared_from_this(), {ofPoint(), (float)arg.width, (float)arg.height}});
                }
                
                inline bool clickDown(const ofPoint &p) {
                    if(!isShown()) return false;
                    for(auto subview = subviews.rbegin();
                        subview != subviews.rend();
                        ++subview) if((*subview)->clickDown(p)) return true;
                    if(isEnabledUserInteraction() && isInside(p)) {
                        if(!isClickedNow_) clickedPoint_ = std::make_shared<ofPoint>(p);
                        isClickedNow_ = true;
                        clickDownCallback({shared_from_this(), p, false});
                        return !isEventTransparent();
                    }
                    return false;
                }
                
                inline bool clickUp(const ofPoint &p) {
                    bool wasClicked = isClickedNow_;
                    isClickedNow_ = false;
                    clickedPoint_.reset();
                    
                    if(!isShown()) return false;
                    for(auto subview = subviews.rbegin();
                        subview != subviews.rend();
                        ++subview) if((*subview)->clickUp(p)) return true;
                    if((isEnabledUserInteraction() && isInside(p)) || wasClicked) {
                        clickUpCallback({shared_from_this(), p, false});
                        return !isEventTransparent();
                    }
                    return false;
                }
                
                inline void mouseOver(const ofPoint &p) {
                    if(!isShown()) return;
                    for(auto subview = subviews.rbegin();
                        subview != subviews.rend();
                        ++subview) (*subview)->mouseOver(p);
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
                
                virtual void drawInternal() {};
                
                inline void windowResized(resized_event_arg super_arg) {
                    windowResizeInternal(super_arg);
                    for(auto &&subview : subviews) {
                        subview->windowResized({subview, {position, width, height}});
                    }
                }
                
                virtual void windowResizeInternal(resized_event_arg arg) {
                    windowResizedCallback(arg);
                };
                
                ofPoint position;
                float width;
                float height;
                
                bbb::opt_arg_function<void(mouse_event_arg)> clickDownCallback{mouse_default};
                bbb::opt_arg_function<void(mouse_event_arg)> clickUpCallback{mouse_default};
                bbb::opt_arg_function<void(mouse_event_arg)> mouseOverCallback{mouse_default};
                bbb::opt_arg_function<void(mouse_event_arg)> draggedCallback{mouse_default};
                
                bbb::opt_arg_function<void(resized_event_arg)> windowResizedCallback{resized_default};
                
                std::string name{""};
                std::vector<view::ref> subviews;
                std::weak_ptr<view> parent{};
            };
            
            namespace { // make static
                void mouse_default(mouse_event_arg) {};
                void resized_default(resized_event_arg arg) {};
                void fitToParent(resized_event_arg arg) {
                    arg.target->setSize(arg.rect.width, arg.rect.height);
                };
            };
        }; // components
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_view_hpp */
