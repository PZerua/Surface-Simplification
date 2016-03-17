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
		return a.cost < b.cost;
	}
};

class Mesh
{
public:
	std::map<Vector3, vector<unsigned int>, customVec3Comparator> vertexTriangles;
	std::map<Vector3, vector<unsigned int>, customVec3Comparator> vertexEdges;
	vector<Edge> edges;

	std::vector<Vector3> indexed_positions;
	std::vector<Vector3> indexed_normals;
	std::vector<Vector3> indexed_normalsFinal;
	std::vector<Vector2> indexed_uvs;
	std::vector<Triangle> triangles;

	Mesh();
	void clear();
	void render(const int &primitive);

	Matrix44 getTriangleMatrix(const unsigned int &tri);
	Matrix44 getTriangleVectorMatrix(std::vector<unsigned int> triangles);

	void addEdge(const unsigned int &i, const unsigned int &j, const unsigned int &triangleIndex);
	void updateEdges(const unsigned int &i);
	int totalTriangles();

	void computeAllCosts();
	void computeCost(Edge *edge);
	void edgeContraction(const unsigned &numTriang);
	bool loadOBJ(const char* filename);
};



#endif