/*========================================================================
* COSC 363  Computer Graphics (2018)
* Assignment 2
* David Mayo (35120341)
*=========================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/glut.h>
#include "Plane.h"
using namespace std;

const float WIDTH = 20.0;
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX = WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX = HEIGHT * 0.5;
const float SHININESS = 5;

bool aaFlag = true;

vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene


									//---The most important function in a ray tracer! ---------------------------------- 
									//   Computes the colour value obtained by tracing a ray and finding its 
									//     closest point of intersection with objects in the scene.
									//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);
	glm::vec3 light(-40, 20, -10);
	glm::vec3 light2(0, 29, -130);
	glm::vec3 ambientCol(0.2);   //Ambient color of light

	ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray

	if (ray.xindex == -1) return backgroundCol;      //If there is no intersection return background colour

	glm::vec3 color1;
	glm::vec3 color2;
	glm::vec3 colorSum;
	glm::vec3 materialCol = sceneObjects[ray.xindex]->getColor(); //else return object's colour
	glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt);

	glm::vec3 lightVector = glm::normalize(light - ray.xpt);
	glm::vec3 lightVector2 = glm::normalize(light2 - ray.xpt);
	glm::vec3 reflVector = glm::reflect(-lightVector, normalVector);
	glm::vec3 reflVector2 = glm::reflect(-lightVector2, normalVector);

	float lightDist = glm::length(light - ray.xpt);
	float lightDist2 = glm::length(light2 - ray.xpt);
	float lDotn = glm::dot(lightVector, normalVector);
	float lDotn2 = glm::dot(lightVector2, normalVector);
	float specularTerm = pow(glm::dot(reflVector, lightVector), SHININESS);
	float specularTerm2 = pow(glm::dot(reflVector2, lightVector2), SHININESS);
	float rDotl = glm::dot(reflVector, lightVector);
	float rDotl2 = glm::dot(reflVector2, lightVector2);

	Ray shadow(ray.xpt, lightVector);
	Ray shadow2(ray.xpt, lightVector2);
	shadow.closestPt(sceneObjects);
	shadow2.closestPt(sceneObjects);

	if (ray.xindex == 3)
	{
		int xval = ray.xpt.x;
		int zval = ray.xpt.z;
		bool xCheck = false;
		bool zCheck = false;
		if (xval % 16 > 7 || (xval % 16 < 0 && xval % 16 > -9)) xCheck = true;
		if (zval % 16 > 7 || (zval % 16 < 0 && zval % 16 > -9)) zCheck = true;

		if ((xCheck && !zCheck) || (zCheck && !xCheck))
		{
			materialCol = glm::vec3(0, 1, 0);
		}
		else
		{
			materialCol = glm::vec3(1, 0, 0);
		}
	}

	if (ray.xindex == 1)
	{
		int xval = ray.xpt.x;

		if (xval % 4 < -1)
		{
			materialCol = glm::vec3(0, 0.6, 0);
		}
		else
		{
			materialCol = glm::vec3(0.4, 0.8, 0.4);
		}
	}

	if (lDotn <= 0 || (shadow.xindex > -1 && shadow.xdist < lightDist))
	{
		color1 = ambientCol * materialCol;
	}
	else
	{
		if (rDotl < 0)
		{
			specularTerm = 0;
		}
		color1 = (ambientCol * materialCol) + (lDotn * materialCol) + (specularTerm * (1, 1, 1));
	}

	if (lDotn2 <= 0 || (shadow2.xindex > -1 && shadow2.xdist < lightDist2))
	{
		color2 = ambientCol * materialCol;
	}
	else
	{
		if (rDotl2 < 0)
		{
			specularTerm2 = 0;
		}
		color2 = (ambientCol * materialCol) + (lDotn2 * materialCol) + (specularTerm2 * (1, 1, 1));
	}

	colorSum = (color1 * 0.6f) + (color2 * 0.4f);

	if (ray.xindex == 2)
	{
		glm::vec3 transDir = ray.dir;
		Ray transRay(ray.xpt, transDir);
		glm::vec3 transCol = trace(transRay, step + 1);
		colorSum = (colorSum * 0.9f) + (transCol * 0.8f);
	}

	if ((ray.xindex == 0 || ray.xindex == 3 || ray.xindex == 2) && step < MAX_STEPS)
	{
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVector);
		Ray reflectedRay(ray.xpt, reflectedDir);
		glm::vec3 reflectedCol = trace(reflectedRay, step + 1);
		if (ray.xindex == 3)
		{
			colorSum = colorSum + (0.4f * reflectedCol);
		}
		else if (ray.xindex == 2)
		{
			colorSum = colorSum + (0.2f * reflectedCol);
		}
		else
		{
			colorSum = colorSum + (0.8f * reflectedCol);
		}
	}

	return colorSum;

}

void aaHelper(glm::vec3 eye, float xp, float yp, float cellX, float cellY)
{
	if (aaFlag) {
		float rVal, gVal, bVal;
		glm::vec3 dir1(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);
		glm::vec3 dir2(xp + 0.75 * cellX, yp + 0.25 * cellY, -EDIST);
		glm::vec3 dir3(xp + 0.25 * cellX, yp + 0.75 * cellY, -EDIST);
		glm::vec3 dir4(xp + 0.75 * cellX, yp + 0.75 * cellY, -EDIST);
		Ray ray1 = Ray(eye, dir1);
		Ray ray2 = Ray(eye, dir2);
		Ray ray3 = Ray(eye, dir3);
		Ray ray4 = Ray(eye, dir4);
		ray1.normalize();
		ray2.normalize();
		ray3.normalize();
		ray4.normalize();
		glm::vec3 col1 = trace(ray1, 1);
		glm::vec3 col2 = trace(ray2, 1);
		glm::vec3 col3 = trace(ray3, 1);
		glm::vec3 col4 = trace(ray4, 1);
		rVal = (col1.r + col2.r + col3.r + col4.r) / 4;
		gVal = (col1.g + col2.g + col3.g + col4.g) / 4;
		bVal = (col1.b + col2.b + col3.b + col4.b) / 4;
		glColor3f(rVal, gVal, bVal);
	}
	else
	{
		glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);
		Ray ray = Ray(eye, dir);
		ray.normalize();
		glm::vec3 col = trace(ray, 1);
		glColor3f(col.r, col.g, col.b);
	}
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height

	glm::vec3 eye(0., 0., 0.);  //The eye position (source of primary rays) is the origin

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a quad.

	for (int i = 0; i < NUMDIV; i++)  	//For each grid point xp, yp
	{
		xp = XMIN + i*cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

			aaHelper(eye, xp, yp, cellX, cellY);

			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
}

void drawSphere(float size, float x_offset, float y_offset, float z_offset, glm::vec3 color)
{
	Sphere *sphere = new Sphere(glm::vec3(x_offset, y_offset, z_offset), size, color);
	sceneObjects.push_back(sphere);
}

void drawFloor(float width, float length, float x_offset, float y_offset, float z_offset, glm::vec3 color)
{
	float x1 = x_offset - (width * 0.5);
	float x2 = x_offset + (width * 0.5);
	float y = y_offset;
	float z1 = z_offset - (length * 0.5);
	float z2 = z_offset + (length * 0.5);

	Plane *plane = new Plane(glm::vec3(x1, y, z2),		//Point A
		glm::vec3(x2, y, z2),		//Point B
		glm::vec3(x2, y, z1),		//Point C
		glm::vec3(x1, y, z1),		//Point D
		color						//Colour
	);
	sceneObjects.push_back(plane);
}

void drawBack(float width, float height, float x_offset, float y_offset, float z_offset, glm::vec3 color)
{
	float x1 = x_offset - (width * 0.5);
	float x2 = x_offset + (width * 0.5);
	float y1 = y_offset - (height * 0.5);
	float y2 = y_offset + (height * 0.5);
	float z = z_offset;

	Plane *plane = new Plane(glm::vec3(x1, y2, z),		//Point A
		glm::vec3(x2, y2, z),		//Point B
		glm::vec3(x2, y1, z),		//Point C
		glm::vec3(x1, y1, z),		//Point D
		color						//Colour
	);
	sceneObjects.push_back(plane);
}

void drawCube(float size, float x_offset, float y_offset, float z_offset, glm::vec3 color)
{
	float x1 = x_offset - (size * 0.5);
	float x2 = x_offset + (size * 0.5);
	float y1 = y_offset - (size * 0.5);
	float y2 = y_offset + (size * 0.5);
	float z1 = z_offset - (size * 0.5);
	float z2 = z_offset + (size * 0.5);

	Plane *cubeFront = new Plane(glm::vec3(x1, y2, z1),
		glm::vec3(x2, y2, z1),
		glm::vec3(x2, y1, z1),
		glm::vec3(x1, y1, z1),
		color
	);
	Plane *cubeBack = new Plane(glm::vec3(x1, y2, z2),
		glm::vec3(x2, y2, z2),
		glm::vec3(x2, y1, z2),
		glm::vec3(x1, y1, z2),
		color
	);
	Plane *cubeTop = new Plane(glm::vec3(x1, y2, z1),
		glm::vec3(x1, y2, z2),
		glm::vec3(x2, y2, z2),
		glm::vec3(x2, y2, z1),
		color
	);
	Plane *cubeBottom = new Plane(glm::vec3(x1, y1, z1),
		glm::vec3(x1, y1, z2),
		glm::vec3(x2, y1, z2),
		glm::vec3(x2, y1, z1),
		color
	);
	Plane *cubeLeft = new Plane(glm::vec3(x1, y2, z1),
		glm::vec3(x1, y1, z1),
		glm::vec3(x1, y1, z2),
		glm::vec3(x1, y2, z2),
		color
	);
	Plane *cubeRight = new Plane(glm::vec3(x2, y2, z2),
		glm::vec3(x2, y1, z2),
		glm::vec3(x2, y1, z1),
		glm::vec3(x2, y2, z1),
		color
	);

	sceneObjects.push_back(cubeFront);
	sceneObjects.push_back(cubeBack);
	sceneObjects.push_back(cubeTop);
	sceneObjects.push_back(cubeBottom);
	sceneObjects.push_back(cubeLeft);
	sceneObjects.push_back(cubeRight);

}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
	glClearColor(0, 0, 0, 1);

	drawSphere(30, 0, 0, -200, glm::vec3(0, 0, 1));					// Blue sphere
	drawSphere(8, -20, 20, -150, glm::vec3(1, 0, 0));				// Red sphere
	drawSphere(10, 17, -10, -130, glm::vec3(0.8, 0, 0.4));			// Pink sphere
	drawFloor(600, 600, 0, -30, 0, glm::vec3(0.5, 0.5, 0.5));		// Floor
	drawBack(600, 100, 0, 0, -220, glm::vec3(0, 0.5, 0.5));			// Back wall
	drawFloor(600, 600, 0, 30, 0, glm::vec3(2, 2, 2));				// Ceiling
	drawCube(15, -22, -22.5, -140, glm::vec3(0.8, 0.4, 0.1));		// Bottom left cube
}



int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(20, 20);
	glutCreateWindow("Raytracer");

	glutDisplayFunc(display);
	initialize();

	glutMainLoop();
	return 0;
}
