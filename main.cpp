#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <glview.hpp>


unsigned char *rgbBuffer;
unsigned w = 1920;
unsigned h = 1080;
int fd; // 文件描述符，用于V4L2操作

struct buffer {
    void   *start;
    size_t  length;
};
buffer *buffers;

bool init_camera(const char *dev_name) {
    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        perror("无法打开设备");
        return false;
    }

    // 此处省略了检查和设置摄像头格式等代码,请根据实际情况自行添加

    // 启动捕获
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
        perror("VIDIOC_STREAMON");

    return true;
}

bool capture_image() {
    // 队列缓冲区
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        perror("VIDIOC_QBUF");

    // 取出缓冲区
    if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return false;
            case EIO:
                // 忽略EIO错误
            default:
                perror("VIDIOC_DQBUF");
        }
    }

    assert(buf.index < 2);

    // 此处可以对获取的图像数据进行处理，例如格式转换等
    memcpy(rgbBuffer, buffers[buf.index].start, buf.bytesused);

    return true;
}

void close_camera() {
    // 关闭流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
        perror("VIDIOC_STREAMOFF");

    // 关闭设备
    close(fd);
}



// OpenGL的循环函数，替换原有的loop函数
void loop(int) {
    if (capture_image()) {
        GLViewer::getInstance()->update(w, h, rgbBuffer);
        GLViewer::getInstance()->paint_gl();
    }
    glutTimerFunc(1000 / 25, loop, 0);
}

int main(int argc, char **argv)
{
    setenv("MWV207_GLVERSION", "1", 1);
    setenv("vblank_mode", "1", 1);

    rgbBuffer = new unsigned char[w * h * 3];
    if (!init_camera("/dev/video0")) {
        return 1;
    }

    glutInit(&argc, argv);

    glutInitContextVersion(1, 5); //初始化OpenGL所需文件，设置版本为2.1
    // glutInitDisplayMode(GLUT_DOUBLE); //设置双缓冲模式，即带有front buffer 和 back buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("AV");

    GLViewer::getInstance()->init_gl();


    glutDisplayFunc(GLViewer::getInstance()->paint_gl);
    glutTimerFunc(1000 / 25, loop, 0);
    glutMainLoop();

    close_camera();
    delete[] rgbBuffer;
    return 0;
}