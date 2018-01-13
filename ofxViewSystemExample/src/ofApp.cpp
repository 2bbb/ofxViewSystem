#include "ofMain.h"
#include "view_system.hpp"

class ofApp : public ofBaseApp {
    bbb::vs::view::ref root;

    void setupRootView() {
        root = bbb::vs::view::create(20, 20, ofGetWidth() - 40, ofGetHeight() - 40);
        root->setBackgroundColor(255, 0, 0, 128);
        root->onClickDown([](bbb::vs::view &v, const ofPoint &p) {
            ofLogNotice() << "root: " << p << " / " << v.convertToLocalCoordinate(p);
            auto &c = v.getBackgroundColor();
            c.r = 255 - c.r;
            c.b = 255 - c.b;
        });
        root->registerEvent();
    }
    
    bbb::vs::view::ref createSubView() const {
        static const char subview_moving_animation_name[] = "subview_move";
        
        auto &&subview = bbb::vs::drawer::create(80, 80, root->getWidth() - 160, root->getHeight() - 160);
        subview->setEventTransparentness(true);
        subview->setBackgroundColor(255, 255, 255, 128);
        subview->setDrawCallback([](const bbb::vs::drawer &v) {
            float w = v.getWidth();
            float h = v.getHeight();
            float d = (w - h) * 0.5f;
            ofSetColor(0, 255, 0);
            float t = ofGetElapsedTimef();
            for(float x = 0.0f; x < 2.0f * M_PI; x += 0.01f) ofDrawLine((0.5f * std::cos(0.3f * x + 1.3f * t) + 0.5f) * v.getHeight() + d,
                                                                        (0.5f * std::sin(1.4f * x + 0.4f * t) + 0.5f) * v.getHeight(),
                                                                        (0.5f * std::cos(1.9f * x + 0.7f * t) + 0.5f) * v.getHeight() + d,
                                                                        (0.5f * std::sin(1.7f * x + 2.5f * t) + 0.5f) * v.getHeight());
        });
        
        subview->onClickDown([=](bbb::vs::view &v, const ofPoint &p) {
            auto &&blue_pos = v.getPosition();
            float x = root->left();
            float from_y = root->top();
            float to_y = -blue_pos.y;
            
            if(from_y == to_y) {
                to_y = 20;
            }
            
            bbb::vs::animation::remove(subview_moving_animation_name);
            bbb::vs::animation::add([=](float progress) {
                progress = bbb::vs::easing::exp::in_out(progress);
                this->root->moveTo(x, ofMap(progress, 0.0f, 1.0f, from_y, to_y));
            }, 0.3f, 0.0f, subview_moving_animation_name);
            
            ofLogNotice() << "subview: " << p << " / " << v.convertToLocalCoordinate(p);
        });
        
        return subview;
    }
    
public:
    void setup() {
        setupRootView();
        root->add("subview", createSubView());
        ofSetBackgroundColor(0, 0, 0);
    }
    void draw() {
        root->draw();
    }
};

int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new ofApp());
}

