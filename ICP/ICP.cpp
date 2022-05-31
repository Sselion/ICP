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

#include "mesh2.h"
#include "mesh2_init.h"

#include "texture.h"

#include "shaders.h"

//Camera
#include "camera.h"

// forward declarations
//static void init_cv(void);
static void init(void);
void reset_projection(void);
//void process_video(cv::VideoCapture& capture, std::atomic<glm::vec2>& center_relative);
glm::vec2 process_frame(cv::Mat& frame);
bool CollisionMapDetection(std::vector<mesh2>& map, mesh2& car);
// end of forward declarations


//mesh
mesh2 mesh2_circle;
mesh2 mesh2_cube;
mesh2 mesh2_floor;
mesh2 mesh2_leauto;
std::vector<mesh2> game_map;

//shaders toon_shader;
shaders basic_shader;
shaders basic_tex_shader;
bool shader_ready = false;

GLuint VAO;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//camera
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;
double lastX_m = lastX;
double lastY_m = lastY;

//Start Position
float start_x, start_z = 0.0f;

//movement
glm::vec3 move = glm::vec3(0.0f, 0.0f, 0.0f);
float car_speed = 0.2f;
float rotate = 0.0f;
const float rotate_bias = 90.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Game
glm::vec2 start_pos = glm::vec2(0,0);
glm::vec2 end_pos = glm::vec2(0,0);

// Color change
bool isChange = false;
bool isColorChange = false;
float redChannel = 0.0f;
float greenChannel = 0.0f;

void draw_scene(glm::vec2 & local_center_relative)
{
	// Time measurement, FPS count etc.
	static double time_fps_old = 0.0;
	static double time_frame_old = 0.0;
	static int frame_cnt = 0;
	double time_current, time_frame_delta;

	time_current = glfwGetTime();
	time_frame_delta = time_current - time_frame_old;
	time_frame_old = time_current;

	//FPS
	if (time_current - time_fps_old > 1.0)
	{
		time_fps_old = time_current;
		std::cout << "FPS: " << frame_cnt << std::endl;
		frame_cnt = 0;
	}
	frame_cnt++;

	//
	// DRAW
	//

	// Set the camera
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10.0f);
	glm::mat4 view = camera.GetViewMatrix();

	//glm::mat4 mv_m = glm::lookAt(glm::vec3(5000.0f, 500.0f, 5000.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 diffColor;
	if (isColorChange) {
		diffColor = glm::vec4(redChannel, greenChannel, 1.0f, 1.0f);
	}
	else {
		diffColor = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f);
	}
	

	// plane with texture
	{
		// activate shader with textures support
		basic_tex_shader.activate();
		glUniformMatrix4fv(glGetUniformLocation(basic_tex_shader.ID, "uMV_m"), 1, GL_FALSE, glm::value_ptr(view));
		// set diffuse material
		glUniform4fv(glGetUniformLocation(basic_tex_shader.ID, "u_diffuse_color"), 1, glm::value_ptr(diffColor));
		//set texture unit
		glActiveTexture(GL_TEXTURE0);
		//send unit number to FS
		glUniform1i(glGetUniformLocation(basic_tex_shader.ID, "tex0"), 0);
		glUniform1i(glGetUniformLocation(basic_tex_shader.ID, "isChange"), isChange);
		glUniform4fv(glGetUniformLocation(basic_tex_shader.ID, "ColorChange"), 1, glm::value_ptr(glm::vec4(1.0, 1.0, 1.0, 1.0)));
		// draw object
		mesh2_draw(mesh2_floor);
	}
	// MAP
	{
		// activate shader without textures
		//basic_tex_shader.activate();
		//scale
		//view = glm::scale(view, glm::vec3(1000.0f));
		//view = glm::rotate(view, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		glUniformMatrix4fv(glGetUniformLocation(basic_tex_shader.ID, "uMV_m"), 1, GL_FALSE, glm::value_ptr(view));
		//set material 
		glUniform4fv(glGetUniformLocation(basic_tex_shader.ID, "u_diffuse_color"), 1, glm::value_ptr(diffColor));
		glUniform1i(glGetUniformLocation(basic_tex_shader.ID, "isChange"), isChange);
		glUniform4fv(glGetUniformLocation(basic_tex_shader.ID, "ColorChange"), 1, glm::value_ptr(glm::vec4(1.0, 1.0, 1.0, 1.0)));
		//set texture unit
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(glGetUniformLocation(basic_tex_shader.ID, "tex0"), 0);
		//draw
		for (int i = 0; i < game_map.size(); i++)
		{
			mesh2_draw(game_map[i]);
		}
	}
	// CAR
	{
		view = glm::translate(view, move);
		view = glm::rotate(view, glm::radians(rotate+ rotate_bias), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(basic_tex_shader.ID, "uMV_m"), 1, GL_FALSE, glm::value_ptr(view));
		//set material 
		glUniform4fv(glGetUniformLocation(basic_tex_shader.ID, "u_diffuse_color"), 1, glm::value_ptr(diffColor));
		glUniform1i(glGetUniformLocation(basic_tex_shader.ID, "isChange"), isChange);
		glUniform4fv(glGetUniformLocation(basic_tex_shader.ID, "ColorChange"), 1, glm::value_ptr(glm::vec4(1.0, 1.0, 1.0, 1.0)));
		//set texture unit
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(glGetUniformLocation(basic_tex_shader.ID, "tex0"), 0);

		//draw
		mesh2_draw(mesh2_leauto);
	}
}

