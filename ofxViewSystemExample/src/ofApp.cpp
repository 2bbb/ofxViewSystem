#include "ofMain.h"
#include "view_system.hpp"

constexpr char subview_tag[] = "subview";
constexpr char close_button_tag[] = "close_button";

struct CustomView : public bbb::vs::view {
    CustomView(const bbb::vs::view::setting &setting)
    : bbb::vs::view(setting)
    {}
    virtual void drawInternal() override {
        const float w = getWidth(), h = getHeight(), d = (w - h) * 0.5f;
        ofSetColor(0, 255, 0, getAlpha() * 255.0f);
        const float t = ofGetElapsedTimef();
        for(float x = 0.0f; x < 2.0f * M_PI; x += 0.01f) {
            const float z = std::cos(1.3 * x + 1.1f * t) * 0.5f;
            ofDrawLine((z * std::cos(0.3f * x + 1.3f * t) + 0.5f) * h + d,
                       (z * std::sin(1.4f * x + 0.4f * t) + 0.5f) * h,
                       (z * std::cos(1.9f * x + 0.7f * t) + 0.5f) * h + d,
                       (z * std::sin(1.7f * x + 2.5f * t) + 0.5f) * h);
        }
    }
    virtual void windowResizeInternal(bbb::vs::resized_event_arg arg) override {
        auto &&v = arg.target;
        v->setSize(arg.rect.width, arg.rect.height);
        v->getSubview(close_button_tag)->setPosition({v->getWidth() - 10.0f, -10.0f});
    }
};

class ofApp : public ofBaseApp {
    bbb::vs::view::ref root;
    
    void setupRootView() {
        bbb::vs::view::setting setting = bbb::vs::view::setting(0.0f, 0.0f, ofGetWidth(), ofGetHeight())
            .setBackgroundColor(1.0f, 0.0f, 0.0f, 0.5f)
            .setMargin(20.0f);
        
        root = bbb::vs::view::create(setting);
        root->onClickDown([](bbb::vs::mouse_event_arg arg) {
            auto &&v = arg.target;
            auto &&p = arg.p;
            auto &&c = v->getBackgroundColor();
            c.r = 255 - c.r;
            c.b = 255 - c.b;
        });
        root->onWindowResized(bbb::vs::fitToParent);
        root->onMouseOver([](bbb::vs::mouse_event_arg arg) {
            auto &&v = arg.target;
            if(v->isClickedNow()) {
                v->move(v->mouseDelta());
            }
        });
        root->registerEvents();
    }
    
    bbb::vs::view::ref createCloseButton() const {
        bbb::vs::drawer::setting setting = bbb::vs::drawer::setting(ofRectangle(0, 0, 20, 20))
            .setBackgroundColor(0.0f, 0.0f, 0.0f);
        
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
        bbb::vs::view::setting setting = bbb::vs::view::setting(ofRectangle(0, 0, root->getWidth(), root->getHeight()))
            .setMargin(80.0f)
            .setBackgroundColor(1.0f, 1.0f, 1.0f, 0.5f)
            .setEventTransparent(true);
        
        auto &&subview = std::make_shared<CustomView>(setting);
        
        auto &&closeButton = createCloseButton();
        closeButton->setPosition({subview->getWidth() - 10.0f, -10.0f});
        subview->add(close_button_tag, closeButton);

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
                root->setPosition(0, 0);
                addSubview();
                break;
            default:
                break;
        }
    }
};

int main() {
    ofGLFWWindowSettings setting;
    setting.width = 1280;
    setting.height = 720;
    auto window = ofCreateWindow(setting);
    auto app = std::make_shared<ofApp>();
    ofRunApp(window, app);
    ofRunMainLoop();
}
