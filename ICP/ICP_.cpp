// icp.cpp 
// Author: JJ

// C++ 
#include <iostream>
#include <chrono>
#include <stack>
#include <random>
#include <numeric>

// OpenCV 
#include <opencv2\opencv.hpp>

// OpenGL Extension Wrangler
#include <GL/glew.h> 
#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform) 

// GLFW toolkit
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenGL textures
#include <gli/gli.hpp>

//project includes...
#include "globals.h"
#include "init.h"
#include "callbacks.h"
#include "glerror.h" // Check for GL errors

#include "lua_engine.h"
#include "lua_interface.h"

#include "gl_draw.h"
#include "mesh.h"
#include "mesh_init.h"

// forward declarations
static void init_cv(void);

static void init(void);
void process_video(cv::VideoCapture& capture, std::atomic<glm::vec2>& center_relative);
glm::vec2 process_frame(cv::Mat& frame);
// end of forward declarations


//---------------------------------------------------------------------
// MAIN
//---------------------------------------------------------------------
cv::Mat frame;
std::atomic<bool> new_frame(false);

int main(int argc, char* argv[])
{
	std::thread camera_thread;
	std::atomic<glm::vec2> center_relative;
	glm::vec2 local_center_relative(0.0f,0.0f);
	cv::Mat local_frame;
	int gl_frame_count(0);
	double current_time, last_fps_time = glfwGetTime();

	init(); //all initializations, including camera
	
	mesh c = gen_mesh_circle(100,100);

	camera_thread = std::thread(process_video, std::ref(globals.capture), std::ref(center_relative));

	// Main application loop:
	// Run until exit is requested.
	while (!glfwWindowShouldClose(globals.window))
	{
		// Clear color buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Use ModelView matrix for following trasformations (translate,rotate,scale)
		glMatrixMode(GL_MODELVIEW);
		// Clear all tranformations
		glLoadIdentity();

		// Draw something
		{
			if (new_frame) {
				frame.copyTo(local_frame);
				local_center_relative = center_relative;
				new_frame = false;

				// flip image vertically: screen coordinates and GL world coordinates have opposite Y-axis orientation
				cv::flip(local_frame, local_frame, 0);
				local_center_relative.y = 1.0f + -1.0f * local_center_relative.y;
			}
			{
				// show image using GL, simple method, direct pixel copy
				glRasterPos2i(0, 0);
				glDrawPixels(local_frame.cols, local_frame.rows, GL_BGR, GL_UNSIGNED_BYTE, local_frame.data);
			}
		}

		//move to window center
		//glTranslatef(globals.width / 2.0f, globals.height / 2.0f, 0.0f);
		mesh_draw_arrays(c);

		glTranslatef(globals.width * local_center_relative.x, globals.height * local_center_relative.y, 0.0f);

		glRotatef(glfwGetTime() * 100, 0.0f, 1.0f, 0.0f);
		// Render here 
		draw_simple_triangle();
		glColor3f(1, 1, 1);
		draw_dynamic_circle(0.0f, 0.0f, 150.0f+50*sin(glfwGetTime()), 8+ceil(4*sin(glfwGetTime())));

		// Swap front and back buffers 
		// Calls glFlush() inside
		glfwSwapBuffers(globals.window);

		// Poll for and process events
		glfwPollEvents();

		// print GL FPS
		gl_frame_count++;
		current_time = glfwGetTime();
		if (current_time - last_fps_time > 1.0)
		{
			std::cout << '[' << current_time << ", GL] FPS = " << gl_frame_count / (current_time - last_fps_time) << std::endl;
			last_fps_time = current_time;
			gl_frame_count = 0;
		}

	}

	if (camera_thread.joinable())
		camera_thread.join();

	finalize(EXIT_SUCCESS);
}

void process_video(cv::VideoCapture& capture, std::atomic<glm::vec2>& center_relative)
{
	cv::Mat local_frame;
	glm::vec2 temp_center;
	int camera_frame_count(0);
	double last_fps_time = glfwGetTime(), current_time;

	while (!glfwWindowShouldClose(globals.window))
	{
		if (capture.read(local_frame))
		{
			temp_center = process_frame(local_frame);

			//atomic assignments
			if (!new_frame) {
				center_relative = temp_center;
				local_frame.copyTo(frame);
				new_frame = true;
			}
			camera_frame_count++;

			// print camera analyzer FPS
			current_time = glfwGetTime();
			if (current_time - last_fps_time > 1.0)
			{
				std::cout << '[' << current_time << ",cam] FPS = " << camera_frame_count / (current_time - last_fps_time) << std::endl;
				last_fps_time = current_time;
				camera_frame_count = 0;
			}
		}
		else
			glfwSetWindowShouldClose(globals.window, true);
	}
}

glm::vec2 process_frame(cv::Mat& frame)
{
	glm::vec2 result(0.0f, 0.0f);

	// load clasifier
	cv::CascadeClassifier face_cascade = cv::CascadeClassifier("resources/haarcascade_frontalface_default.xml");

	// find face
	cv::Mat scene_gray;
	cv::cvtColor(frame, scene_gray, cv::COLOR_BGR2GRAY);
	std::vector<cv::Rect> faces;
	face_cascade.detectMultiScale(scene_gray, faces);
	if (faces.size() > 0)
	{
		result.x = (faces[0].x + (faces[0].width / 2.0f)) / frame.cols;
		result.y = (faces[0].y + (faces[0].height / 2.0f)) / frame.rows;
	}

	// DO NOT DISPLAY! Must not create any OpenCV window!
	// NO! cv::imshow("grabbed", frame);
	// DO NOT POLL EVENTS! Must not call cv::waitKey(), it would steal events from GLFW main loop!
	// NO! cv::waitKey(1);

	return result;
}

static void init(void)
{
	init_cv();
	init_glfw();
	init_glew();
	gl_print_info();
}

static void init_cv(void)
{
	globals.capture = cv::VideoCapture(cv::CAP_DSHOW);

	if (!globals.capture.isOpened())
	{
		std::cerr << "no camera" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Initial camera" <<
			": width=" << globals.capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
			", height=" << globals.capture.get(cv::CAP_PROP_FRAME_HEIGHT) <<
			", FPS=" << globals.capture.get(cv::CAP_PROP_FPS) << std::endl;
	}

	if (!globals.capture.set(cv::CAP_PROP_FRAME_WIDTH, 640))
		std::cout << "Failed setting width." << std::endl;
	if (!globals.capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480))
		std::cout << "Failed setting height." << std::endl;
	if (!globals.capture.set(cv::CAP_PROP_FPS, 30))
		std::cout << "Failed setting FPS." << std::endl;

	std::cout << "Camera changed" <<
		": width=" << globals.capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
		", height=" << globals.capture.get(cv::CAP_PROP_FRAME_HEIGHT) <<
		", FPS=" << globals.capture.get(cv::CAP_PROP_FPS) << std::endl;
}

//
//PLEASE IGNORE NOW: callbacks
//
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {};
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {};

void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void fbsize_callback(GLFWwindow* window, int width, int height)
{
	globals.width = width;
	globals.height = height;

	glMatrixMode(GL_PROJECTION);				// set projection matrix for following transformations
	glLoadIdentity();							// clear all transformations

	glOrtho(0, width, 0, height, -20000, 20000);  // set Orthographic projection

	glViewport(0, 0, width, height);			// set visible area
}