static void init_mesh(void)
{
	mesh2_floor = gen_mesh_floor();

	GenerateMap(game_map, start_pos, end_pos, "resources/map.bmp", "resources/textures_transp.png");

	move.x = start_pos.x;
	move.z = start_pos.y;
	mesh2_leauto.center += glm::vec3(start_pos.x, 0.0, start_pos.y);
	camera.Position = glm::vec3(0.0f + start_pos.x, 0.7f, 1.5f + start_pos.y);

	mesh2_leauto = gen_mesh_obj("resources/car.obj", "resources/carGrid_flip.png", glm::vec3(0.0, 0.5, 0.0), glm::vec3(0.0, 0.5, 0.0), true);
	mesh2_leauto.scale(glm::vec3{ 0.1f, 0.1f, 0.1f });
	mesh2_leauto.init();
}

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
	double last_fps_time = glfwGetTime();

	init(); //all initializations, including camera
	init_mesh();
	//toon_shader = shaders("resources/shaders/toon.vert", "resources/shaders/toon.frag");
	basic_shader = shaders("resources/shaders/basic.vert", "resources/shaders/basic.frag");
	basic_tex_shader = shaders("resources/shaders/basic_tex.vert", "resources/shaders/basic_tex.frag");
	basic_tex_shader.activate();
	shader_ready = true;
	reset_projection();

	//camera_thread = std::thread(process_video, std::ref(globals.capture), std::ref(center_relative));

	// Main application loop:
	// Run until exit is requested.
	while (!glfwWindowShouldClose(globals.window))
	{
		// Clear color buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render here 
		{
			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

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
				//glRasterPos2i(0, 0);
				//glDrawPixels(local_frame.cols, local_frame.rows, GL_BGR, GL_UNSIGNED_BYTE, local_frame.data);
			}
		}

		draw_scene(local_center_relative);
		// ...

		// Swap front and back buffers 
		// Calls glFlush() inside
		glfwSwapBuffers(globals.window);

		// Check OpenGL errors
		gl_check_error();

		// Poll for and process events
		glfwPollEvents();
	}

	//if (camera_thread.joinable())
	//	camera_thread.join();

	finalize(EXIT_SUCCESS);
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

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "HIGH: ";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "MEDIUM: ";
		break;
	default: 
		return;
		break;
	}

	std::cout << "[GL CALLBACK]: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") <<
		"type = 0x" << std::hex << type <<
		", severity = 0x" << std::hex << severity <<
		", message = '" << message << '\'' << std::endl << std::dec;
}

