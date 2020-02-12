#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <time.h>
#include <chrono>
#include <mpi.h>

using namespace std;

#include "raytrace.h"
#include "intersections.cpp"

#define ind1 1.0f // indice de l'air
#define ROOT 0 // Processus maître


bool init(char* inputName, scene &myScene) 
    {
        /* Lit le fichier scene.txt et crée l'objet scene */
        int nbMat, nbSphere, nbPlan, nbParab, nbPara, nbLight;
        int i;
        ifstream sceneFile(inputName);

        if (!sceneFile)
        {
            printf("Erreur d'ouverture du fichier scene\n");
            return  false;
        }

        sceneFile >> myScene.sizex >> myScene.sizey;
        sceneFile >> nbMat >> nbSphere >> nbPlan >> nbParab >> nbPara >> nbLight;
        myScene.matTab.resize(nbMat); 
        myScene.sphTab.resize(nbSphere);
        myScene.planTab.resize(nbPlan); 
        myScene.parabTab.resize(nbParab);
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
        if (nbParab != 0)
        {
            for (i=0; i < nbPlan; i++) 
                sceneFile >> myScene.paraTab[i];
        }
        if (nbPara != 0)
        {
            for (i=0; i < nbPara; i++) 
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


bool draw(const char* outputName, scene &myScene, const std::vector<pixel> pix_img)
{
	ofstream imageFile(outputName,ios_base::binary);
    if (!imageFile)
    {
        printf("erreur de generation fichier outpout\n");
        return false; 
    }
    init_tga(imageFile, myScene);
    for (int i = 0; i < pix_img.size(); i ++)
    {
    	imageFile.put(pix_img[i].b).put(pix_img[i].g).put(pix_img[i].r);
    }
    return true;
} 


bool compute_line(scene &myScene, const int line, std::vector<pixel> &pix_line, int numProcess) 
{
    /* Fonction principale qui balaye tous les pixels de la ligne et calcule les pixels */
    // ============================= On balaye tous les pixels ============================= 

        for (int x = 0; x < myScene.sizex; x++ ) 
        {
            // Initialisation des couleurs et remise a zero des compteurs : 
            float red = 0, green = 0, blue = 0;
            float coef = 1.0f;
            int level = 0; 

            // lancer de rayon 
            ray viewRay = { {float(x), float(line), -10000.0f}, {0, 0, 1}}; // lance un rayon à partir de la position (x, line, -10000) dans la direction z 
            ray refract;

            bool quit_plan = false;
            do { 
                // ============================= recherche de l'intersection la plus proche ============================= 

                float t = 200000.0f; // on part du fond de la scene 
                int currentSphere= -1;
                int currentPlan = -1;
                int currentParab = -1;
                int currentPara = -1;
                int obj_type = -1; // 1 = Sphere; 2 = plan; 3 = paraboloide

                if(!find_intersection(myScene, viewRay, t, currentSphere, currentPlan, currentParab, currentPara, obj_type))
                {
                    break; // Si le rayon n'a rencontre aucun objet, on laisse noir et on passe au suivant
                    // Sinon, on actualise t, current_obj et obj_type
                }

                // ============================= Point d'impact ============================= 
                point impact = viewRay.start + t * viewRay.dir; 

                switch (obj_type)
                {
                    case 1 : // Sphere
                        pix_impactSphere(myScene, red, green, blue, coef, viewRay, t, currentSphere, impact);
                        if (myScene.matTab[myScene.sphTab[currentSphere].material].opacity != 1) // Materiau translucide :
                        {
                            scene newScene;
                            newScene % myScene;
                            newScene.sphTab.erase(newScene.sphTab.begin()+currentSphere);

                            if (level == 0)
                            	viewRay = refract_ray_sphere(myScene, viewRay, t, currentSphere, impact); // Le rayon est refracte si on entre dans un materiau a indice different de celui de l'air
                           	pix_impactSphere(newScene, red, green, blue, coef, viewRay, t, currentSphere, impact);
                        }
                        level++;
                        break;
                    
                    case 2 : // Plan
                        pix_impactPlan(myScene, red, green, blue, coef, viewRay, t, currentPlan, impact);
                       	level++;
                       	break;

                    case 3 : // Paraboloide
                        pix_impactParaboloid(myScene, red, green, blue, coef, viewRay, t, currentParab, impact);
                        level ++;
                        break;
                    case 4 : // paralellogramme
                        printf("coucou !!\n");
                        pix_impactParalello(myScene, red, green, blue, coef, viewRay, t, currentPara, impact);
                        level ++;
                        break;
                }
            } while ((coef > 0.0f) && (level < 10)); // Level est le nombre de rebond qu'on autorise au rayon

            pix_line[x].r = (unsigned char)min(red*255.0f, 255.0f);
            pix_line[x].g = (unsigned char)min(green*255.0f, 255.0f);
            pix_line[x].b = (unsigned char)min(blue*255.0f, 255.0f);
        }
        pix_line[myScene.sizex].r = numProcess;
        pix_line[myScene.sizex+1].r = line;
    return true;
}

int main(int argc, char *argv[] ) 
{ 

	int numtasks , rank;
    MPI_Init ( &argc , &argv );
    MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	MPI_Status Stat;

    MPI_Comm_size ( globComm , &numtasks );
    MPI_Comm_rank ( globComm , &rank);

    // argv[1] = nom du fichier scene.txt et argv[2] = nom du fichier outpout
    if  (rank == ROOT && argc < 3) 
    {
        printf("Pas assez d'arguments ! \n");
        return -1;
    }

    scene myScene;

    if (!init(argv[1], myScene)) // On initialise la scene pour tous les threads
    {
        printf("Erreur d'Initialisation de myScene: mauvais fichier scene ! \n");
        return -1;
    }

    if (rank == ROOT)
    	printf("Initialisation reussie \n");
    
    const int stop=-1;

    if (rank==ROOT)
    {
    	// ========================================== MAÎTRE ==========================================
    	printf("Calcul en cours ... \n");
        std::chrono::time_point<std::chrono::system_clock> t_start;
        t_start = std::chrono::system_clock::now();

        int send_lines, received_lines;
        send_lines=0;
        received_lines=0;

        std::vector<pixel> pix_img(myScene.sizex * myScene.sizey); // Image finale
        std::vector<pixel> tmp_line(myScene.sizex+2); // On doit y stocker une ligne de pixels plus les infos suivantes : de quelle ligne il s'agit et quel thread l'a envoyé

        while(send_lines < numtasks-1) // On distribue du travail à tous les threads
        {
            MPI_Send(&send_lines, 1, MPI_INT, send_lines+1, send_lines, globComm);
            send_lines++;
        }

        while(received_lines < myScene.sizey) // On distribue les taches tant que le travail n'est pas terminé
        {
            MPI_Recv(tmp_line.data(),(myScene.sizex+2)*12, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm, &Stat);
            int tmp_rank=tmp_line[myScene.sizex].r; // On stocke le num de process qui a envoyé dans le r de l'avant dernier pixel 
            int tmp_line_nmb=tmp_line[myScene.sizex+1].r; // On stocke le num de de ligne dans le r du dernier pixel

            received_lines++;
            // Distribution initiale des taches
            for (int k=0; k< myScene.sizex; k++)
            {
                pix_img[myScene.sizex*(tmp_line_nmb)+k]=tmp_line[k];
            }

            if(send_lines < myScene.sizey)
            {
                MPI_Send(&send_lines, 1, MPI_INT, tmp_rank, send_lines, globComm);
                send_lines++;
            }
            // On envoie le message de terminaison
            else
            {
                MPI_Send(&stop, 1, MPI_INT, tmp_rank, send_lines, globComm);
            }

        }
        // On a fini les calculs, affichons le réultat :

        draw(argv[2], myScene, pix_img);


        std::chrono::time_point<std::chrono::system_clock> t_end;
        t_end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_time = t_end-t_start;
        std::cout << "elapsed_time = " << elapsed_time.count() << " s" << std::endl;
    }
    else
    {
    	// ========================================== ESCLAVE ==========================================
        int current_line; 
        std::vector<pixel> pix_line(myScene.sizex+2);

        MPI_Recv(&current_line, 1, MPI_INT, ROOT, MPI_ANY_TAG, globComm, &Stat); // Ordre du maitre
        while(current_line!=-1) 
        {
        	compute_line(myScene, current_line,pix_line, rank);
            MPI_Send(pix_line.data(),(myScene.sizex+2)*12 , MPI_BYTE, ROOT, current_line,globComm);
            // On attend la prochaine instruction
            MPI_Recv(&current_line, 1, MPI_INT, ROOT, MPI_ANY_TAG, globComm, &Stat);
        }
    }

    MPI_Finalize ();
    return EXIT_SUCCESS;   
 }

    
