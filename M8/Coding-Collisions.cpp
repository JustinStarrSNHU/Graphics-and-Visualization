#include <glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };


class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;
	int brick_strength;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, int strength)
	{
		brick_type = bt; 
		x = xx; 
		y = yy; 
		width = ww; 
		brick_strength = strength;;
		red = 1;
		green = 1;
		blue = 1;
		onoff = ON;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			// as the brick's strength is reduced the color of each brick is changed
			double halfside = width / 2;
			if (brick_strength == 4000) {
				red = 0;
				green = 1; // green color to indicate strong
				blue = 0;
			}
			else if (brick_strength == 3000) {
				red = 1; // a yellow color to indicate medium strength
				green = 1;
				blue = 0;
			}
			else if (brick_strength == 2000) {
				red = 1; // a red color to indicate weak strength
				green = 0;
				blue = 0;
			}

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}

	void ReduceStrength() {
		brick_strength -= 200;
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
		
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection(); 
				brk->ReduceStrength();
				x = x + 0.03;
				y = y + 0.04;
				
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				if (brk->brick_strength > 0) {
					brk->ReduceStrength();
					direction = GetRandomDirection();
					
				}
				else {
					brk->onoff = OFF; // turns the brick off when the brick's strength is equal to zero
				}
			}
		}
	}

	// returns a random number 1-8
	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	//The speed of the circles is decreased when they touch the sides of the screen
	void MoveOneStep(Circle &cir)
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
				this->speed -= .002;
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
				this->speed -= .002;
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
				this->speed -= .002;
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
				this->speed -= .002;
			}
		}
	}

	// draws the circle
	void DrawCircle()
	{	
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();		
	}

	// Checks if circles collide. If a collision occurs the size of the circles that collide are reduced
	// Also, when a collision occurs the color of the circles are changed to a random color
	void CheckCircleCollision(Circle& cir)
	{
		auto circleDist = sqrt(std::pow(cir.x - this->x, 2) + (std::pow(cir.y - this->y, 2)));

		if (circleDist < this->radius + cir.radius) {
			
			y *= -1;
			x *= -1;

			this->x += x * (this->radius - (circleDist / 2));
			this->y += y * (this->radius - (circleDist / 2));

			cir.x *= -1;
			cir.y *= -1;

			cir.x += cir.x * (cir.radius - (circleDist / 2));
			cir.y += cir.y * (cir.radius - (circleDist / 2));

			this->radius = this->radius / 2;
			cir.radius = cir.radius / 2;

			double r, g, b;
			r = rand() / 10000;
			g = rand() / 10000;
			b = rand() / 10000;

			cir.red = r;
			cir.green = g;
			cir.blue = b;
		}
	}
};

// creates the vector of circles called world
vector<Circle> world;

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(1280, 800, "Justin Starr CS-330 Module 8 Coding Collisions", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// each brick starts off green and change color as they are collided with. When the bricks lose all of their health, they are destroyed
	Brick brick1(DESTRUCTABLE, 0.0, 0.0, 0.2, 4000); 
	Brick brick2(DESTRUCTABLE, 0.0, 0.2, 0.2, 4000); 
	Brick brick3(DESTRUCTABLE, -0.2, 0.0, 0.2, 4000); 
	Brick brick4(DESTRUCTABLE, 0.2, 0, 0.2, 4000); 
	Brick brick5(DESTRUCTABLE, 0.0, -0.2, 0.2, 4000); 
	Brick brick6(DESTRUCTABLE, -0.2, -0.2, 0.2, 4000); 
	Brick brick7(DESTRUCTABLE, 0.2, -0.2, 0.2, 4000); 
	Brick brick8(DESTRUCTABLE, -0.4, -0.2, 0.2, 4000); 
	Brick brick9(DESTRUCTABLE, 0.4, -0.2, 0.2, 4000);

	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		// calls the function to process any input from the user
		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			// checks collisions with bricks
			world[i].CheckCollision(&brick1);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].CheckCollision(&brick5);
			world[i].CheckCollision(&brick6);
			world[i].CheckCollision(&brick7);
			world[i].CheckCollision(&brick8);
			world[i].CheckCollision(&brick9);

			// checks circle collisions
			Circle& currentCircle = world[i];

			if (i + 1 < world.size()) {
				for (int l = i + 1; l < world.size(); l++) {
					Circle& otherCircle = world[l];
					currentCircle.CheckCircleCollision(otherCircle);
				}
			}

			// moves the circle
			world[i].MoveOneStep(currentCircle);
			// draws the circle
			world[i].DrawCircle();

		}

		// draws the bricks
		brick1.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		brick5.drawBrick();
		brick6.drawBrick();
		brick7.drawBrick();
		brick8.drawBrick();
		brick9.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	// for the escape key
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// for the space-bar key
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		int randDirection = (rand() % 8) + 1;
		int randX = (rand() % 2);
		int randY = (rand() % 2);
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		// creates a new circle ans positions it randomly and set with a random direction and random color
		Circle B(randX, randY, 02, randDirection, 0.05, r, g, b);
		world.push_back(B);
	}
}