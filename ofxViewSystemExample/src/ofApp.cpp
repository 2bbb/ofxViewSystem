#include "ofMain.h"
#include "view_system.hpp"

constexpr char subview_tag[] = "subview";
constexpr char close_button_tag[] = "close_button";

class ofApp : public ofBaseApp {
    bbb::vs::view::ref root;
    
    void setupRootView() {
        bbb::vs::view::setting setting(0, 0, ofGetWidth(), ofGetHeight());
        setting.backgroundColor.set(1.0f, 0.0f, 0.0f, 0.5f);
        setting.margin.set(20, 20, 20, 20);
        root = bbb::vs::view::create(setting);
        root->onClickDown([](bbb::vs::mouse_event_arg arg) {
            auto v = arg.target;
            auto &&p = arg.p;
            ofLogNotice() << "root: " << p << " / " << v->convertToLocalCoordinate(p);
            auto &c = v->getBackgroundColor();
            c.r = 255 - c.r;
            c.b = 255 - c.b;
        });
        root->onWindowResized(bbb::vs::fitToParent);
        root->onMouseOver([](bbb::vs::mouse_event_arg arg) {
            auto v = arg.target;
            if(v->isClickedNow()) {
                v->move(v->mouseDelta());
            }
        });
        root->registerEvents();
    }
    
    bbb::vs::view::ref createCloseButton() const {
        bbb::vs::drawer::setting setting(ofRectangle(0, 0, 20, 20));
        setting.backgroundColor.set(0.0f, 0.0f, 0.0f);
        auto &&button = bbb::vs::drawer::create(setting);
        button->onClickDown([](bbb::vs::mouse_event_arg arg) {
            auto &&v = arg.target;
            auto &&subview = v->getParent();
            float alpha = subview->getAlpha();
            bbb::vs::animation::add([=](float progress) {
                subview->setAlpha(ofMap(progress, 0.0f, 1.0f, alpha, 0.0f));
            }, 0.5f, 0.0f, "remove_subview", [=](const std::string &name) {
                subview->removeFromParent();
            });
        });
        button->onDraw([](bbb::vs::drawer::const_ref drawer) {
            ofSetColor(255);
            ofDrawLine(0, 0, drawer->getWidth(), drawer->getHeight());
            ofDrawLine(drawer->getWidth(), 0, 0, drawer->getHeight());
        });
        return button;
    }
    
    bbb::vs::view::ref createSubView() const {
        static const char subview_moving_animation_name[] = "subview_move";
        bbb::vs::drawer::setting setting(ofRectangle(0, 0, root->getWidth(), root->getHeight()));
        setting.margin = {80.0f};
        setting.backgroundColor.set(1.0f, 1.0f, 1.0f, 0.5f);
        setting.isEventTransparent = true;
        
        auto &&subview = bbb::vs::drawer::create(setting);
        subview->onDraw([](bbb::vs::drawer::const_ref v) {
            const float w = v->getWidth(), h = v->getHeight(), d = (w - h) * 0.5f;
            ofSetColor(0, 255, 0, v->getAlpha() * 255.0f);
            const float t = ofGetElapsedTimef();
            for(float x = 0.0f; x < 2.0f * M_PI; x += 0.01f) {
                const float z = std::cos(1.3 * x + 1.1f * t) * 0.5f;
                ofDrawLine((z * std::cos(0.3f * x + 1.3f * t) + 0.5f) * h + d,
                           (z * std::sin(1.4f * x + 0.4f * t) + 0.5f) * h,
                           (z * std::cos(1.9f * x + 0.7f * t) + 0.5f) * h + d,
                           (z * std::sin(1.7f * x + 2.5f * t) + 0.5f) * h);
            }
        });
        
        auto &&closeButton = createCloseButton();
        closeButton->setPosition({subview->getWidth() - 10.0f, -10.0f});
        
        subview->add(close_button_tag, closeButton);
        subview->onWindowResized([](bbb::vs::resized_event_arg arg) {
            auto v = arg.target;
            v->setSize(arg.rect.width, arg.rect.height);
            v->getSubview(close_button_tag)->setPosition({v->getWidth() - 10.0f, -10.0f});
        });

        return subview;
    }
    
    void addSubview() {
        if(root->find(subview_tag)) return;
        root->add(subview_tag, createSubView());
    }
    
public:
    void setup() override {
        setupRootView();
        addSubview();
        ofSetBackgroundColor(0, 0, 0);
    }
    void draw() override {
        root->draw();
    }
    void keyPressed(int key) override {
        switch(key) {
            case 'F':
                ofToggleFullscreen();
                break;
            case 'R':
                addSubview();
                break;
            default:
                break;
        }
    }
};

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new ofApp());
}
