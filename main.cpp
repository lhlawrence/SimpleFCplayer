#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include <chrono>
#include <unordered_map>
// 定义清除宏
#define CLEAR(x) memset(&(x),0,sizeof(x))

// 窗口尺寸
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
unsigned w = 1920;
unsigned h = 1080;
unsigned char *rgbBuffer;


int fd; // 文件描述符，用于V4L2操作

struct buffer {
    void   *start;
    size_t  length;
};
buffer *buffers;

// 添加的代码
bool init_mmap() {
    struct v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            std::cerr << "不支持内存映射" << std::endl;
            return false;
        } else {
            perror("VIDIOC_REQBUFS");
            return false;
        }
    }
    std::cout << "open camera2 !" << std::endl;
    buffers = new buffer[req.count];

    //memset(&buffers,0, sizeof(buffers));
 
    //CLEAR(buffers);
    for (size_t n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;
        //CLEAR(buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            perror("VIDIOC_QUERYBUF");
            return false;
        }

        buffers[n_buffers].length = buf.length;
            std::cout << buffers[n_buffers].length << std::endl;
        buffers[n_buffers].start = mmap(NULL, buf.length,
                                        PROT_READ | PROT_WRITE, MAP_SHARED,
                                        fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            perror("mmap");
            return false;
        }
    }
    std::cout << "open camera3 !" << std::endl;
    for (size_t i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;
        //CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
            perror("VIDIOC_QBUF");
            return false;
        }
    }
    std::cout << "open camera4 !" << std::endl;

    return true;
}


bool init_camera(const char *dev_name) {
    // 摄像头初始化代码
    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        perror("无法打开设备");
        return false;
    }

    // 此处省略了检查和设置摄像头格式等代码,请根据实际情况自行添加
    // 设置摄像头格式
    struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = w;
    fmt.fmt.pix.height      = h;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565; // 或您需要的其他格式
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == ioctl(fd, VIDIOC_S_FMT, &fmt)) {
        perror("VIDIOC_S_FMT");
        return false;
    }
    std::cout << "open camera !" << std::endl;
    // 初始化内存映射
    if (!init_mmap()) {
        return false;
    }

    // 启动捕获
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
        perror("VIDIOC_STREAMON");

    return true;
}

unsigned int compileShader(const char* shaderSource, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    // 检查编译错误（省略）
    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(shader, length, &length, message);
        std::cout << "Failed to compile " 
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
                  << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(shader);
        delete[] message;
        return 0;
    }

    return shader;
}

unsigned int initShaderProgram() {
    //const char *vertexShaderSource = R"glsl(
      //  #version 310 es
        //layout (location = 0) in vec3 aPos;
       // layout (location = 1) in vec2 aTexCoord;
//        out vec2 TexCoord;
  //      void main() {
    //        gl_Position = vec4(aPos, 1.0);
      //      TexCoord = aTexCoord;
        //}
    //)glsl";
    //const char *fragmentShaderSource = R"glsl(
    //    #version 310 es
    //    precision mediump float;
    //    in vec2 TexCoord;
    //    out vec4 FragColor;
    //    uniform sampler2D ourTexture;
    //    void main() {
    //        FragColor = texture(ourTexture, TexCoord);
    //    }
    //)glsl";

    const char *vertexShaderSource =
        "#version 310 es\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "}\0";

    const char *fragmentShaderSource =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {\n"
        "    FragColor = texture(ourTexture, TexCoord);\n"
        "}\n\0";


    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // 检查链接错误（省略）
    // ...

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

bool capture_image() {
    //auto start1 = std::chrono::high_resolution_clock::now();
    // 图像捕获代码
    // 队列缓冲区
    struct v4l2_buffer buf;
    //CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
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
    //std::cout << "open !" << std::endl;
    //assert(buf.index < 2);

    // 此处可以对获取的图像数据进行处理，例如格式转换等
      // std::cout << buf.index << std::endl;
    
    memcpy(rgbBuffer, buffers[buf.index].start, buf.bytesused);

    //std::cout << "open2 !" << std::endl;
    // 更新纹理
    // glBindTexture(GL_TEXTURE_2D, texture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
    //std::cout << "open2 !" << std::endl;
    if (ioctl(fd, VIDIOC_QBUF, &buf)<0)
    {perror("VIDIOC_QBUF fail\n");
    return -1;}
    //auto stop1  =std::chrono::high_resolution_clock::now();
    //auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop1-start1);
    //std::cout << "memcpy time:"<<duration1.count()<<"ms"<<std::endl;
    return true;
}

void close_camera() {
    // 关闭摄像头代码
    // 关闭流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
        perror("VIDIOC_STREAMOFF");

    for (size_t i = 0; i < 4; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }
    delete[] buffers;
    // 关闭设备
    close(fd);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int argc, char* argv[]) {
    std::unordered_map<std::string, std::string> args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] == '-') {  // If it's a parameter name
            if (i + 1 < argc) {  // Make sure we aren't at the end of argv!
                args[arg] = argv[++i];  // Increment 'i' and get the parameter value
            } else {
                std::cerr << "Missing value for parameter: " << arg << std::endl;
                return 1;
            }
        }
    }
    std::string device;    
    // 使用参数
    if (args.find("-dev") != args.end()) {
        device = args["-dev"];
        std::cout << "Device: " << device << std::endl;
        // Initialize your application with the device
    } else {
        std::cerr << "Device not specified" << std::endl;
        return 1;
    }
    
        
    rgbBuffer=NULL;
    rgbBuffer=(unsigned char *)calloc(1,(size_t)(w*h*2));
    if(rgbBuffer==NULL)
    std::cout<<"error"<<std::endl;
    else
    std::cout<<"calloc sucess!"<<std::endl;
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // 指定OpenGL ES的主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // 指定OpenGL ES的副版本号
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AV", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 加载 OpenGL ES 函数指针
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 初始化着色器
    unsigned int shaderProgram = initShaderProgram();
    unsigned int VBO, VAO, EBO;

    float vertices[] = {
        // 顶点坐标    // 纹理坐标
        //1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // 右上角
        //1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 右下角
        //-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 左下角
        //-1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // 左上角
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 右上角
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // 右下角
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // 左下角
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f  // 左上角
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 顶点位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 创建纹理
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 初始化摄像头
    if (!init_camera(device.c_str())) {
        return 1;
    }

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
    //auto start = std::chrono::high_resolution_clock::now();
        if (capture_image()) {
        
            glClearColor(0.f,0.f,0.f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shaderProgram);

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, rgbBuffer);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
                //auto stop  =std::chrono::high_resolution_clock::now();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    //std::cout << "render time:"<<duration.count()<<"ms"<<std::endl;
    }

    close_camera();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