static void init(void)
{
	//init_cv();
	init_glfw();
	init_glew();

	//since GL 4.3
	glDebugMessageCallback(MessageCallback, 0);
	glEnable(GL_DEBUG_OUTPUT);

	gl_print_info();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_SMOOTH); //antialiasing
	glEnable(GL_LINE_SMOOTH);

	// ALL objects are non-transparent 
	//glEnable(GL_CULL_FACE);

	// scene contains semi-transparent objects
	glDisable( GL_CULL_FACE );                    // no polygon removal
}

void reset_projection(void)
{
	float ratio = globals.width * 1.0f / globals.height;

	glm::mat4 projectionMatrix = glm::perspective(
		glm::radians(60.0f), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
		ratio,			     // Aspect Ratio. Depends on the size of your window.
		0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		20000.0f              // Far clipping plane. Keep as little as possible.
	);

	if (shader_ready)
	{	
		GLint currProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
		
		// set projection for all shaders
		glUniformMatrix4fv(glGetUniformLocation(currProgram, "uProj_m"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	}
}

// CALLBACKS
//
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	greenChannel = xpos / SCR_WIDTH;
	redChannel = ypos / SCR_HEIGHT;

	//Jinak pøetýká do ostatních barev (asi)
	if (greenChannel > 1.0) greenChannel = 1.0f;
	if (redChannel > 1.0) redChannel = 1.0f;

	//camera.ProcessMouseMovement(lastX_m - xpos, lastY_m - ypos);
	//lastX_m = xpos;
	//lastY_m = ypos;
};
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {};

void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static GLfloat point_size = 1.0f;
	float local_move_x;
	float local_move_z;
	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT))
	{
		switch (key) {
		case GLFW_KEY_0:
			isChange = false;
			isColorChange = false;
			break;
		case GLFW_KEY_1:
			isChange = true;
			break;
		case GLFW_KEY_2:
			isColorChange = true;
			break;
		case GLFW_KEY_W:
			local_move_x = cos(glm::radians(rotate)) * car_speed;
			local_move_z = -sin(glm::radians(rotate)) * car_speed;
			std::cout << "Collision: " << CollisionMapDetection(game_map, mesh2_leauto);
			camera.ProcessKeyboard(RIGHT, local_move_x);
			camera.ProcessKeyboard(FORWARD, -local_move_z);
			move.x += local_move_x;
			move.z += local_move_z;
			mesh2_leauto.center += glm::vec3(local_move_x, 0.0, local_move_z);
			break;
		case GLFW_KEY_S:
			local_move_x = -cos(glm::radians(rotate)) * car_speed;
			local_move_z = sin(glm::radians(rotate)) * car_speed;
			camera.ProcessKeyboard(LEFT, -local_move_x);
			camera.ProcessKeyboard(BACKWARD, local_move_z);
			move.x += local_move_x;
			move.z += local_move_z;

			mesh2_leauto.center += glm::vec3(local_move_x, 0.0, local_move_z);
			break;
		case GLFW_KEY_A:
			rotate += 2.0f;
			//camera.ProcessKeyboard(LEFT, 1);
			break;
		case GLFW_KEY_D:
			rotate -= 2.0f;
			break;
		case GLFW_KEY_Q:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		default:
			break;
		}
	}
}

void fbsize_callback(GLFWwindow* window, int width, int height)
{
	// check for limit case (prevent division by 0)
	if (height == 0)
		height = 1;

	globals.width = width;
	globals.height = height;

	glViewport(0, 0, width, height);			// set visible area

	reset_projection();
}

bool CollisionMapDetection(std::vector<mesh2>& map, mesh2& car) {
	for (int i = 0; i < map.size(); i++)
	{
		if (map[i].collisionEnable && collision_detect(map[i], car))
			return true;
	}
	return false;
}
