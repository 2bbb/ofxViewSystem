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
                using drawCallback = std::function<void(drawer::const_ref)>;
                
                template <typename type>
                struct setting_base : public view::setting_base<setting_base<type>> {
                    using super_type = view::setting_base<setting_base<type>>;
                    using self_type = type_utils::return_type_t<type, setting_base>;
                    inline self_type &self() { return reinterpret_cast<self_type &>(*this); };
                    
                    using super_type::super_type;
                    
                    inline setting_base(drawCallback callback,
                                        const ofRectangle &frame = {},
                                        const layout::margin &margin = {})
                    : super_type(frame, margin)
                    , callback(callback) {};
                    
                    inline setting_base(const setting_base &) = default;
                    template <typename _>
                    inline setting_base(const setting_base<_> &setting) { operator=(setting); };
                    
                    inline setting_base &operator=(const setting_base &) = default;
                    inline setting_base &operator=(setting_base &&) = default;
                    template <typename _>
                    inline setting_base &operator=(const setting_base<_> &setting) {
                        super_type::operator=(setting);
                        callback = setting.callback;
                        return *this;
                    }
                    
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
                
                inline static drawer::ref create(drawCallback callback, float x, float y, float width, float height) {
                    return create(callback, {ofRectangle(x, y, width, height)});
                }
                
                inline static drawer::ref create(drawCallback callback, const ofRectangle &rect) {
                    return create(callback, {rect});
                };
                
                inline static drawer::ref create(drawCallback callback, const setting &setting_) {
                    return std::make_shared<drawer>(callback, setting_);
                };
                
                inline static drawer::ref create(const drawer::setting &setting_ = {}) {
                    return std::make_shared<drawer>(setting_);
                }
                
                inline drawer(drawCallback callback, const setting &setting_ = {})
                : view(setting_)
                , callback(callback)
                {};
                inline drawer(const setting &setting_ = {})
                : view(setting_)
                {};
                
                virtual ~drawer() {};
                
                inline setting &getSetting() { return setting_; }
                inline const setting &getSetting() const { return setting_; }
                
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
