#ifndef __INTERSECTIONS_H
#define __INTERSECTIONS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <time.h>
#include <chrono>
using namespace std;

#include "intersections.h"
#include "raytrace.h"
#define ind1 1.0f // indice de l'air

bool hitPlan(const ray &r, const plan &pl, float &t)
{
    /* Fonction qui indique si on a rencontre un plan et actualise la distance t */
    float t_calc(0.0f);
    float temp = 1/sqrtf(pl.normale*pl.normale);
    vecteur n = temp * pl.normale;
    float A = pl.d - (r.start*n);
    float B = n*r.dir;

    t_calc = A/B;
    if (t_calc < t)
    {
        t = t_calc;
        return true;
    }
return false;
}
 
bool hitSphere(const ray &r, const sphere &s, float &t) 
{ 
    /* Fonction qui indique si on a rencontre une sphere et actualise la distance t */
    vecteur dist = s.pos - r.start; 
    float B = r.dir * dist;
    float D = B*B - dist * dist + s.size * s.size; // D est le determinant 
    if (D < 0.0f) 
        return false; // Déterminant nul -> pas d'intersections
    float t0 = B - sqrtf(D); //1ere racine
    float t1 = B + sqrtf(D); // 2eme racine
    bool retvalue = false;  
    if ((t0 > 0.1f) && (t0 < t)) // Si t0<t c'est que la sphere est devant, il faut actualiser le pixel
    {
        t = t0;
        retvalue = true; 
    } 
    if ((t1 > 0.1f) && (t1 < t)) // Si t1<t c'est que la sphere est devant, il faut actualiser le pixel
    {
        t = t1; 
        retvalue = true; 
    }
    return retvalue; 
}

bool hitParaboloid (const ray &r, const paraboloid &para, float &t)
{
    /* Fonction qui indique si on a rencontre une paraboloide et actualise la distance t */

    /* Dans le cas de la paraboloide on obtient une equation de degre 2 verifiee par t */
	float a = ((r.dir.x)/para.a)*((r.dir.x)/para.a) + ((r.dir.y)/para.b)*((r.dir.y)/para.b);
	float b = 2*r.start.x*r.dir.x/para.a + 2*(r.start.y)*r.dir.y/para.b - r.dir.z;
	float c = (r.start.x/para.a)*(r.start.x/para.a) + (r.start.y/para.b)*(r.start.y/para.b) - r.start.z;

	float D = b*b - 4*a*c; // D est le determinant
	if (D < 0.0f)
	{
		return false;
	}
    bool retvalue = false;
	float t0 = (-b-sqrtf(D))/2*a;
	float t1 = (-b+sqrtf(D))/2*a;
	if ((t0 > 0.1f) && (t0 < t)) // Si t0<t c'est que l'objet est devant, il faut actualiser le pixel
    {
        t = t0;
        retvalue = true; 
    } 
    if ((t1 > 0.1f) && (t1 < t)) // Si t1<t c'est que l'objet est devant, il faut actualiser le pixel
    {
        t = t1; 
        retvalue = true; 
    }
    return retvalue; 
}


void pix_impactPlan(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentPlan, point &impact)
{
    /* Fonction qui actualise la couleur du pixel d'impact quand c'est un plan */
    vecteur n = myScene.planTab[currentPlan].normale; // normale au point d'intersection
    float temp = 1/ sqrtf(n * n); 
    n = temp * n ; // On normalise n
               
    material currentMat = myScene.matTab[myScene.planTab[currentPlan].material]; 

    // ============================= couleur du point d'impact =============================  
    for (unsigned int j = 0; j < myScene.lgtTab.size(); ++j) 
    {
        light current = myScene.lgtTab[j];
        vecteur dist = current.pos - impact;

        if (n * dist <= 0.0f)
            continue; // Si la lumiere est tangente, elle n'eclaire pas

        float temp = sqrtf(dist * dist);
        if ( temp <= 0.0f )
            continue;
                     
        ray lightRay;
        lightRay.start = impact;
        lightRay.dir = (1/temp) * dist; // On normalise

        // calcul des ombres 
        bool inShadow = false; 
        for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
        {
            if (hitSphere(lightRay, myScene.sphTab[i], t) && currentMat.opacity == 1) 
            {
                inShadow = true;
                break;
            }
        }
        if (!inShadow) 
        {
            // lambert
            float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
            red += lambert * current.red * currentMat.red;
            green += lambert * current.green * currentMat.green;
            blue += lambert * current.blue * currentMat.blue;
        }
    }
                 
    // on itére sur la prochaine reflexion
    coef *= currentMat.reflection;
    float reflet = 2.0f * (viewRay.dir * n);
    viewRay.start = impact;
    viewRay.dir = viewRay.dir - reflet * n;
}

