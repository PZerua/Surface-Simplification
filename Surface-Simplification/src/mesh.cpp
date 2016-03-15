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

void Mesh::render(int primitive)
{
	//render the mesh using your rasterizer
	assert(indexed_positions.size() && "No vertices in this mesh");

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &indexed_positions[0]);

	if (indexed_normals.size())
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &indexed_normals[0] );
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

			for (unsigned iPoly = 2; iPoly < tokens.size() - 1; iPoly++)
			{
				v2 = parseVector3( tokens[iPoly].c_str(), '/' );
				v3 = parseVector3( tokens[iPoly+1].c_str(), '/' );

				Triangle tri(unsigned int(v1.x) - 1, unsigned int(v2.x) - 1, unsigned int(v3.x) - 1);
				triangles.push_back(tri);

				edges.push_back(Edge(tri.i, tri.j));
				edges.push_back(Edge(tri.j, tri.k));
				edges.push_back(Edge(tri.k, tri.i));
				
				vector<unsigned int> vecAux;
				vecAux.push_back(triangles.size() - 1);

				Vector3 aux = indexed_positions[tri.i];
				if(vertexTriangles.find(aux) == vertexTriangles.end())
					vertexTriangles[aux] = vecAux;
				else vertexTriangles[aux].push_back(triangles.size() - 1);
				
				aux = indexed_positions[tri.j];
				if(vertexTriangles.find(aux) == vertexTriangles.end())
					vertexTriangles[aux] = vecAux;
				else vertexTriangles[aux].push_back(triangles.size() - 1);
				
				aux = indexed_positions[tri.k];
				if(vertexTriangles.find(aux) == vertexTriangles.end())
					vertexTriangles[aux] = vecAux;
				else vertexTriangles[aux].push_back(triangles.size() - 1);
			}
		}
	}

	delete data;

	//cout << "Triangles Size: " << this->triangles.size() << endl;

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

Matrix44 Mesh::getTriangleMatrix(unsigned int index)
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
	Matrix44 K;

	//cout << "Triangles: " << indices.size() << endl;
	
	if(indices.size() > 0)
	{
		K = this->getTriangleMatrix(indices[0]);
		for (unsigned int i = 1; i < indices.size(); ++i)
		{
			//cout << "\tTriangle: " << indices[i] << endl;
			K = K + this->getTriangleMatrix(indices[i]);
		}
	}

	return K;
}

void Mesh::calculateCost()
{
	for (unsigned i = 0; i < edges.size(); i++)
	{
		edges[i].Q = this->getTriangleVectorMatrix(this->vertexTriangles[this->indexed_positions[edges[i].a]]) + 
			this->getTriangleVectorMatrix(this->vertexTriangles[this->indexed_positions[edges[i].b]]);

		Matrix44 temp = edges[i].Q;
		temp.M[3][0] = 0;
		temp.M[3][1] = 0;
		temp.M[3][2] = 0;
		temp.M[3][3] = 1;

		temp.inverse();

		Vector4 temp2(0, 0, 0, 1);

		Vector4 tempW = multM4xV4(temp, temp2);

		edges[i].w = Vector3(tempW.x, tempW.y, tempW.z);

		Vector4 wQ = multV4xM4(tempW, edges[i].Q);

		edges[i].cost = wQ.dot(tempW);
	}

	sort(edges.begin(), edges.end(), LessCost());
}

void Mesh::edgeContraction(unsigned int removeCount)
{
 
    int count = 0;
 
    while (count < 1)
    {
		Edge e = edges[count];

		vector<unsigned int> triA = this->vertexTriangles[this->indexed_positions[e.a]];
		vector<unsigned int> triB = this->vertexTriangles[this->indexed_positions[e.b]];

		for(int i = 0; i < triA.size();i++)
		{
			cout << "\Check: " << e.a << " - " << e.b << " --> " << this->triangles[triA[i]].containsIndices(e.a, e.b) << endl;
			if(this->triangles[triA[i]].containsIndices(e.a, e.b))
			{
				this->triangles.erase(this->triangles.begin() + triA[i]);
				cout << "Destroy: " << triA[i] << endl;
			}
		}
       
        count++;
    }

	cout << "Finished edgeContraction!" << endl;
}