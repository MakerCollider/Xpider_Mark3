#include <iostream>
#include <sstream>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <iomanip>

#include <opencv2/opencv.hpp>

#include <xpiderctl/xpiderctl.h>
#include <xpiderctl/config.h>

bool running;
pthread_t grabThread;
pthread_mutex_t mutexLock;

cv::VideoCapture camera;
cv::CascadeClassifier face_cascade;
std::string face_cascade_name;

void clear(int signo)
{
    std::cout << "Get exit signal" << std::endl;
    running = false;
}

void* grabFunc(void* in_data)
{
    while(running)
    {
        pthread_mutex_lock(&mutexLock);
        if(camera.isOpened())
        {
            camera.grab();
        }
        pthread_mutex_unlock(&mutexLock);
    }

    std::cout << "Grab thread exit." << std::endl;
}

void init()
{
    //摄像头
    camera.open(0);

    if(!camera.isOpened())
    {
        std::cout << "Can not find camera!" << std::endl;
        exit(-1);
    }

    camera.set(cv::CAP_PROP_FRAME_WIDTH,160);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT,120);

/*    double width = camera.get(cv::CAP_PROP_FRAME_WIDTH);
    double height = camera.get(cv::CAP_PROP_FRAME_HEIGHT);

    std::cout << "width:" << width << "\t";
    std::cout << "height:" << height << "\t" << std::endl;*/

    face_cascade_name = "/usr/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";

    if(!face_cascade.load(face_cascade_name))
    {
        std::cout << "can not find face_cascade_file!" << std::endl;
        exit(-1);
    }

    running = true;
    pthread_mutex_init(&mutexLock, NULL);
    pthread_create(&grabThread, NULL, grabFunc, NULL);

    //Setup edi robot mraa control lib
    spider_init();

    signal(SIGINT, clear);
    signal(SIGTERM, clear);
}

int main()
{
    double timeUse;
    struct timeval startTime, stopTime;

    cv::Mat rawImage, grayImage;
    std::vector<cv::Rect> faces;

    init();
    spider_head(45);

    std::stringstream logStream, outStream;
    float faceX, faceY;
    int8_t rotateDegree;
    uint8_t rotatePwm;
    uint8_t lostCounter = 0;
    while(running)
    {
        logStream.str("");
        logStream << std::fixed << std::setprecision(3);
        outStream.str("");
        outStream << std::fixed << std::setprecision(3);

        gettimeofday(&startTime, NULL);

        camera.retrieve(rawImage);

        cv::cvtColor(rawImage, grayImage, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayImage, grayImage);

        faces.clear();
        face_cascade.detectMultiScale(grayImage, faces, 1.1,
            2, 0|cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
        if(faces.empty())
        {
            if(lostCounter != 0)
            {
                lostCounter --;
                spider_rotate_degree(rotateDegree, rotatePwm, NULL, NULL);

                logStream << "Face lost, lost counter: " << static_cast<int>(lostCounter) << ", ";
            }
            else
            {
                spider_move_stop();
                
                logStream << "No face!";
            }
        }
        else
        {
            lostCounter = 5;
            faceX = faces[0].x+faces[0].width*0.5;
            faceY = faces[0].y+faces[0].height*0.5;

            logStream << "Get face, size: " << faces.size() << ", ";
            logStream << "coordinate: x " << faceX << " y " << faceY;

            if(faceX < 80)
            {
                rotateDegree = -2;
                rotatePwm = 80;
            }
            else if(faceX > 80)
            {
                rotateDegree = 2;
                rotatePwm = 80;
            }

            if(faceX < 70 || faceX > 90)
            {
                spider_rotate_degree(rotateDegree, rotatePwm, NULL, NULL);
            }

            //spider_move(1, 55);
        }

        gettimeofday(&stopTime, NULL);
        timeUse = stopTime.tv_sec - startTime.tv_sec + (stopTime.tv_usec - startTime.tv_usec)/1000000.0;
        if(timeUse < 0.25)
            usleep((0.25 - timeUse) * 1000000);

        outStream << "Time use: " << timeUse << "s, " << logStream.str();
        std::cout << outStream.str() << std::endl;
    }

    void* result;
    pthread_join(grabThread, &result);

    spider_head(35);
    spider_move_stop();
    spider_rotate_stop();
    spider_close();

    camera.release();

    std::cout << "Program exit!" << std::endl;

    return 0;
}