#pragma once

// OpenGL Extension Wrangler
#include <GL/glew.h> 
#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform) 

// GLFW toolkit
#include <GLFW/glfw3.h>

// OpenCV 
#include <opencv2\opencv.hpp>


struct s_globals {
	GLFWwindow* window;
	int height;
	int width;
	double app_start_time;

	cv::VideoCapture capture;
};

extern s_globals globals;
