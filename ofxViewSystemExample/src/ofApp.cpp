#include "ofMain.h"
#include "view_system.hpp"

constexpr char subview_tag[] = "subview";
constexpr char close_button_tag[] = "close_button";

class ofApp : public ofBaseApp {
    bbb::vs::view::ref root;
    
    void setupRootView() {
        root = bbb::vs::view::create(20, 20, ofGetWidth() - 40, ofGetHeight() - 40);
        root->setBackgroundColor(255, 0, 0, 128);
        root->onClickDown([](bbb::vs::mouse_event_arg arg) {
            auto v = arg.target;
            auto &&p = arg.p;
            ofLogNotice() << "root: " << p << " / " << v->convertToLocalCoordinate(p);
            auto &c = v->getBackgroundColor();
            c.r = 255 - c.r;
            c.b = 255 - c.b;
        });
        root->onWindowResized([](bbb::vs::resized_event_arg arg) {
            auto v = arg.target;
            v->getWidth() = arg.rect.width - 40;
            v->getHeight() = arg.rect.height - 40;
        });
        root->onMouseOver([](bbb::vs::mouse_event_arg arg) {
            auto v = arg.target;
            if(v->isClickedNow()) {
                v->move(v->mouseDelta());
            }
        });
        root->registerEvents();
    }
    
    bbb::vs::view::ref createCloseButton() const {
        auto &&button = bbb::vs::drawer::create(0, 0, 20, 20);
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
        button->setBackgroundColor(0, 0, 0);
        return button;
    }
    
    bbb::vs::view::ref createSubView() const {
        static const char subview_moving_animation_name[] = "subview_move";
        
        auto &&subview = bbb::vs::drawer::create(80, 80, root->getWidth() - 160, root->getHeight() - 160);
        subview->setEventTransparentness(true);
        subview->setBackgroundColor(255, 255, 255, 128);
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
            v->getWidth() = arg.rect.width - 160;
            v->getHeight() = arg.rect.height - 160;
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
