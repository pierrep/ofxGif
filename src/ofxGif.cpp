#include "ofxGif.h"

namespace ofxGIF {

//----------------------------------------------------------
// static variable for freeImage initialization:
void ofInitFreeImage(bool deinit = false)
{
    // need a new bool to avoid c++ "deinitialization order fiasco":
    // http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.15
    static bool* bFreeImageInited = new bool(false);
    if (!*bFreeImageInited && !deinit) {
        FreeImage_Initialise();
        *bFreeImageInited = true;
    }
    if (*bFreeImageInited && deinit) {
        FreeImage_DeInitialise();
    }
}

template <typename T>
FREE_IMAGE_TYPE getFreeImageType(const ofPixels_<T>& pix);

template <>
FREE_IMAGE_TYPE getFreeImageType(const ofPixels& pix)
{
    return FIT_BITMAP;
}

/*
	 *	Load file from disk
	 *		: not support png file right
	 */
FIBITMAP* loadImage(string fileName)
{
    ofxGIF::ofInitFreeImage();

    fileName = ofToDataPath(fileName);
    bool bLoaded = false;
    FIBITMAP* bmp = nullptr;

    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = FreeImage_GetFileType(fileName.c_str(), 0);
    if (fif == FIF_UNKNOWN) {
        // or guess via filename
        fif = FreeImage_GetFIFFromFilename(fileName.c_str());
    }
    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        bmp = FreeImage_Load(fif, fileName.c_str(), 0);

        if (bmp != NULL) {
            bLoaded = true;
        }
    }

    // this will create a 256-color palette from the image for gif
    FIBITMAP* convertedBmp;
    convertedBmp = FreeImage_ColorQuantize(bmp, FIQ_WUQUANT);

    FreeImage_Unload(bmp);

    return convertedBmp;
}

//----------------------------------------------------
template <typename PixelType>
FIBITMAP* getBmpFromPixels(const ofPixels_<PixelType>& pix)
{
    const PixelType* pixels = pix.getData();
    size_t width = pix.getWidth();
    size_t height = pix.getHeight();
    size_t bpp = pix.getBitsPerPixel();
    ofLogNotice() << " width: " << width << "  height: " << height << " Bits per pixel: " << bpp;

    FREE_IMAGE_TYPE freeImageType = getFreeImageType(pix);
    FIBITMAP* bmp = FreeImage_AllocateT(freeImageType, width, height, bpp);
    unsigned char* bmpBits = FreeImage_GetBits(bmp);
    if (bmpBits != nullptr) {
        size_t srcStride = width * pix.getBytesPerPixel();
        size_t dstStride = FreeImage_GetPitch(bmp);
        unsigned char* src = (unsigned char*)pixels;
        unsigned char* dst = bmpBits;
        if (srcStride != dstStride) {
            for (int i = 0; i < (int)height; i++) {
                memcpy(dst, src, srcStride);
                src += srcStride;
                dst += dstStride;
            }
        } else {
            memcpy(dst, src, dstStride * height);
        }
    } else {
        ofLogError("ofImage") << "getBmpFromPixels(): unable to get FIBITMAP from ofPixels";
    }

    // ofPixels are top left, FIBITMAP is bottom left
    FreeImage_FlipVertical(bmp);

    // this will create a 256-color palette from the image for gif
    FIBITMAP* convertedBmp;
    if (bpp == 32) {
        convertedBmp = FreeImage_ColorQuantize(bmp, FIQ_WUQUANT);
    } else if (bpp == 24) {
        convertedBmp = FreeImage_ColorQuantize(bmp, FIQ_NNQUANT);
    }

    FreeImage_Unload(bmp);

    return convertedBmp;
}

//----------------------------------------------------
void putBmpIntoPixels(FIBITMAP* bmp, ofPixels& pix, bool swapForLittleEndian = true)
{

    unsigned int width = FreeImage_GetWidth(bmp);
    unsigned int height = FreeImage_GetHeight(bmp);
    unsigned int pitch = FreeImage_GetPitch(bmp);
    unsigned int bpp = 24;

    if (FreeImage_GetColorType(bmp) == FIC_PALETTE || FreeImage_GetBPP(bmp) < 8) {
        if (FreeImage_IsTransparent(bmp)) {
            bpp = 32;
        }
    }

    unsigned int channels = (bpp / sizeof(unsigned char)) / 8;

    FreeImage_FlipVertical(bmp); // ofPixels are top left, FIBITMAP is bottom left

    RGBQUAD* pal = FreeImage_GetPalette(bmp);
    FIBITMAP* tmp = nullptr;
    tmp = FreeImage_Allocate(width, height, bpp);
    //BYTE* tmp_bits = FreeImage_GetBits(tmp);

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            BYTE t;
            FreeImage_GetPixelIndex(bmp, x, y, &t);

            RGBQUAD color = pal[t];
            BYTE r, g, b;
            r = color.rgbRed;
            b = color.rgbBlue;
            g = color.rgbGreen;            
            color.rgbRed = b;
            color.rgbGreen = g;
            color.rgbBlue = r;
            FreeImage_SetPixelColor(tmp, x, y, &color);
        }
    }

