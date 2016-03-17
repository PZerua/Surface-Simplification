#include "mesh.h"
#include <cassert>
#include "includes.h"

#include <string>
#include <sys/stat.h>

std::vector<std::string> tokenize(const std::string& source, const char* delimiters, bool process_strings = false);
Vector2 parseVector2(const char* text);
Vector3 parseVector3(const char* text, const char separator);


Mesh::Mesh()
{
}

void Mesh::clear()
{
	indexed_normals.clear();
	indexed_positions.clear();
}

void Mesh::render(const int &primitive)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//render the mesh using your rasterizer
	assert(indexed_positions.size() && "No vertices in this mesh");

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &indexed_positions[0]);

	if (indexed_normals.size())
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &indexed_normalsFinal[0]);
	}

	//glDrawArrays(primitive, 0, vertices.size() );
	glDrawElements(primitive, triangles.size() * 3, GL_UNSIGNED_INT, &triangles[0]);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (indexed_normals.size())
		glDisableClientState(GL_NORMAL_ARRAY);
}

bool Mesh::loadOBJ(const char* filename)
{
	struct stat stbuffer;

	std::cout << "Loading Mesh: " << filename << std::endl;

	FILE* f = fopen(filename,"rb");
	if (f == NULL)
	{
		std::cerr << "File not found: " << filename << std::endl;
		return false;
	}

	stat(filename,&stbuffer);

	unsigned int size = stbuffer.st_size;
	char* data = new char[size+1];
	fread(data,size,1,f);
	fclose(f);
	data[size] = 0;

	char* pos = data;
	char line[255];
	int i = 0;

	const float max_float = 10000000;
	const float min_float = -10000000;

	unsigned int vertex_i = 0;

	//parse file
	while(*pos != 0)
	{
		if (*pos == '\n') pos++;
		if (*pos == '\r') pos++;

		//read one line
		i = 0;
		while(i < 255 && pos[i] != '\n' && pos[i] != '\r' && pos[i] != 0) i++;
		memcpy(line,pos,i);
		line[i] = 0;
		pos = pos + i;

		//std::cout << "Line: \"" << line << "\"" << std::endl;
		if (*line == '#' || *line == 0) continue; //comment

		//tokenize line
		std::vector<std::string> tokens = tokenize(line," ");

		if (tokens.empty()) continue;

		if (tokens[0] == "v" && tokens.size() == 4)
		{
			Vector3 v( atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()) );
			indexed_positions.push_back(v);
		}
		else if (tokens[0] == "vt" && tokens.size() == 4)
		{
			Vector2 v( atof(tokens[1].c_str()), atof(tokens[2].c_str()) );
			indexed_uvs.push_back(v);
		}
		else if (tokens[0] == "vn" && tokens.size() == 4)
		{
			Vector3 v( atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()) );
			indexed_normals.push_back(v);
		}
		else if (tokens[0] == "f" && tokens.size() >= 4)
		{
			Vector3 v1,v2,v3;
			v1 = parseVector3( tokens[1].c_str(), '/' );

			if (!indexed_normalsFinal.size())
				indexed_normalsFinal.assign(indexed_normals.size(), Vector3(0, 0, 0));

			for (unsigned iPoly = 2; iPoly < tokens.size() - 1; iPoly++)
			{
				v2 = parseVector3( tokens[iPoly].c_str(), '/' );
				v3 = parseVector3( tokens[iPoly+1].c_str(), '/' );

				if (indexed_normalsFinal.size())
				{
					indexed_normalsFinal[v1.x - 1] = indexed_normals[v1.z - 1];
					indexed_normalsFinal[v2.x - 1] = indexed_normals[v2.z - 1];
					indexed_normalsFinal[v3.x - 1] = indexed_normals[v3.z - 1];
				}

				Triangle tri(unsigned int(v1.x) - 1, unsigned int(v2.x) - 1, unsigned int(v3.x) - 1);
				unsigned int triIndex = this->triangles.size();
				this->triangles.push_back(tri);

				this->addEdge(tri.i, tri.j, triIndex);
				this->addEdge(tri.j, tri.k, triIndex);
				this->addEdge(tri.k, tri.i, triIndex);
				
				vector<unsigned int> vecAux;
				vecAux.push_back(triIndex);

				Vector3 vec3 = this->indexed_positions[tri.i];
				if(vertexTriangles.find(vec3) == vertexTriangles.end())
					vertexTriangles[vec3] = vecAux;
				else vertexTriangles[vec3].push_back(triIndex);
				
				vec3 = this->indexed_positions[tri.j];
				if(vertexTriangles.find(vec3) == vertexTriangles.end())
					vertexTriangles[vec3] = vecAux;
				else vertexTriangles[vec3].push_back(triIndex);
				
				vec3 = this->indexed_positions[tri.k];
				if(vertexTriangles.find(vec3) == vertexTriangles.end())
					vertexTriangles[vec3] = vecAux;
				else vertexTriangles[vec3].push_back(triIndex);
			}
		}
	}

	delete data;

	return true;
}