void pix_impactSphere(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentSphere, point &impact)
{
    /* Fonction qui actualise la couleur du pixel d'impact quand c'est une sphere */
    vecteur n = impact - myScene.sphTab[currentSphere].pos; // normale au point d'intersection
    n = n / sqrtf(n*n);
               
    material currentMat = myScene.matTab[myScene.sphTab[currentSphere].material]; 

    // ============================= couleur du point d'impact =============================  
    for (unsigned int j = 0; j < myScene.lgtTab.size(); ++j) 
    {
        light current = myScene.lgtTab[j];
        vecteur dist = current.pos - impact;

        if (n * dist <= 0.0f)
            continue; // Si n*dist < 0 : pas de specularite : on saute l'etape

        float t = sqrtf(dist * dist);
        if ( t <= 0.0f )
            continue;
        ray lightRay;
        lightRay.start = impact;
        lightRay.dir = (1/t) * dist; // On normalise

        // calcul des ombres 
        bool inShadow = false; 
        for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
        {
            if (hitSphere(lightRay, myScene.sphTab[i], t) && myScene.matTab[myScene.sphTab[i].material].opacity == 1) 
            {
                inShadow = true;
                break;
            }
        }
        //inShadow = false;
        if (!inShadow ) 
        {
            // lambert
            float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
            red += lambert * current.red * currentMat.red;
            green += lambert * current.green * currentMat.green;
            blue += lambert * current.blue * currentMat.blue;
        }
    }
                 
    // on itére sur la prochaine reflexion
    coef *= currentMat.reflection;
    float reflet = 2.0f * (viewRay.dir * n);
    viewRay.start = impact;
    viewRay.dir = viewRay.dir - reflet * n; // viewray devient le rayon réfléchi
}

void pix_impactParaboloid(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentPara, point &impact)
{
    /* Fonction qui actualise la couleur du pixel d'impact quand c'est une paraboloide */
    paraboloid Para = myScene.paraTab[currentPara];
    vecteur n ;
    n.x = 2*impact.x/Para.a;
    n.y = 2*(impact.y)/Para.b;
    n.z = -1.0f;
    n = n / sqrtf(n*n);
               
    material currentMat = myScene.matTab[Para.material]; 

    // ============================= couleur du point d'impact =============================  
    for (unsigned int j = 0; j < myScene.lgtTab.size(); ++j) 
    {
        light current = myScene.lgtTab[j];
        vecteur dist = current.pos - impact;

        if (n * dist <= 0.0f)
            continue; // Si n*dist < 0 : pas de specularite : on saute l'etape

        float t = sqrtf(dist * dist);
        if ( t <= 0.0f )
            continue;
        ray lightRay;
        lightRay.start = impact;
        lightRay.dir = (1/t) * dist; // On normalise

        // calcul des ombres 
        bool inShadow = false; 
        for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
        {
            if (hitSphere(lightRay, myScene.sphTab[i], t) && myScene.matTab[myScene.sphTab[i].material].opacity == 1) 
            {
                inShadow = true;
                break;
            }
        }
        inShadow = false;
        if (!inShadow ) 
        {
            // lambert
            float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
            red += lambert * current.red * currentMat.red;
            green += lambert * current.green * currentMat.green;
            blue += lambert * current.blue * currentMat.blue;
        }
    }
                 
    // on itére sur la prochaine reflexion
    coef *= currentMat.reflection;
    float reflet = 2.0f * (viewRay.dir * n);
    viewRay.start = impact;
    viewRay.dir = viewRay.dir - reflet * n; // viewray devient le rayon réfléchi
}

bool find_intersection(scene &myScene, ray &viewRay, float &t, int &currentSphere, int &currentPlan, int &currentPara, int &obj_type)
{
    for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
    { 
        if (hitSphere(viewRay, myScene.sphTab[i], t)) 
        {
            currentSphere = i;
            obj_type = 1;
        }
    }
    for (unsigned int i =0; i < myScene.planTab.size(); i++)
    {
        if (hitPlan(viewRay, myScene.planTab[i], t))
        {
            currentPlan = i;
            obj_type = 2;
        }  
    }
    for (unsigned int i =0; i < myScene.paraTab.size(); i++)
    {
        if (hitParaboloid(viewRay, myScene.paraTab[i], t))
        {
            currentPara = i;
            obj_type = 3;
        }  
    }

    if (obj_type == -1)
        return false;
     
    return true;
}

ray refract_ray_sphere_tmp(scene &myScene, ray &viewRay, float &t, int currentSphere, point &impact)
{
    // C'est deux refractions d'affile :
    material currentMat = myScene.matTab[myScene.sphTab[currentSphere].material]; 
    float ind2 = currentMat.refraction; // indice du materiau

    // Premire refraction : on cree un rayon refracte t_ray:
    vecteur n = impact - myScene.sphTab[currentSphere].pos; // normale au point d'impact
    n = n / sqrtf(n*n);

    vecteur v = viewRay.dir;
    v = v / sqrtf(v*v);
    float c = v * n;
    c = -c;
    float r = ind1/ind2 ;
    float A =(r*c + sqrtf(1 - r*r*(1-c*c)));

    // On genere le 1er rayon transmis
    ray t_ray;
    t_ray.start = impact;
    vecteur refract_dir = (r*v) + (A*n);
    refract_dir = refract_dir / sqrtf(refract_dir*refract_dir);
    t_ray.dir = refract_dir;
    return t_ray;
}


#endif