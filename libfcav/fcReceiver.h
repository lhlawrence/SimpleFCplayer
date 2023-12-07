#pragma once
#include <stdlib.h>
#include <mutex>
#include <condition_variable>

class FCReceiver
{
public:
    typedef enum
    {
        FC_AV_RGB = 0,
        FC_AV_YUV,
    } ColorType;
    void init(unsigned fcAddr, unsigned winID, unsigned row, unsigned col, unsigned colorbit, ColorType colorType);
    void start();
    void stop();
    void gotFrame();
    void getFrame(unsigned char *buf);
    static FCReceiver *getInstance();

private:
    unsigned fcAddr;
    unsigned winID;
    unsigned row;
    unsigned col;
    unsigned colorbit;
    ColorType colorType;
    unsigned char *rxBuf;
    std::mutex mtx;
    std::condition_variable cond;
    std::mutex mtxCopy;
    unsigned char *rgbBuf;

    static void recvFrame();
};