std::vector<std::string> tokenize(const std::string& source, const char* delimiters, bool process_strings )
{
	std::vector<std::string> tokens;

	std::string str;
	char del_size = strlen(delimiters);
	const char* pos = source.c_str();
	char in_string = 0;
	int i = 0;
	while(*pos != 0)
	{
		bool split = false;

		if (!process_strings || (process_strings && in_string == 0))
		{
			for (i = 0; i < del_size && *pos != delimiters[i]; i++);
			if (i != del_size) split = true;
		}

		if (process_strings && (*pos == '\"' || *pos == '\'') )
		{ 
			if (!str.empty() && in_string == 0) //some chars remaining
			{
				tokens.push_back(str);
				str.clear();
			}
			
			in_string = (in_string != 0 ? 0 : *pos );
			if (in_string == 0) 
			{
				str += *pos;
				split = true;
			}
		}

		if (split)
		{
			if (!str.empty())
			{
				tokens.push_back(str);
				str.clear();
			}
		}
		else
			str += *pos;
		pos++;
	}
	if (!str.empty())
		tokens.push_back(str);
	return tokens;
}

Vector2 parseVector2(const char* text)
{
	int pos = 0;
	char num[255];
	const char* start = text;
	const char* current = text;
	Vector2 result;

	while(1)
	{
		if (*current == ',' || (*current == '\0' && current != text) )
		{
			strncpy(num, start, current - start);
			num[current - start] = '\0';
			start = current + 1;
			switch(pos)
			{
				case 0: result.x = (float)atof(num); break;
				case 1: result.y = (float)atof(num); break;
				default: return result; break;
			}
			++pos;
			if (*current == '\0')
				break;
		}

		++current;
	}
	return result;
}

Vector3 parseVector3(const char* text, const char separator)
{
	int pos = 0;
	char num[255];
	const char* start = text;
	const char* current = text;
	Vector3 result;

	while(1)
	{
		if (*current == separator || (*current == '\0' && current != text) )
		{
			strncpy(num, start, current - start);
			num[current - start] = '\0';
			start = current + 1;
			if (num[0] != 'x') //¿?
				switch(pos)
				{
					case 0: result.x = (float)atof(num); break;
					case 1: result.y = (float)atof(num); break;
					case 2: result.z = (float)atof(num); break;
					default: return result; break;
				}

			++pos;
			if (*current == '\0')
				break;
		}

		++current;
	}

	return result;
};

Matrix44 Mesh::getTriangleMatrix(const unsigned int &index)
{
	Triangle tri = this->triangles[index];
	Vector3 a = this->indexed_positions[tri.i];
	Vector3 b = this->indexed_positions[tri.j];
	Vector3 c = this->indexed_positions[tri.k];

	Vector3 AB(b.x - a.x, b.y - a.y, b.z - a.z);
	Vector3 BC(c.x - b.x, c.y - b.y, c.z - b.z);

	Vector3 normal = AB.cross(BC);
	normal.normalize();

	float D(-(a.x * normal.x) - (a.y * normal.y) - (a.z * normal.z));

	Matrix44 K;

	K.M[0][0] = normal.x * normal.x;
	K.M[0][1] = normal.x * normal.y;
	K.M[0][2] = normal.x * normal.z;
	K.M[0][3] = normal.x * D;
	K.M[1][0] = normal.x * normal.y;
	K.M[2][0] = normal.x * normal.z;
	K.M[3][0] = normal.x * D;

	K.M[1][1] = normal.y * normal.y;
	K.M[1][2] = normal.y * normal.z;
	K.M[1][3] = normal.y * D;
	K.M[2][1] = normal.y * normal.z;
	K.M[3][1] = normal.y * D;

	K.M[2][2] = normal.z * normal.z;
	K.M[2][3] = normal.z * D;
	K.M[3][2] = normal.z * D;

	K.M[3][3] = D * D;

	return K;
}

