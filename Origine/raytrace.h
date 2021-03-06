/* =========================== DEFINITION DES CLASSES =========================== */ 

struct point {
	float x, y, z;
};


struct vecteur {
	float x, y, z;
};

struct material 
{
	float red, green, blue, reflection;
};


struct sphere {
	point pos;
	float size;
	int material;
};

struct plan {
	point origin;
	vecteur normal;
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
		return inputFile >> mat.red >> mat.green >> mat.blue >> mat.reflection; 
	}

istream & operator >> ( istream &inputFile, sphere& sph ) 
	{
		return inputFile >> sph.pos >> sph.size >> sph.material;
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