#ifdef TARGET_LITTLE_ENDIAN
    bool swapRG = channels && swapForLittleEndian && (bpp / channels == 8);
#else
    bool swapRG = false;
#endif

    ofLogNotice() << " GIF width: " << width << "  height: " << height << " Bits per pixel: " << bpp;

    unsigned char* bmpBits = FreeImage_GetBits(tmp);
    //unsigned char* bmpBits = FreeImage_GetBits(bmp);
    if (bmpBits != nullptr) {
        pix.setFromAlignedPixels((unsigned char*)bmpBits, width, height, channels, pitch);
    } else {
        ofLogError("ofImage") << "putBmpIntoPixels(): unable to set ofPixels from FIBITMAP";
    }

    if (tmp != nullptr) {
        FreeImage_Unload(tmp);
    }

    if (swapRG && channels >= 3) {
        pix.swapRgb();
    }
}
};

void ofxGif::create(string filename)
{
    //filename = ofToDataPath(filename);
    FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename.c_str());
    gif = FreeImage_OpenMultiBitmap(fif, filename.c_str(), TRUE, FALSE, TRUE);
}

void ofxGif::save()
{
    if (gif != nullptr) {
        ofLogNotice() << "Saving GIF image";
        FreeImage_CloseMultiBitmap(gif);
        gif = nullptr;
    } else {
        ofLogError() << "GIF image is empty, not saved...";
    }
}

void ofxGif::append(string filename)
{
    if (gif) {
        FIBITMAP* page = ofxGIF::loadImage(filename);
        FreeImage_AppendPage(gif, page);
        FreeImage_Unload(page);
    }
}

void ofxGif::append(ofPixels& pixels)
{
    if (gif) {
        FIBITMAP* page = ofxGIF::getBmpFromPixels(pixels);
        FreeImage_AppendPage(gif, page);
        FreeImage_Unload(page);
    }
}

bool ofxGif::load(string filename)
{
    ofxGIF::ofInitFreeImage();
    filename = ofToDataPath(filename);
    FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename.c_str());
    gif = FreeImage_OpenMultiBitmap(fif, filename.c_str(), FALSE, TRUE, TRUE);

    if (gif == nullptr) {
        ofLogNotice() << "Failed to open GIF file.";
        return false;
    }

    // Get src page count
    int count = FreeImage_GetPageCount(gif);

    ofLogNotice() << "Loading GIF, page count: " << count;

    // Clone src to dst
    for (int page = 0; page < count; page++) {
        // Load the bitmap at position 'page'
        FIBITMAP* dib = FreeImage_LockPage(gif, page);
        if (dib) {

            if (page == 0) { // print GIF file info
                unsigned int width = FreeImage_GetWidth(dib);
                unsigned int height = FreeImage_GetHeight(dib);
                unsigned int bpp = FreeImage_GetBPP(dib);
                string indexed = "no";
                string transparent = "no";
                if (FreeImage_GetColorType(dib) == FIC_PALETTE)
                    indexed = "yes";
                if (FreeImage_IsTransparent(dib))
                    transparent = "yes";
                ofLogNotice() << " width: " << width << "  height: " << height << " bpp: " << bpp;
                ofLogNotice() << " indexed: " << indexed << " transparent: " << transparent;
            }
            ofImage img;
            pages.push_back(img);
            ofxGIF::putBmpIntoPixels(dib, pages.back().getPixels());
            // Unload the bitmap (do not apply any change to src)
            FreeImage_UnlockPage(gif, dib, FALSE);
        } else {
            ofLogError() << "Error occurred loading GIF page...";
            FreeImage_CloseMultiBitmap(gif, 0);
            return false;
        }
    }

    // Close src
    FreeImage_CloseMultiBitmap(gif, 0);

    // weird, sometimes last page get empty, check all
    for (int i = 0; i < pages.size(); i++) {
        if ((int)pages[i].getWidth() == 0) {
            ofLogNotice() << "Erasing empty Gif page";
            pages.erase(pages.begin() + i--);
        }
    }

    return true;
}
