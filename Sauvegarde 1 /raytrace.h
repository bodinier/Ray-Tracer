/* =========================== DEFINITION DES CLASSES =========================== */ 

struct point {
	float x, y, z;
};


struct vecteur {
	float x, y, z;
};

struct material 
{
	float red, green, blue, reflection, opacity, refraction;
};


struct sphere {
	point pos;
	float size;
	int material;
};

struct plan {
	vecteur normale;
	float d;
	int material;
};

struct light 
{
	point pos;
	float red, green, blue;
};


struct ray 
	{
		point start;
		vecteur dir;
	};

struct scene 
	{
		vector<material> matTab;
		vector<sphere>   sphTab;
		vector<plan> planTab;
		vector<light>    lgtTab;
		int sizex, sizey;
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
		return inputFile >> pl.normale >> pl.d >> pl.material;
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

vecteur operator - (const point&p1, const point &p2)
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

void operator % (scene &scene1, const scene scene2 ) 
	{
		scene1.matTab = scene2.matTab;
		scene1.sphTab = scene2.sphTab;
		scene1.planTab = scene2.planTab;
		scene1.lgtTab = scene2.lgtTab;
		scene1.sizex = scene2.sizex;
		scene1.sizey = scene2.sizey;
	}
