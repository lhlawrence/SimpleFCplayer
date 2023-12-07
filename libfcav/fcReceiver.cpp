#include "fcReceiver.h"
#include "ulibuser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <thread>

FCReceiver *FCReceiver::getInstance()
{
    static FCReceiver *instance = new FCReceiver();
    return instance;
}

static void sig_handler(void *ctx)
{
    FCReceiver::getInstance()->gotFrame();
    // printf("got frame\n");
}

void FCReceiver::gotFrame()
{
    cond.notify_all();
}

void FCReceiver::init(unsigned fcAddr, unsigned winID, unsigned row, unsigned col, unsigned colorbit, ColorType colorType)
{

    if (colorbit != 16)
    {
        printf("fc color bit error.\n");
        return;
    }
    if (colorType != FC_AV_RGB)
    {
        printf("fc color type error.\n");
        return;
    }
    printf("rw_init: %d\n", rw_Initial());

    printf("av_init: %d\n", av_tx_init());

    printf("FC rows=%d, cols=%d\n", row, col);

    this->fcAddr = fcAddr;
    this->winID = winID;
    this->row = row;
    this->col = col;
    this->colorbit = colorbit;
    this->colorType = colorType;

    long rxBuf_ptr = (long)rxBuf;
    rxBuf = map_txbuf(row * col * colorbit / 8);
    if (!rxBuf)
    {
        printf("rxBuf mmap error!!!\n");
        return;
    }
    else
    {
        printf("allocate avrx mem %p\n", rxBuf);
    }
    usleep(125000);
}

void FCReceiver::stop()
{
    munmap(rxBuf, row * col * colorbit / 8);
    rxBuf = NULL;
    cond.notify_all();
}

void FCReceiver::start()
{
    if (!rxBuf)
    {
        printf("rxBuf is NULL!!!\n");
        return;
    }

    av_rx_ctrl(winID, 1, fcAddr, 0, 0, col, row, colorbit, rxBuf);

    std::thread fc_thread(recvFrame);
    fc_thread.detach();

    int res_sig = add_avrx_handler(sig_handler, winID);
    usleep(125000);
}

void FCReceiver::recvFrame()
{
    FCReceiver::getInstance()->rgbBuf = new unsigned char[FCReceiver::getInstance()->row * FCReceiver::getInstance()->col * 2];
    av_rx_ctrl(
        FCReceiver::getInstance()->winID,
        1,
        FCReceiver::getInstance()->fcAddr,
        0,
        0,
        FCReceiver::getInstance()->col,
        FCReceiver::getInstance()->row,
        FCReceiver::getInstance()->colorbit + 0x80000000,
        FCReceiver::getInstance()->rxBuf);
    while (FCReceiver::getInstance()->rxBuf)
    {
        std::unique_lock<std::mutex> lock(FCReceiver::getInstance()->mtx);
        FCReceiver::getInstance()->cond.wait(lock);

        // convert rgb565 to rgba8888
        {
            std::lock_guard<std::mutex> lock(FCReceiver::getInstance()->mtxCopy);
            memcpy(FCReceiver::getInstance()->rgbBuf, FCReceiver::getInstance()->rxBuf, FCReceiver::getInstance()->row * FCReceiver::getInstance()->col * 2);
        }
    }
    printf("rgbBuf release\n");
    delete[] FCReceiver::getInstance()->rgbBuf;
}

// maybe dangerous
void FCReceiver::getFrame(unsigned char *buf)
{
    if (!rgbBuf)
    {
        printf("rgbBuf is NULL!!!\n");
        return;
    }
    std::lock_guard<std::mutex> lock(mtxCopy);
    memcpy(buf, rgbBuf, row * col * 2);
}