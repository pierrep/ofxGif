#pragma once

#include "FreeImage.h"
#include "ofMain.h"

class ofxGif {
public:
    void create(string filename);
    void save();

    void append(string filename); // not alpha channel
    void append(ofPixels& pixels);

    bool load(string filename);

    vector<ofImage> pages;

protected:
    // Pointer to a multi-page file stream
    FIMULTIBITMAP* gif = nullptr;
};
