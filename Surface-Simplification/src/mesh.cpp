#include "mesh.h"
#include <cassert>
#include "includes.h"

#include <string>
#include <sys/stat.h>

std::vector<std::string> tokenize(const std::string& source, const char* delimiters, bool process_strings = false);
Vector2 parseVector2(const char* text);
Vector3 parseVector3(const char* text, const char separator);

std::vector<Vector3> indexed_positions;
std::vector<Vector3> indexed_normals;
std::vector<Vector2> indexed_uvs;


Mesh::Mesh()
{
}

void Mesh::clear()
{
	vertices.clear();
	normals.clear();
	uvs.clear();
}

void Mesh::render(int primitive)
{
	//render the mesh using your rasterizer
	assert(vertices.size() && "No vertices in this mesh");

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &indexed_positions[0]);

	if (normals.size())
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &indexed_normals[0] );
	}

	//glDrawArrays(primitive, 0, vertices.size() );
	glDrawElements(primitive, trianglesT.size(), GL_UNSIGNED_INT, &trianglesT[0]);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (normals.size())
		glDisableClientState(GL_NORMAL_ARRAY);
	if (uvs.size())
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Mesh::createPlane(float size)
{
	vertices.clear();
	normals.clear();
	uvs.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	vertices.push_back( Vector3(size,0,size) );
	vertices.push_back( Vector3(size,0,-size) );
	vertices.push_back( Vector3(-size,0,-size) );
	vertices.push_back( Vector3(-size,0,size) );
	vertices.push_back( Vector3(size,0,size) );
	vertices.push_back( Vector3(-size,0,-size) );

	//all of them have the same normal
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );

	//texture coordinates
	uvs.push_back( Vector2(1,1) );
	uvs.push_back( Vector2(1,0) );
	uvs.push_back( Vector2(0,0) );
	uvs.push_back( Vector2(0,1) );
	uvs.push_back( Vector2(1,1) );
	uvs.push_back( Vector2(0,0) );
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
		else if (tokens[0] == "s") //surface? it appears one time before the faces
		{
			//process mesh
			if (uvs.size() == 0 && indexed_uvs.size() )
				uvs.resize(1);
		}
		else if (tokens[0] == "f" && tokens.size() >= 4)
		{
			Vector3 v1,v2,v3;
			v1 = parseVector3( tokens[1].c_str(), '/' );

			for (unsigned iPoly = 2; iPoly < tokens.size() - 1; iPoly++)
			{
				v2 = parseVector3( tokens[iPoly].c_str(), '/' );
				v3 = parseVector3( tokens[iPoly+1].c_str(), '/' );

				vertices.push_back( indexed_positions[ unsigned int(v1.x) -1] );
				vertices.push_back( indexed_positions[ unsigned int(v2.x) -1] );
				vertices.push_back( indexed_positions[ unsigned int(v3.x) -1] );

				Edge edgeA(unsigned int(v1.x) - 1, unsigned int(v2.x) - 1);
				edges.push_back(edgeA);

				Edge edgeB(unsigned int(v2.x) - 1, unsigned int(v3.x) - 1);
				edges.push_back(edgeB);

				Edge edgeC(unsigned int(v3.x) - 1, unsigned int(v1.x) - 1);
				edges.push_back(edgeC);

				trianglesT.push_back(unsigned int(v1.x) - 1);
				trianglesT.push_back(unsigned int(v2.x) - 1);
				trianglesT.push_back(unsigned int(v3.x) - 1);

				createTrianglePlanes(indexed_positions[unsigned int(v1.x) - 1], indexed_positions[unsigned int(v2.x) - 1], indexed_positions[unsigned int(v3.x) - 1]);

				//triangles.push_back( VECTOR_INDICES_TYPE(vertex_i, vertex_i+1, vertex_i+2) ); //not needed
				vertex_i += 3;

				if (indexed_uvs.size() > 0)
				{
					uvs.push_back( indexed_uvs[ unsigned int(v1.y) -1] );
					uvs.push_back( indexed_uvs[ unsigned int(v2.y) -1] );
					uvs.push_back( indexed_uvs[ unsigned int(v3.y) -1] );
				}

				if (indexed_normals.size() > 0)
				{
					normals.push_back( indexed_normals[ unsigned int(v1.z) -1] );
					normals.push_back( indexed_normals[ unsigned int(v2.z) -1] );
					normals.push_back( indexed_normals[ unsigned int(v3.z) -1] );
				}
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

void Mesh::calculateCost()
{

	for (std::map<Vector3, vector<Matrix44>>::iterator it = vertexPlane.begin(); it != vertexPlane.end(); it++)
	{
		Matrix44 temp = it->second[0];
		for (unsigned i = 1; i < it->second.size(); i++)
		{
			temp = temp + it->second[i];
		}
		vertexQ[it->first] = temp;
	}

	for (unsigned i = 0; i < edges.size(); i++)
	{
		edges[i].Q = vertexQ[indexed_positions[edges[i].a]] + vertexQ[indexed_positions[edges[i].b]];

		Matrix44 temp = edges[i].Q;
		temp.M[3][0] = 0;
		temp.M[3][1] = 0;
		temp.M[3][2] = 0;
		temp.M[3][3] = 1;

		temp.inverse();

		Vector4 temp2(0, 0, 0, 1);

		Vector4 tempW = multM4xV4(temp , temp2);

		edges[i].w = Vector3(tempW.x, tempW.y, tempW.z);

		Vector4 wQ = multV4xM4(tempW, edges[i].Q);

		edges[i].cost = wQ.dot(tempW);

		edgeQueue.push(edges[i]);
	}
	
}


void Mesh::createTrianglePlanes(Vector3 &a, Vector3 &b, Vector3 &c)
{
	/*std::cout << a.x << ", " << a.y << ", " << a.z << std::endl;
	std::cout << b.x << ", " << b.y << ", " << b.z << std::endl;
	std::cout << c.x << ", " << c.y << ", " << c.z << std::endl;*/

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

	vector < Matrix44 > temp;
	temp.push_back(K);

	if (vertexPlane.find(a) != vertexPlane.end())
		vertexPlane[a].push_back(K);
	else vertexPlane[a] = temp;

	if (vertexPlane.find(b) != vertexPlane.end())
		vertexPlane[b].push_back(K);
	else vertexPlane[b] = temp;

	if (vertexPlane.find(c) != vertexPlane.end())
		vertexPlane[c].push_back(K);
	else vertexPlane[c] = temp;

}

void Mesh::edgeContraction()
{
	for (unsigned i = 0; i < edgeQueue.size(); i++)
	{
		// TODO : Delete v2 and recalculate adjacent edges cost

		indexed_positions[edgeQueue.top().a] = edgeQueue.top().w;
		
		auto it = indexed_positions.begin();
		while (it != indexed_positions.end())
		{
			if (*it == indexed_positions[edgeQueue.top().b])
			{
				indexed_positions.erase(it);
				break;
			}
			it++;
		}

		auto it2 = trianglesT.begin();
		int count = 0;
		while (it2 != trianglesT.end())
		{
			if (count == 3)
				count = 0;
			if (*it2 == edgeQueue.top().b)
			{
				it2 = trianglesT.erase(it2);

				if (count == 0)
				{
					it2 = trianglesT.erase(it2 + 1);
					it2 = trianglesT.erase(it2 + 2);
				}
				else if (count == 1)
				{
					it2 = trianglesT.erase(it2 - 1);
					it2 = trianglesT.erase(it2 + 1);
				}
				else if (count == 2)
				{
					it2 = trianglesT.erase(it2 - 1);
					it2 = trianglesT.erase(it2 - 2);
				}
				break;
			}
			it2++;
			count++;
		}

		edgeQueue.pop();
	}
}

int Mesh::totalTriangles()
{
	int numVertex;
	numVertex = vertices.size();
	int totalTriangles = numVertex / 3;
	return totalTriangles;
}