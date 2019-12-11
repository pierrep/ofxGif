#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    bLoaded = gifloader.load("hand.gif");
    index = 0;
    ofSetFrameRate(2);
    ofSetBackgroundAuto(false);
}

//--------------------------------------------------------------
void ofApp::update()
{
    //if ((ofGetElapsedTimeMillis() % 30) == 0)
    {
        index++;
        if (index > gifloader.pages.size() - 1)
            index = 0;
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
   // ofBackground(0);

    ofDrawBitmapString("index = " + ofToString(index), 20, ofGetHeight() - 20);
    for (int i = 0; i < index; i++) {
        if (bLoaded) {
            ofPushMatrix();
            ofScale(0.5);
            gifloader.pages[i].draw(0, 0);
            ofPopMatrix();
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}
