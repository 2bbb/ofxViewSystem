//
//  components/image.hpp
//
//  Created by ISHII 2bit on 2018/02/21.
//

#ifndef bbb_components_image_hpp
#define bbb_components_image_hpp

#include "./view.hpp"

namespace bbb {
    namespace view_system {
        inline namespace components {
            struct image : public view {
                using ref = std::shared_ptr<image>;
                
                template <typename type>
                struct setting_base : public view::setting_base<setting_base<type>> {
                    using super_type = view::setting_base<setting_base<type>>;
                    using self_type = type_utils::return_type_t<type, setting_base>;
                    inline self_type &self() { return reinterpret_cast<self_type &>(*this); };
                    
                    using super_type::super_type;
                    
                    inline setting_base(const boost::filesystem::path &imagePath,
                                        const ofRectangle &frame = {},
                                        const layout::margin &margin = {})
                    : super_type(frame, margin)
                    , imagePath(imagePath) {};
                    
                    inline setting_base(const setting_base &) = default;
                    template <typename _>
                    inline setting_base(const setting_base<_> &setting) { operator=(setting); };
                    
                    inline setting_base &operator=(const setting_base &) = default;
                    inline setting_base &operator=(setting_base &&) = default;
                    template <typename _>
                    inline setting_base &operator=(const setting_base<_> &setting) {
                        super_type::operator=(setting);
                        imagePath = setting.imagePath;
                        return *this;
                    }
                    
                    inline self_type &setImagePath(const boost::filesystem::path &imagePath) {
                        this->imagePath = imagePath;
                        return self();
                    };
                    boost::filesystem::path imagePath{""};
                };
                
                using setting = setting_base<void>;
                
                inline static image::ref create(const setting &setting_ = setting()) {
                    return std::make_shared<image>(setting_);
                }
                
                inline static image::ref create(const ofRectangle &rect) {
                    return create(setting(rect));
                }
                
                inline static image::ref create(float x, float y,
                                                float width, float height)
                {
                    return create(ofRectangle(x, y, width, height));
                }
                
                inline static image::ref create(const ofImage &image_,
                                                const setting &setting_ = setting())
                {
                    return std::make_shared<image>(image_, setting_);
                };
                
                inline static image::ref create(const ofImage &image_,
                                                const ofRectangle &rect)
                {
                    return create(image_, setting(rect));
                };
                
                inline static image::ref create(const ofImage &image_,
                                                float x, float y,
                                                float width, float height)
                {
                    return create(image_, ofRectangle(x, y, width, height));
                }
                
                inline static image::ref create(const boost::filesystem::path &imagePath,
                                                const setting &setting_ = setting())
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
                inline image(const ofImage &image_, const setting &setting_)
                : view(setting_)
                , setting_(setting_)
                , image_(image_)
                {};
                inline image(const setting &setting_)
                : view(setting_)
                , setting_(setting_)
                , image_(setting_.imagePath)
                {};
                
                virtual ~image() {};
                
                inline setting &getSetting() { return setting_; }
                inline const setting &getSetting() const { return setting_; }
                
                inline void fitToImage() {
                    setWidth(image_.getWidth());
                    setHeight(image_.getHeight());
                }
                
                virtual void drawInternal() override {
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
                    setting_.imagePath = path;
                    return image_.load(path);
                }
                
            protected:
                setting setting_;
                ofImage image_;
            };
        }; // components
    }; // view_system
    namespace vs = view_system;
}; // bbb

#endif /* bbb_components_image_hpp */