Matrix44 Mesh::getTriangleVectorMatrix(std::vector<unsigned int> indices)
{
	Matrix44 Q;

	//cout << "Triangles: " << indices.size() << endl;
	
	if(indices.size() > 0)
	{
		Q = this->getTriangleMatrix(indices[0]);
		for (unsigned int i = 1; i < indices.size(); ++i)
		{
			//cout << "\tTriangle: " << indices[i] << endl;
			Q = Q + this->getTriangleMatrix(indices[i]);
		}
	}

	return Q;
}

void Mesh::computeAllCosts()
{
	sort(edges.begin(), edges.end(), LessCost());
	for (unsigned i = 0; i < edges.size(); i++)
	{
		this->computeCost(&edges[i]);
	}
}

void Mesh::computeCost(Edge *edge)
{
	edge->Q = this->getTriangleVectorMatrix(this->vertexTriangles[this->indexed_positions[edge->a]]) +
		this->getTriangleVectorMatrix(this->vertexTriangles[this->indexed_positions[edge->b]]);

	Matrix44 temp = edge->Q;
	temp.M[3][0] = 0;
	temp.M[3][1] = 0;
	temp.M[3][2] = 0;
	temp.M[3][3] = 1;

	temp.inverse();

	Vector4 temp2(0, 0, 0, 1);

	Vector4 tempW = multM4xV4(temp, temp2);

	edge->w = Vector3(tempW.x, tempW.y, tempW.z);

	Vector4 wQ = multV4xM4(tempW, edge->Q);

	edge->cost = wQ.dot(tempW);
}

