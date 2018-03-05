//
//  components/image.hpp
//
//  Created by ISHII 2bit on 2018/02/21.
//

#pragma once

#ifndef bbb_components_image_hpp
#define bbb_components_image_hpp

#include "./view.hpp"

namespace bbb {
    namespace view_system {
        inline namespace components {
            struct image : public view {
                using ref = std::shared_ptr<image>;
                using const_ref = std::shared_ptr<const image>;
                using image_ref = std::shared_ptr<ofImage>;
                
                template <typename type>
                struct setting_base : public view::setting_base<setting_base<type>> {
                    template <typename _>
                    struct is_family : std::false_type {};
                    template <typename _>
                    struct is_family<setting_base<_>> : std::true_type {};
                    
                    using super_type = view::setting_base<setting_base<type>>;
                    using self_type = type_utils::return_type_t<type, setting_base>;
                    inline self_type &self() { return reinterpret_cast<self_type &>(*this); };
                    
                    template <typename _>
                    operator setting_base<_>&()
                    { return reinterpret_cast<setting_base<_> &>(*this); };
                    
                    template <typename _>
                    operator const setting_base<_>&() const
                    { return reinterpret_cast<setting_base<_> &>(*this); };
                    
                    template <typename _>
                    operator view::setting_base<_>&()
                    { return reinterpret_cast<view::setting_base<_> &>(*this); }
                    
                    template <typename _>
                    operator const view::setting_base<_>&() const
                    { return reinterpret_cast<const view::setting_base<_> &>(*this); }
                    
                    using super_type::super_type;
                    inline setting_base(const boost::filesystem::path &imagePath,
                                        const ofRectangle &frame = {},
                                        const layout::margin &margin = {})
                    : super_type(frame, margin)
                    , imagePath(imagePath) {};
                    
                    inline setting_base(const setting_base &) = default;
                    inline setting_base(setting_base &&) = default;
                    
                    using super_type::operator=;
                    inline setting_base &operator=(const setting_base &) = default;
                    inline setting_base &operator=(setting_base &&) = default;
                    
                    inline self_type &setImagePath(const boost::filesystem::path &imagePath) {
                        this->imagePath = imagePath;
                        return self();
                    };
                    boost::filesystem::path imagePath{""};
                };
                
                using setting = setting_base<void>;
                
                inline static image::ref create() { return std::make_shared<image>(); }
                
                template <typename _>
                inline static image::ref create(const view::setting_base<_> &setting_)
                { return std::make_shared<image>(setting_); }
                
                template <typename _>
                inline static image::ref create(const setting_base<_> &setting_)
                { return std::make_shared<image>(setting_); }
                
                inline static image::ref create(const ofRectangle &rect)
                { return create(setting(rect)); }
                
                inline static image::ref create(float x, float y,
                                                float width, float height)
                { return create(ofRectangle(x, y, width, height)); }
                
                template <typename _>
                inline static image::ref create(const ofImage &image_,
                                                const view::setting_base<_> &setting_ = {})
                { return std::make_shared<image>(image_, setting_); };
                
                template <typename _>
                inline static image::ref create(const ofImage &image_,
                                                const setting_base<_> &setting_ = {})
                { return std::make_shared<image>(image_, setting_); };
                
                inline static image::ref create(const ofImage &image_,
                                                const ofRectangle &rect)
                { return create(image_, setting(rect)); };
                
                inline static image::ref create(const ofImage &image_,
                                                float x, float y,
                                                float width, float height)
                { return create(image_, ofRectangle(x, y, width, height)); };
                
                template <typename _>
                inline static image::ref create(const boost::filesystem::path &imagePath,
                                                const view::setting_base<_> &setting_ = {})
                {
                    setting setting__(setting_);
                    setting__.imagePath = imagePath;
                    return std::make_shared<image>(setting__);
                };
                
                template <typename _>
                inline static image::ref create(const boost::filesystem::path &imagePath,
                                                const setting_base<_> &setting_ = {})
                {
                    setting setting__(setting_);
                    setting__.imagePath = imagePath;
                    return std::make_shared<image>(setting__);
                };
                
                inline static image::ref create(const boost::filesystem::path &imagePath,
                                                const ofRectangle &rect)
                {
                    return create(imagePath, setting(rect));
                };
                
                inline static image::ref create(const boost::filesystem::path &imagePath,
                                                float x, float y,
                                                float width, float height)
                {
                    return create(imagePath, ofRectangle(x, y, width, height));
                }
                
                inline image() = default;
                
                inline image(const ofImage &image_, const view::setting &setting_)
                : view(setting_)
                , setting_(setting_)
                , image_(new ofImage(image_))
                {};
                
                inline image(const ofImage &image_, const setting &setting_)
                : view(setting_)
                , setting_(setting_)
                , image_(new ofImage(image_))
                {};
                
                inline image(const view::setting &setting_)
                : view(setting_)
                , setting_(setting_)
                {};
                
                inline image(const setting &setting_)
                : view(setting_)
                , setting_(setting_)
                , image_(setting_.imagePath == "" ? new ofImage() : new ofImage(setting_.imagePath))
                {};
                
                virtual ~image() {};
                
                inline setting &getSetting() { return setting_; }
                inline const setting &getSetting() const { return setting_; }
                
                using view::setSetting;
                inline void setSetting(const setting &setting_) {
                    this->setting_.imagePath = setting_.imagePath;
                    load(setting_.imagePath);
                    view::setSetting(setting_);
                };
                inline void setSetting(setting &&setting_) {
                    this->setting_.imagePath = std::move(setting_.imagePath);
                    load(setting_.imagePath);
                    view::setSetting(std::move(setting_));
                };
                
                using view::operator=;
                inline image &operator=(const setting &setting_) {
                    setSetting(setting_);
                    return *this;
                };
                inline image &operator=(setting &&setting_) {
                    setSetting(std::move(setting_));
                    return *this;
                };
                
#pragma mark specific
                
                inline void fitToImage() {
                    if(image_) {
                        setWidth(image_->getWidth());
                        setHeight(image_->getHeight());
                    } else {
                        ofLogWarning() << "image isn't load";
                    }
                }
                
                virtual void drawInternal() override {
                    if(image_ && image_->isAllocated()) {
                        ofSetColor(ofColor::white, getParentAlpha() * 255.0f);
                        image_->draw(0.0f, 0.0f, width, height);
                    }
                }
                
                inline const image_ref &getImageRef() const & { return image_; };
                inline image_ref &&getImageRef() && { return std::move(image_); };
                inline operator const image_ref &() const & { return image_; };
                inline operator image_ref &&() && { return std::move(image_); };
                
//                inline const ofImage &getImage() const & { return *image_; };
//                inline ofImage &&getImage() && { return std::move(*image_); };
//                inline operator const ofImage &() const & { return *image_; };
//                inline operator ofImage &&() && { return std::move(*image_); };
                
                inline bool load(const boost::filesystem::path &path) {
                    setting_.imagePath = path;
                    if(!image_) image_ = std::make_shared<ofImage>();
                    return image_->load(path);
                }
                
                inline void setImage(image_ref image_) { this->image_ = image_; }
                
            protected:
                setting setting_;
                std::shared_ptr<ofImage> image_;
            };
        }; // components
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_image_hpp */
