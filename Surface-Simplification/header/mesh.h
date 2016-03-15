/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	The Mesh contains the info about how to render a mesh and also how to parse it from a file.
*/

#ifndef MESH_H
#define MESH_H

#include <vector>
#include "framework.h"
#include <map>
#include <queue>

using namespace std;

struct customVec3Comparator {
	bool operator()(const Vector3& a, const Vector3& b) const 
	{
		if (a.x != b.x)
			return (a.x < b.x);

		if (a.y != b.y)
			return (a.y < b.y);

		return (a.z < b.z);
	}
};

struct LessCost
{
	bool operator()(const Edge& a, const Edge& b) const
	{
		return a.cost > b.cost;
	}
};

class Mesh
{
public:
	std::vector< Vector3 > vertices; //here we store the vertices
	std::vector< Vector3 > normals;	 //here we store the normals
	std::vector< Vector2 > uvs;	 //here we store the texture coordinates
	std::map<Vector3, vector< Matrix44 >, customVec3Comparator> vertexPlane;
	std::map<Vector3, Matrix44, customVec3Comparator> vertexQ;
	std::vector< unsigned > trianglesT;
	vector < Edge > edges;
	priority_queue <Edge, vector<Edge>, LessCost> edgeQueue;

	Mesh();
	void clear();
	void render(int primitive); //TODO
	void createTrianglePlanes(Vector3 &a, Vector3 &b, Vector3 &c);
	void calculateCost();
	void edgeContraction();
	int totalTriangles();

	void createPlane(float size);
	bool loadOBJ(const char* filename);
};



#endif