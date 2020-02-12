#ifndef __INTERSECTIONS_H
#define __INTERSECTIONS_H

#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <time.h>
#include <chrono>
using namespace std;
#include "raytrace.h"
#define ind1 1.0f // indice de l'air

bool hitPlan(const ray &r, const plan &pl, float &t);
 
bool hitSphere(const ray &r, const sphere &s, float &t) ;

void pix_impactPlan(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentPlan, point &impact);

void pix_impactSphere(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentSphere, point &impact);

bool find_intersection(scene &myScene, ray &viewRay, float &t, int &currentSphere, int &currentPlan, int &obj_type);

ray refract_ray_sphere_tmp(scene &myScene, ray &viewRay, float &t, int currentSphere, point &impact);

void pix_impactparaboloid(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentPara, point &impact);

bool hitParaboloid (const ray &r, const paraboloid &para, float &t);


#endif