void Mesh::edgeContraction(const unsigned &numTriang)
{

	unsigned triangSize = triangles.size();
	int rest = triangSize - numTriang;
	if (rest > 0)
	{
		int count = 0;
		while (count < ((rest) / 2))
		{
			sort(edges.begin(), edges.end(), LessCost());
			Edge e = edges[0];
			edges.erase(edges.begin());

			vector<unsigned int> triA = this->vertexTriangles[this->indexed_positions[e.a]];
			vector<unsigned int> triB = this->vertexTriangles[this->indexed_positions[e.b]];
			//remove all the edges affected by the change
			for (unsigned i = 0; i < edges.size(); i++)
			{
				if ((std::find(triA.begin(), triA.end(), edges[i].triangleIndex) != triA.end())
					|| (std::find(triB.begin(), triB.end(), edges[i].triangleIndex) != triB.end()))
				{
					this->edges.erase(this->edges.begin() + i);
					i -= 1;
				}
			}
			cout << "Removing triangles of the edge... " << e.a << " - " << e.b << endl;
			//we remove the triangles containing the edge from triA
			for (unsigned i = 0; i < triA.size(); i++)
			{
				Triangle tri = this->triangles[triA[i]];
				//cout << "\tTriangle: " << tri.i << ", " << tri.j << ", " << tri.k << endl;
				if (tri.containsIndex(e.b))
				{
					this->triangles.erase(this->triangles.begin() + triA[i]);
					for (unsigned j = 0; j < edges.size(); j++)
					{
						if (edges[j].triangleIndex > triA[i])
							edges[j].triangleIndex -= 1;
					}
					cout << "\t\tDestroy: " << triA[i] << endl;
					this->vertexTriangles[this->indexed_positions[e.a]].erase(
						this->vertexTriangles[this->indexed_positions[e.a]].begin() + i);
					std::map<Vector3, vector<unsigned int>>::iterator it;
					for (it = this->vertexTriangles.begin(); it != this->vertexTriangles.end(); ++it)
					{
						for (unsigned j = 0; j < it->second.size(); j++)
						{
							if (it->second[j] == triA[i])
								it->second.erase(it->second.begin() + (j--));
							else if (it->second[j] > triA[i])
								it->second[j] -= 1;
						}
					}
					triA = this->vertexTriangles[this->indexed_positions[e.a]];
					//triB = this->vertexTriangles[e.b];
					i -= 1;
				}
			}
			//refresh both vectors to get the new indices
			triA = this->vertexTriangles[this->indexed_positions[e.a]];
			triB = this->vertexTriangles[this->indexed_positions[e.b]];
			//e.a is now the new vertex w
			this->indexed_positions[e.a] = e.w;
			//this->indexed_positions[e.b] = e.w;
			//replace all indices of e.b for e.a
			for (unsigned i = 0; i < triB.size(); i++)
			{
				Triangle tri = this->triangles[triB[i]];
				if (tri.i == e.b) this->triangles[triB[i]].i = e.a;
				if (tri.j == e.b) this->triangles[triB[i]].j = e.a;
				if (tri.k == e.b) this->triangles[triB[i]].k = e.a;
			}
			//add the triangles of the new vertex
			vector<unsigned int> auxTriangles;
			auxTriangles.insert(auxTriangles.end(), triA.begin(), triA.end());
			auxTriangles.insert(auxTriangles.end(), triB.begin(), triB.end());
			this->vertexTriangles[this->indexed_positions[e.a]] = auxTriangles;
			//add the edges of the new triangles and compute the cost
			//update all edges that may ahve changed cost and w
			for (unsigned i = 0; i < triA.size(); i++)
			{
				Triangle t = this->triangles[triA[i]];
				this->addEdge(t.i, t.j, triA[i]);
				this->addEdge(t.j, t.k, triA[i]);
				this->addEdge(t.k, t.i, triA[i]);

				this->updateEdges(t.i);
				this->updateEdges(t.j);
				this->updateEdges(t.k);
			}
			for (unsigned i = 0; i < triB.size(); i++)
			{
				Triangle t = this->triangles[triB[i]];
				this->addEdge(t.i, t.j, triB[i]);
				this->addEdge(t.j, t.k, triB[i]);
				this->addEdge(t.k, t.i, triB[i]);

				this->updateEdges(t.i);
				this->updateEdges(t.j);
				this->updateEdges(t.k);
			}
			//finally remove the vertex e.b from the list of vertices
			this->indexed_positions.erase(this->indexed_positions.begin() + e.b);
			this->indexed_normalsFinal.erase(this->indexed_normalsFinal.begin() + e.b);
			//update all the indices inside the edges accordingly
			for (unsigned int i = 0; i < this->edges.size(); i++)
			{
				if (this->edges[i].a > e.b) this->edges[i].a -= 1;
				if (this->edges[i].b > e.b) this->edges[i].b -= 1;
			}
			//and inside the triangles as well
			for (unsigned int i = 0; i < this->triangles.size(); i++)
			{
				if (this->triangles[i].i > e.b) this->triangles[i].i -= 1;
				if (this->triangles[i].j > e.b) this->triangles[i].j -= 1;
				if (this->triangles[i].k > e.b) this->triangles[i].k -= 1;
			}
			count++;
		}

		cout << "Finished edgeContraction!" << endl;
	}
	else cout << "Can't do a contraction to " << numTriang << " triangles, the resultant mesh would have less than 0 triangles" << endl;
}

void Mesh::addEdge(const unsigned int &i, const unsigned int &j, const unsigned int &triangleIndex)
{
	Edge e1(i, j);
	e1.triangleIndex = triangleIndex;
	this->computeCost(&e1);
	edges.push_back(e1);

	Vector3 vi = this->indexed_positions[i];
	Vector3 vj = this->indexed_positions[j];

	vector<unsigned int> vecAux1;
	vecAux1.push_back(edges.size() - 1);
	if (this->vertexEdges.find(vi) != this->vertexEdges.end())
		this->vertexEdges[vi] = vecAux1;
	else this->vertexEdges[vi].push_back(edges.size() - 1);

	vector<unsigned int> vecAux2;
	vecAux2.push_back(edges.size() - 1);
	if (this->vertexEdges.find(vj) != this->vertexEdges.end())
		this->vertexEdges[vj] = vecAux2;
	else this->vertexEdges[vj].push_back(edges.size() - 1);
}

void Mesh::updateEdges(const unsigned int &i)
{
	Vector3 vec = this->indexed_positions[i];
	if (this->vertexEdges.find(vec) != this->vertexEdges.end())
	{
		vector<unsigned int> edgeIndices = this->vertexEdges[vec];
		for (unsigned int i = 0; i < edgeIndices.size(); i++)
		{
			Edge e = this->edges[edgeIndices[i]];
			this->computeCost(&e);
		}
	}
}

int Mesh::totalTriangles()
{
	return triangles.size();
}