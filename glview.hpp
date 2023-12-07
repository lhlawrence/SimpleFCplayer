#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <string.h>
#include <memory>
#include <vector>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>

class GLViewer
{
public:
    static GLViewer *getInstance()
    {
        static GLViewer *instance = new GLViewer();
        return instance;
    }

    GLViewer()
    {
        width = 1920;
        height = 1080;
        data = new unsigned char[width * height * 2];
    }

    void init_gl()
    {
        // glMatrixMode(GL_PROJECTION); //设置为投影矩阵
        // glLoadIdentity();            //加载单位矩阵，和前一句一起，意思是设置投影矩阵为单位矩阵。

        // glMatrixMode(GL_MODELVIEW); //设置为模型矩阵
        // glLoadIdentity();           //加载单位矩阵，和前一句一起，意思是设置模型矩阵为单位矩阵。

        glClearColor(0.f, 0.3f, 0.f, 1.f); //设置清屏颜色，也就是调用glClear后屏幕的颜色
        // GLenum error = glGetError();
        // if (error != GL_NO_ERROR) //检查初始化是否成功
        // {
        //     printf("Error initializing OpenGL! %s\n", gluErrorString(error)); //因为glGetError()返回的是错误码，所以gluErrorString()获取错误信息
        //     return;
        // }

        glShadeModel(GL_FLAT);
        glDisable(GL_DEPTH_TEST);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        texture_id.resize(1, 0);
        glGenTextures(1, texture_id.data());
        glBindTexture(GL_TEXTURE_2D, texture_id[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    void resize_gl(int w, int h)
    {
        glViewport(0, 0, (GLsizei)w, (GLsizei)h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
    }

    void update(int w, int h, unsigned char *data)
    {
        width = w;
        height = h;
        memcpy(this->data, data, width * height * 2);
        // this->data = data;
    }

    static void paint_gl()
    {
        GLViewer *viewer = GLViewer::getInstance();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (1)
        {
            glBindTexture(GL_TEXTURE_2D, viewer->texture_id[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         viewer->width, viewer->height,
                         0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (GLvoid *)viewer->data);
            glEnable(GL_TEXTURE_2D);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            // glEnd();
            glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(-1.0, -1.0, 0.0);
            glTexCoord2f(0.0, 0.0);
            glVertex3f(-1.0, 1.0, 0.0);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(1.0, 1.0, 0.0);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(1.0, -1.0, 0.0);
            glEnd();
            glutSwapBuffers();
        }
    }

private:
    unsigned char *data;
    int width;
    int height;
    std::vector<GLuint> texture_id;
};
