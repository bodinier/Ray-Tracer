#ifndef __INTERSECTIONS_H
#define __INTERSECTIONS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <math.h>
#include <limits>
#include <algorithm>
#include <time.h>
#include <chrono>
using namespace std;

#include "intersections.h"
#include "raytrace.h"
#define ind1 1.0f // indice de l'air

#define specvalue 0.4
#define specpower 70
#define mode 0 // Active ou non la spécuarité
#define enable_refraction 1 // Active ou non la réfraction des rayons
#define enableNoise 0
#define L 100

float max_val(float a, float b)
{
    return (a >= b) ? a : b;
}

bool chooseColorTexture(const point impact, const plan pl)
{
    //float x = (pl.d - pl.normale.y*impact.y - pl.normale.z*impact.z)/pl.normale.x;
    if (((int)floor(impact.x/L)%2 == 0 && (int)floor(impact.z/L)%2 == 0) || ((int)floor(impact.x/L)%2 == 1 && (int)floor(impact.z/L)%2 == 1))
        return true;
    else 
        return false;
}

bool hitPlan(const ray &r, const plan &pl, float &t)
{
    /* Fonction qui indique si on a rencontre un plan et actualise la distance t */
    float t_calc(0.0f);
    vecteur n = pl.normale;
    n.normalize();
    float A = pl.d - (r.start*n);
    float B = n*r.dir;

    t_calc = A/B;
    if ((t >0.1f) && t_calc < t && t_calc > 0 /*&& t_calc < 15000*/)
    {
        t = t_calc;
        return true;
    }
	return false;
}

/*bool hitTri(const ray &r, const triangle &tri, float &t)
{
    Fonction qui indique si on a rencontre un plan et actualise la distance t
    float t_calc(0.0f);
    vecteur n = pl.normale;
    n.normalize();
    float A = pl.d - (r.start*n);
    float B = n*r.dir;

    t_calc = A/B;
    if ((t >0.1f) && t_calc < t && t_calc > 0//&& t_calc < 15000)
    {
        t = t_calc;
        return true;
    }
	return false;
}*/
 
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
    bool isImpacted = false;  
    if ((t0 > 0.1f) && (t0 < t)) // Si t0<t c'est que la sphere est devant, il faut actualiser le pixel
    {
        t = t0;
        isImpacted = true; 
    } 
    if ((t1 > 0.1f) && (t1 < t)) // Si t1<t c'est que la sphere est devant, il faut actualiser le pixel
    {
        t = t1; 
        isImpacted = true; 
    }
    return isImpacted; 
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
    bool isImpacted = false;
	float t0 = (-b-sqrtf(D))/2*a;
	float t1 = (-b+sqrtf(D))/2*a;
	if ((t0 > 0.1f) && (t0 < t)) // Si t0<t c'est que l'objet est devant, il faut actualiser le pixel
    {
        t = t0;
        isImpacted = true; 
    } 
    if ((t1 > 0.1f) && (t1 < t)) // Si t1<t c'est que l'objet est devant, il faut actualiser le pixel
    {
        t = t1; 
        isImpacted = true; 
    }
    return isImpacted; 
}


