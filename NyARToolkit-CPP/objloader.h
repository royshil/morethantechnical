#include <vector>

using namespace std;

typedef struct Vector3D {
	float x;
	float y;
	float z;
	Vector3D(){x=0.0f;y=0.0f;z=0.0f;}
	Vector3D(const Vector3D& v) {x=v.x;y=v.y;z=v.z;}
	Vector3D(float vx,float vy,float vz) {x = vx; y = vy; z = vz;}
} Vector3D;

typedef struct Vertex {
	Vector3D location;
	Vector3D normal;
	Vertex(){}
	Vertex(Vector3D l){location = l;}
	Vertex(Vector3D l,Vector3D n){location = l;normal = n;}
	Vertex(const Vertex& v) {location=v.location;normal=v.normal;}
} Vertex;

typedef struct Triangle {
	Vertex a;
	Vertex b;
	Vertex c;

	Triangle(Vertex va, Vertex vb, Vertex vc) { a=va;b=vb;c=vc;}
} Triangle;

// Convert a string into a Vector3D
Vector3D processVertex(std::string line,int);

// Create triangle
std::vector<Triangle> createTriangel(std::vector<Vector3D> list);

std::vector<Triangle> loadFromFile(char* FileName);