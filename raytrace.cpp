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
#include "intersections.cpp"

#define ind1 1.0f // indice de l'air

bool init(char* inputName, scene &myScene) 
    {
        /* Lit le fichier scene.txt et crée l'objet scene */
        int nbMat, nbSphere, nbPlan, nbPara, nbLight;
        int i;
        ifstream sceneFile(inputName);

        if (!sceneFile)
        {
            printf("Erreur d'ouverture du fichier scene\n");
            return  false;
        }

        sceneFile >> myScene.sizex >> myScene.sizey;
        sceneFile >> nbMat >> nbSphere >> nbPlan >> nbPara >> nbLight;
        myScene.matTab.resize(nbMat); 
        myScene.sphTab.resize(nbSphere);
        myScene.planTab.resize(nbPlan); 
        myScene.paraTab.resize(nbPara);
        myScene.lgtTab.resize(nbLight); 
        for (i=0; i < nbMat; i++) 
            sceneFile >> myScene.matTab[i];
        for (i=0; i < nbSphere; i++) 
            sceneFile >> myScene.sphTab[i];
        if (nbPlan !=0)
        {
            for (i=0; i < nbPlan; i++) 
                sceneFile >> myScene.planTab[i];
        }
        if (nbPara != 0)
        {
            for (i=0; i < nbPlan; i++) 
                sceneFile >> myScene.paraTab[i];
        }
        for (i=0; i < nbLight; i++)
            sceneFile >> myScene.lgtTab[i];
        return true;
    } 

void init_tga(ofstream& imageFile, scene& myScene)
{
    /* Initialise le header du fichier TGA de sortie */
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

bool draw(char* outputName, scene &myScene) 
{
    /* Fonction principale qui balaye tous les pixels et dessine l'image */
    ofstream imageFile(outputName,ios_base::binary);
    if (!imageFile)
    {
        printf("erreur de generation fichier outpout\n");
        return false; 
    }
    init_tga(imageFile, myScene);
    // ============================= On balaye tous les pixels ============================= 
    for (int y = 0; y < myScene.sizey; y++ ) 
    { 
        for (int x = 0; x < myScene.sizex; x++ ) 
        {
            // Initialisation des couleurs et remise a zero des compteurs : 
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
                int currentPara = -1;
                int obj_type = -1; // 1 = Sphere; 2 = plan; 3 = paraboloide

                if(!find_intersection(myScene, viewRay, t, currentSphere, currentPlan, currentPara, obj_type))
                {
                    break; // Si le rayon n'a rencontre aucun objet, on laisse noir et on passe au suivant
                }

                // ============================= Point d'impact ============================= 
                point impact = viewRay.start + t * viewRay.dir; 
                switch (obj_type)
                {
                    case 1 : // Sphere
                        pix_impactSphere(myScene, red, green, blue, coef, viewRay, t, currentSphere, impact);
                        if (myScene.matTab[myScene.sphTab[currentSphere].material].opacity != 1) // Materiau translucide :
                        {
                            //scene newScene(myScene);
                            scene newScene;
                            newScene % myScene;
                            newScene.sphTab.erase(newScene.sphTab.begin()+currentSphere);
                            /*if (level == 0)
                                newScene.sphTab.erase(newScene.sphTab.begin()+currentSphere-1);*/
                            if (level == 0 )
                                viewRay = refract_ray_sphere_tmp(myScene, viewRay, t, currentSphere, impact); // Le rayon est refracte si on entre dans un materiau a indice different de celui de l'air
                            pix_impactSphere(newScene, red, green, blue, coef, viewRay, t, currentSphere, impact);
                        }
                        level++;
                        break;
                    
                    case 2 : // Plan
                        pix_impactPlan(myScene, red, green, blue, coef, viewRay, t, currentPlan, impact);
                        level++;
                        break;

                    case 3 : // Paraboloide
                        pix_impactParaboloid(myScene, red, green, blue, coef, viewRay, t, currentPara, impact);
                        level ++;
                        break;
                    
                }
            } while ((coef > 0.0f) && (level < 10)); // Level est le nombre de rebond qu'on autorise au rayon

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
