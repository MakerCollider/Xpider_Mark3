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

void clear(int signo)
{
    std::cout << "Get exit signal" << std::endl;
    running = false;
}

static int map(int dstmin, int dstmax, 
                 int srcmin,int srcmax,
                 int srcval)
{
    float tmp=(float)(srcval-srcmin)/(srcmax-srcmin);
    float dst=tmp*(dstmax-dstmin)+dstmin;
    return (int)dst;
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

    std::cout << "width:" << width << ", ";
    std::cout << "height:" << height << std::endl;*/

    running = true;
    pthread_mutex_init(&mutexLock, NULL);
    pthread_create(&grabThread, NULL, grabFunc, NULL);

    //Setup edi robot mraa control lib
    spider_init();
    spider_head(40);
    signal(SIGINT, clear);
    signal(SIGTERM, clear);
}

int main()
{
    double timeUse;
    struct timeval startTime, stopTime;

    cv::Mat rawImage;
    cv::Mat grayImage, thresImage, cannyImage;

    cv::Mat circleImage(160, 120, CV_8UC1);
    std::vector<cv::Vec3f> circles;
    std::vector<std::vector<cv::Point> > contours;

    init();

    double area;
    bool hasCircles;
    float targetX, targetY;
    int8_t rotatePwm;
    int8_t rotateDegree;
    uint8_t lostCounter = 0;
    std::stringstream logStream, outStream;
    while (running)
    {
        logStream.str("");
        logStream << std::fixed << std::setprecision(3);
        outStream.str("");
        outStream << std::fixed << std::setprecision(3);

        gettimeofday(&startTime, NULL);

        camera.retrieve(rawImage);

        cv::cvtColor(rawImage, grayImage, cv::COLOR_RGB2GRAY);
        cv::adaptiveThreshold(grayImage, thresImage, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,201,20);
        cv::Canny(thresImage, cannyImage, 150, 200);

        hasCircles = false;
        cv::findContours(cannyImage, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        for (int i = 0; i < contours.size(); i++)
        {
            area = fabs(cv::contourArea(contours[i]));
            if (area > 200)
            {
                circleImage.setTo(0);
                cv::drawContours(circleImage, contours, i, cv::Scalar(255, 255, 255), cv::FILLED);
                cv::HoughCircles(circleImage, circles, cv::HOUGH_GRADIENT, 2, circleImage.rows / 2, 30, 15);
                if (circles.size() > 0)
                {
                    hasCircles = true;
                    targetX = circles[0][0];
                    targetY = circles[0][1];
                    logStream << "Get circle, size: " << area << ", ";
                    logStream << "coordinate: x " << targetX << " y " << targetY;
                    break;
                }
            }
        }

        if(hasCircles)
        {
            if(targetX < 80)
            {
                rotateDegree = -1;//-1 * map(1, 5, 0, 80, targetX);//(targetX / 80.0) * -5;
                rotatePwm = 50;//map(25, 30, 0, 80, targetX);//(targetX / 80.0) * 40;
            }
            else if(targetX > 80)
            {
                rotateDegree = 1;//map(1, 5, 80, 160, targetX);//((targetX - 80) / 80.0) * 5;
                rotatePwm = 50;//map(25, 30, 80, 160, targetX);//((targetX - 80) / 80.0) * 40;
            }

            if(targetX < 65 || targetX > 95)
            {
                spider_rotate_degree(rotateDegree, rotatePwm, NULL, NULL);
                //spider_rotate(rotateDegree, rotatePwm);
                lostCounter = 20;
            }
            else
            {
                spider_rotate_stop();
            }

            //spider_move(1, 60);
        }
        else
        {
            if(lostCounter != 0)
            {
                lostCounter --;
                spider_rotate_degree(rotateDegree * 3 , rotatePwm, NULL, NULL);
                //spider_rotate(rotateDegree, 30);

                logStream << "Circle lost, lost counter: " << static_cast<int>(lostCounter) << ", ";
            }
            else
            {
                spider_move_stop();
                spider_rotate_stop();
                
                logStream << "No Circle!";
            }
        }

        gettimeofday(&stopTime, NULL);
        timeUse = stopTime.tv_sec - startTime.tv_sec + (stopTime.tv_usec - startTime.tv_usec)/1000000.0;
        //if(timeUse < 0.06)
          //  usleep((0.06 - timeUse) * 1000000);

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