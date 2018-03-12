//
//  components/drawer.hpp
//
//  Created by ISHII 2bit on 2018/02/21.
//

#pragma once

#ifndef bbb_components_drawer_hpp
#define bbb_components_drawer_hpp

#include "./view.hpp"

namespace bbb {
    namespace view_system {
        inline namespace components {
            struct drawer : public view {
                using ref = std::shared_ptr<drawer>;
                using const_ref = std::shared_ptr<const drawer>;
                using drawCallback = bbb::opt_arg_function<void(drawer::const_ref)>;
                
                template <typename type>
                struct setting_base : public view::setting_base<setting_base<type>> {
                    using super_type = view::setting_base<setting_base<type>>;
                    
					setting_base() {};
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
                    
                    template <typename _>
                    operator view::setting_base<_>&()
                    { return reinterpret_cast<view::setting_base<_> &>(*this); }
                    
                    template <typename _>
                    operator const view::setting_base<_>&() const
                    { return reinterpret_cast<const view::setting_base<_> &>(*this); }
                    
                    using super_type::super_type;
                    inline setting_base(drawCallback callback,
                                        const ofRectangle &frame = {},
                                        const layout::margin &margin = {})
                    : super_type(frame, margin)
                    , callback(callback) {};
                    
                    inline setting_base(const setting_base &) = default;
                    inline setting_base(setting_base &&) = default;
                    
                    using super_type::operator=;
                    inline setting_base &operator=(const setting_base &) = default;
                    inline setting_base &operator=(setting_base &&) = default;
                    
                    inline self_type &setDrawer(drawCallback callback) {
                        this->callback = callback;
                        return self();
                    }
                    drawCallback callback{[](drawer::const_ref){}};
                };
                
                using setting = setting_base<void>;
                
                inline static drawer::ref create(float x, float y, float width, float height) {
                    return create({ofRectangle(x, y, width, height)});
                }
                
                inline static drawer::ref create(const ofRectangle &rect) {
                    return create({rect});
                }
                
                inline static drawer::ref create(const drawer::setting &setting_ = drawer::setting()) {
                    return std::make_shared<drawer>(setting_);
                }
                
                inline static drawer::ref create(drawCallback callback, float x, float y, float width, float height) {
                    return create(callback, {ofRectangle(x, y, width, height)});
                }
                
                inline static drawer::ref create(drawCallback callback, const ofRectangle &rect) {
                    return create(callback, {rect});
                };
                
                inline static drawer::ref create(drawCallback callback, const setting &setting_) {
                    return std::make_shared<drawer>(callback, setting_);
                };
                
                inline drawer(drawCallback callback, const setting &setting_ = setting())
                : view(setting_)
                , callback(callback)
                {};
                inline drawer(const setting &setting_ = setting())
                : view(setting_)
                {};
                
                virtual ~drawer() {};
                
                inline setting &getSetting() { return setting_; }
                inline const setting &getSetting() const { return setting_; }
                
                using view::setSetting;
                inline void setSetting(const setting &setting_) {
                    this->setting_.callback = setting_.callback;
                    view::setSetting(setting_);
                };
                inline void setSetting(setting &&setting_) {
                    this->setting_.callback = std::move(setting_.callback);
                    view::setSetting(std::move(setting_));
                };
                
                using view::operator=;
                inline drawer &operator=(const setting &setting_) {
                    setSetting(setting_);
                    return *this;
                };
                inline drawer &operator=(setting &&setting_) {
                    setSetting(std::move(setting_));
                    return *this;
                };
                
#pragma mark specific
                
                virtual void drawInternal() override {
                    callback(std::dynamic_pointer_cast<const drawer>(shared_from_this()));
                }
                
                inline void onDraw(drawCallback callback) {
                    this->callback = callback;
                }
                
            protected:
                setting setting_;
                drawCallback callback{[](drawer::const_ref){}};
            };
        }; // components
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_drawer_hpp */
