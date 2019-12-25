#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <time.h>
#include <chrono>

using namespace std;

#include "raytrace.h"
#define ind1 1.0f // indice de l'air

bool init(char* inputName, scene &myScene) 
    {
        int nbMat, nbSphere, nbPlan, nbLight;
        int i;
        ifstream sceneFile(inputName);

        if (!sceneFile)
        {
            printf("Erreur d'ouverture du fichier scene\n");
            return  false;
        }

        sceneFile >> myScene.sizex >> myScene.sizey;
        sceneFile >> nbMat >> nbSphere >> nbPlan >> nbLight;
        myScene.matTab.resize(nbMat); 
        myScene.sphTab.resize(nbSphere);
        myScene.planTab.resize(nbPlan); 
        myScene.lgtTab.resize(nbLight); 
        for (i=0; i < nbMat; i++) 
            sceneFile >> myScene.matTab[i];
        for (i=0; i < nbSphere; i++) 
            sceneFile >> myScene.sphTab[i];
        for (i=0; i < nbPlan; i++) 
            sceneFile >> myScene.planTab[i];
        for (i=0; i < nbLight; i++)
            sceneFile >> myScene.lgtTab[i];
        return true;
    } 

bool hitPlan(const ray &r, const plan &pl, float &t)
{
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
    // intersection rayon/sphere ssi distance entre pt de la droite et centre sphere < R :
    vecteur dist = s.pos - r.start; 
    float B = r.dir * dist;
    float D = B*B - dist * dist + s.size * s.size; // D est le determinant 
    if (D < 0.0f) 
        return false; // Déterminant nul -> pas d'intersections
    float t0 = B - sqrtf(D); //1ere racine
    float t1 = B + sqrtf(D); // 2eme racine
    bool retvalue = false;  // Si retvalue, il faut actualiser un pixel
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

void init_img(ofstream& imageFile, scene& myScene)
{
    // Ajout du header TGA
    imageFile.put(0).put(0);
    imageFile.put(2);        /* RGB non compresse */

    imageFile.put(0).put(0);
    imageFile.put(0).put(0);
    imageFile.put(0);

    imageFile.put(0).put(0); /* origine X */ 
    imageFile.put(0).put(0); /* origine Y */

    imageFile.put((myScene.sizex & 0x00FF)).put((myScene.sizex & 0xFF00) / 256); // 0x00FF = 255 en hexa
    imageFile.put((myScene.sizey & 0x00FF)).put((myScene.sizey & 0xFF00) / 256); 
    imageFile.put(24);       /* 24 bit bitmap */ 
    imageFile.put(0); 
    // fin du header TGA
}

void pix_impactPlan(scene &myScene, float &red, float &green, float &blue, float &coef, ray &viewRay, float &t, int currentPlan, point &impact)
{
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

bool find_intersection(scene &myScene, ray &viewRay, float &t, int &currentSphere, int &currentPlan, int &obj_type)
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

bool draw(char* outputName, scene &myScene) 
{
    ofstream imageFile(outputName,ios_base::binary);
    if (!imageFile)
    {
        printf("erreur de generation fichier outpout\n");
        return false; 
    }
    init_img(imageFile, myScene);
    // ============================= On balaye tous les pixels ============================= 

    for (int y = 0; y < myScene.sizey; y++ ) 
    { 
        for (int x = 0; x < myScene.sizex; x++ ) 
        {
            // Initialisation des couleurs : 
            float red = 0, green = 0, blue = 0;
            float coef = 1.0f;
            int level = 0; 

            // lancer de rayon 
            ray viewRay = { {float(x), float(y), -10000.0f}, {0, 0, 1}}; // lance un rayon à partir de la position (x, y, -10000) dans la direction z 
            ray refract;
            do { 
                // ============================= recherche de l'intersection la plus proche ============================= 

                float t = 200000.0f; // on part du fond de la scene 
                int currentSphere= -1;
                int currentPlan = -1;
                int obj_type = -1;

                if(!find_intersection(myScene, viewRay, t, currentSphere, currentPlan, obj_type))
                {
                    break;
                }

                // ============================= Point d'impact ============================= 
                point impact = viewRay.start + t * viewRay.dir; // impact est le point d'intersection du rayon et de l'objet
                switch (obj_type)
                {
                    case 1 :
                    
                        pix_impactSphere(myScene, red, green, blue, coef, viewRay, t, currentSphere, impact);
                        if (myScene.matTab[myScene.sphTab[currentSphere].material].opacity != 1)
                        {
                            scene newScene;
                            newScene % myScene;
                            newScene.sphTab.erase(newScene.sphTab.begin()+currentSphere);
                            /*if (level == 0)
                                newScene.sphTab.erase(newScene.sphTab.begin()+currentSphere-1);*/
                            if (level == 0 )
                                viewRay = refract_ray_sphere_tmp(myScene, viewRay, t, currentSphere, impact);
                            pix_impactSphere(newScene, red, green, blue, coef, viewRay, t, currentSphere, impact);
                        }
                        level++;
                        break;
                    
                    case 2 :
                    
                        pix_impactPlan(myScene, red, green, blue, coef, viewRay, t, currentPlan, impact);
                        level++;
                        break;
                    
                }
            } while ((coef > 0.0f) && (level < 5));   

            imageFile.put((unsigned char)min(blue*255.0f,255.0f)).put((unsigned char)min(green*255.0f, 255.0f)).put((unsigned char)min(red*255.0f, 255.0f));
        }
    }
    return true;
}

int main(int argc, char* argv[]) 
{
    // argv[1] = nom du fichier scene.txt et argv[2] = nom du fichier outpout
    printf("Calcul en cours ... \n");
    if  (argc < 3) 
    {
        printf("Pas assez d'arguments ! \n");
        return -1;
    }

    scene myScene;

    if (!init(argv[1], myScene))
    {
        printf("Erreur d'Initialisation : mauvais fichier scene ! \n");
        return -1;
    }
    printf("Initialisation reussie \n");

    std::chrono::time_point<std::chrono::system_clock> start,end;
    start = std::chrono::system_clock::now();
    if (!draw(argv[2], myScene))
    {
        printf("Erreur 3 ! \n");
        return -1;
    }
    printf("Termine ! \n");

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = end-start;
    std::cout << "elapsed_time = " << elapsed_time.count() << " s" << std::endl;
    return 0;
}
