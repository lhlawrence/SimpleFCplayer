#include <libfcav/fcReceiver.h>
#include <unistd.h>
#include <glview.hpp>
#include <libfcav/fcUtils.h>
#include <libfcav/ctlCam.h>

unsigned char *rgbBuffer;
unsigned port_id = 0x334400;
unsigned win_id = 25;
unsigned tgt_id = 10;
unsigned w = 1920;
unsigned h = 1080;

void loop(int)
{
    GLViewer::getInstance()->paint_gl();
    FCReceiver::getInstance()->getFrame(rgbBuffer);
    GLViewer::getInstance()->update(w, h, rgbBuffer);
    glutTimerFunc(1000 / 25, loop, 0);
};

int main(int argc, char **argv)
{
    setenv("MWV206_GLVERSION", "1", 1);
    setenv("vblank_mode", "1", 1);

    rgbBuffer = new unsigned char[w * h * 2];

    // check fc driver
    if (get_mod_status() != 0)
    {
        // insmod
        if (insmod_fc_driver() != 0)
        {
            printf("insmod fc driver failed\n");
            return -1;
        }
    }

    // check port_login
    char reg_path[256];
    if (get_fc_pci_reg_path(reg_path) == 0)
    {
        int linkStatus = port_login(reg_path, 0, port_id);
        if (linkStatus != 0)
        {
            printf("port_login failed once, try again\n");
            linkStatus = port_login(reg_path, 0, port_id);
            if (linkStatus == -1)
            {
                sleep(5);
                if (linkStatus == -1)
                {
                    printf("port_login failed twice, exit\n");
                    return -1;
                }
            }
        }
    }
    else
    {
        printf("get fc pci reg path failed\n");
        return -1;
    }

    glutInit(&argc, argv);

    glutInitContextVersion(1, 5); //初始化OpenGL所需文件，设置版本为2.1
    // glutInitDisplayMode(GLUT_DOUBLE); //设置双缓冲模式，即带有front buffer 和 back buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("FCAV");

    GLViewer::getInstance()->init_gl();

    FCReceiver *fcReceiver = FCReceiver::getInstance();
    fcReceiver->init(port_id, win_id, w, h, 16, FCReceiver::FC_AV_RGB);

    fcReceiver->start();

    // start camera
    int camera_ret = start_cam(port_id, tgt_id, win_id, w, h, 0x80780438);
    if (camera_ret != 0)
    {
        printf("start camera failed\n");
        return -1;
    }
    printf("start camera success\n");

    glutDisplayFunc(GLViewer::getInstance()->paint_gl);
    glutTimerFunc(1000 / 25, loop, 0);
    glutMainLoop();

    stop_cam(port_id);
    delete[] rgbBuffer;
    fcReceiver->stop();
    return 0;
}