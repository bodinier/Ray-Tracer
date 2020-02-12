#pragma once 
/* =========================== DEFINITION DES CLASSES =========================== */ 

class point {
	public :

	float x, y, z;
};


class vecteur {
	public : 

	float x, y, z;

	void normalize(){
		float norm = sqrtf(x*x + y*y + z*z);

		x /=norm;
		y /=norm;
		z /=norm;
	}
};

class pixel {
	public :

	int r, g, b;
};

class material 
{
	public : 

	float red, green, blue, reflection, opacity, refraction;
};

class paralello{
	public :

	point A;
	point B;
	point C;
	int material;
};


class sphere {
	public : 

	point pos;
	float size;
	int material;
};

class plan {
	public : 

	vecteur normale;
	float d;
	int material;
	int textureOn;
};

class paraboloid {
	public : 

	float a, b;
	int material;
};

class light 
{
	public : 

	point pos;
	float red, green, blue;
};


class ray 
	{
		public : 

		point start;
		vecteur dir;
	};

class scene 
	{
		public :
		scene(){};
		//scene(const scene &){};

		vector<material> matTab;
		vector<sphere> sphTab;
		vector<plan> planTab;
		vector<paraboloid> parabTab;
		vector<paralello> paraTab;
		vector<light> lgtTab;
		int sizex, sizey;
		int nbRebond;
		bool shadow;
		bool phong;

		private :

	};

/* =========================== DEFINITION DES OPERATEURS DE FLUX =========================== */

istream & operator >> ( istream &inputFile,  point& p ) 
	{
		return inputFile >> p.x >> p.y >> p.z ; 
	}

istream & operator >> ( istream &inputFile,  vecteur& v ) 
	{
		return inputFile >> v.x >> v.y >> v.z ; 
	}


istream & operator >> ( istream &inputFile, material& mat ) 
	{
		return inputFile >> mat.red >> mat.green >> mat.blue >> mat.reflection >> mat.opacity >> mat.refraction; 
	}

istream & operator >> ( istream &inputFile, sphere& sph ) 
	{
		return inputFile >> sph.pos >> sph.size >> sph.material;
	}

istream & operator >> ( istream &inputFile, plan& pl) 
	{
		return inputFile >> pl.normale >> pl.d >> pl.textureOn >> pl.material;
	}

istream & operator >> ( istream &inputFile, paraboloid& para) 
	{
		return inputFile >> para.a >> para.b >> para.material;
	}

istream & operator >> ( istream &inputFile, paralello& para) 
	{
		return inputFile >> para.A >> para.B >> para.C >> para.material;
	}

istream & operator >> ( istream &inputFile, light& lig ) 
	{
		return inputFile >> lig.pos >> lig.red >> lig.green >> lig.blue;
	}

/* =========================== DEFINITION DES OPERATEURS =========================== */

point operator + (const point&p, const vecteur &v)
	{
		point p2={p.x + v.x, p.y + v.y, p.z + v.z };
		return p2;
	}

vecteur operator + (const vecteur &u, const vecteur &v)
	{
		vecteur w = {u.x + v.x, u.y + v.y, u.z + v.z };
		return w;
	}

point operator - (const point&p, const vecteur &v)
	{
		point p2={p.x - v.x, p.y - v.y, p.z - v.z };
		return p2;
	}

vecteur operator - (const point &p1, const point &p2)
	{
		vecteur v={p1.x - p2.x, p1.y - p2.y, p1.z - p2.z };
		return v;
	}

vecteur operator * (float c, const vecteur &v)
	{
		vecteur v2={v.x *c, v.y * c, v.z * c };
		return v2;
	}

vecteur operator / (const vecteur &v, float c)
	{
		vecteur v2={v.x / c, v.y / c, v.z / c };
		return v2;
	}

vecteur operator - (const vecteur&v1, const vecteur &v2)
	{
		vecteur v={v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
		return v;
	}

float operator * (const vecteur&v1, const vecteur &v2 ) 
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

float operator * (const point &v1, const vecteur &v2 ) 
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
vecteur operator ^(const vecteur u, const vecteur v)
{
	vecteur w;
	w.x = u.y*v.z - u.z*v.y;
	w.y = u.z*v.x - u.x*v.z;
	w.z = u.x*v.y - u.y*v.x;
	return w;
}

void operator % (scene &scene1, const scene scene2 ) // Operateur de copie
	{
		scene1.matTab = scene2.matTab;
		scene1.sphTab = scene2.sphTab;
		scene1.planTab = scene2.planTab;
		scene1.lgtTab = scene2.lgtTab;
		scene1.sizex = scene2.sizex;
		scene1.sizey = scene2.sizey;
	}