void pix_impactPlan(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentPlan, point &impact)
{
    /* Fonction qui actualise la couleur du pixel d'impact quand c'est un plan */
    vecteur n = myScene.planTab[currentPlan].normale; // normale au point d'intersection
    n.normalize();
               
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
                     
        ray lightRay = {{impact.x, impact.y, impact.z}, {dist.x, dist.y, dist.z}};
        lightRay.dir.normalize(); // On normalise

        // calcul des ombres 
        bool shadow = false; 
        for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
        {
            if (hitSphere(lightRay, myScene.sphTab[i], t) && currentMat.opacity == 1) 
            {
                shadow = true;
                break;
            }
        }
        if (!shadow) 
        {   if ((myScene.planTab[currentPlan].textureOn == 1) && chooseColorTexture(impact, myScene.planTab[currentPlan]))
            {
                // lambert
            float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
            red += lambert * current.red * currentMat.red;
            green += lambert * current.green * currentMat.green;
            blue += lambert * current.blue * currentMat.blue;
            }
            else {
                // lambert
                float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
                red += 0;
                green += 0;
                blue += 0;
            }
            if (myScene.planTab[currentPlan].textureOn == 0)
            {
                {
                        // lambert
                    float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
                    red += lambert * current.red * currentMat.red;
                    green += lambert * current.green * currentMat.green;
                    blue += lambert * current.blue * currentMat.blue;
                }
            }
        }

        if (mode == 1)
        {
            float reflet = 2.0f * (lightRay.dir * n);
            vecteur phongDir = lightRay.dir - reflet * n;
            float phongTerm = max_val(phongDir * viewRay.dir, 0.0f) ;
            phongTerm = specvalue * powf(phongTerm, specpower) * coef;
            red += phongTerm * current.red;
            green += phongTerm * current.green;
            blue += phongTerm * current.blue;
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
    if (enableNoise)
    {
    	impact.x +=0.1*n.x;
    	impact.y *= n.y;
    	impact.z -= 0.1*n.z;
    }
    n.normalize();
               
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

        ray lightRay = {{impact.x, impact.y, impact.z}, {dist.x, dist.y, dist.z}};
        lightRay.dir.normalize(); // On normalise

        // calcul des ombres 
        bool shadow = false; 
        for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
        {
            if (hitSphere(lightRay, myScene.sphTab[i], t)) 
            {
                shadow = true;
                break;
            }
        }

        if (!shadow) 
        {
            // lambert
            float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
            red += lambert * current.red * currentMat.red;
            green += lambert * current.green * currentMat.green;
            blue += lambert * current.blue * currentMat.blue;
        }
        if (mode == 1)
        {
            float reflet = 2.0f * (lightRay.dir * n);
            vecteur phongDir = lightRay.dir - reflet * n;
            float phongTerm = max_val(phongDir * viewRay.dir, 0.0f) ;
            phongTerm = specvalue * powf(phongTerm, specpower) * coef;
            red += phongTerm * current.red;
            green += phongTerm * current.green;
            blue += phongTerm * current.blue;
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
    n.normalize();
               
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
        
        ray lightRay = {{impact.x, impact.y, impact.z}, {dist.x, dist.y, dist.z}};
        lightRay.dir.normalize(); // On normalise

        // calcul des ombres 
        bool shadow = false; 
        for (unsigned int i = 0; i < myScene.sphTab.size(); ++i) 
        {
            if (hitSphere(lightRay, myScene.sphTab[i], t) && myScene.matTab[myScene.sphTab[i].material].opacity == 1) 
            {
                shadow = true;
                break;
            }
        }
        shadow = false;
        if (!shadow ) 
        {
            // lambert
            float lambert = (lightRay.dir * n) * coef * currentMat.opacity;
            red += lambert * current.red * currentMat.red;
            green += lambert * current.green * currentMat.green;
            blue += lambert * current.blue * currentMat.blue;
        }
        
        if (mode == 1)
        {
            float reflet = 2.0f * (lightRay.dir * n);
            vecteur phongDir = lightRay.dir - reflet * n;
            float phongTerm = max_val(phongDir * viewRay.dir, 0.0f) ;
            phongTerm = specvalue * powf(phongTerm, specpower) * coef;
            red += phongTerm * current.red;
            green += phongTerm * current.green;
            blue += phongTerm * current.blue;
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

ray refract_ray_sphere(scene &myScene, ray &viewRay, float &t, int currentSphere, point &impact)
{
    if (enable_refraction == 0)
        return viewRay;
    
    // C'est deux refractions d'affile :
    material currentMat = myScene.matTab[myScene.sphTab[currentSphere].material]; 
    float ind2 = currentMat.refraction; // indice du materiau

    // Premire refraction : on cree un rayon refracte t_ray:
    vecteur n = impact - myScene.sphTab[currentSphere].pos; // normale au point d'impact
    n = n / sqrtf(n*n);

    vecteur v = viewRay.dir;

    v = v / sqrtf(v*v);
    float c = -1*v * n;
    float r = ind1/ind2 ;
    if (1 - r*r*(1-c*c) < 0)
        return viewRay;
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