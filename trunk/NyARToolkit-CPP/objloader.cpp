#include "objloader.h"

#include <string>
#include <fstream>

// Read the file, and convert it so that it can be used to something usefull
std::vector<Triangle> loadFromFile(char* FileName)
{     
      std::string currentLine;            // Variable that holds the current line
                                          // we are using.
      std::vector<Vertex> vertexList;   // A dynamic vector array for holding
                                          // the info about the vertexs
      //std::vector<UV> uvList;             // Dynamic list of uvs
      std::vector<Vector3D> normal;       // Dynamic list with the normals of 
                                          // the object
                                          
      std::vector<Triangle> mesh;         // The list we return to create the object
                                          // as this is bacis we dont need to 
                                          // return uv and normals
        
      // Open file
      std::ifstream myFile (FileName);
      if (myFile.is_open())
      {
		  Vector3D tmp;
          // While the file is open, we read the info and construct triangles
          while ( getline(myFile, currentLine) ) 
          {
              // Check if this line holds vertex information. If the line starts 
              // with "v ", we will use it.
              if (currentLine[0] == 'v' && currentLine[1] == ' ' )
              {
                  vertexList.push_back(Vertex(processVertex(currentLine,2),tmp));
              }
              
              // Check if the line holds uv information. If its starts with
              // "vt" if a uv.
              if (currentLine[0] == 'v' && currentLine[1] == 't')
              {
                 // Read uv info
                 // My render dont have textures at this point
              }
              
              // Check if the line holds normal information. If its starts with
              // "vn" if a normal.
              if (currentLine[0] == 'v' && currentLine[1] == 'n')
              {
                 // Read normal info
                 // going to add this later
				  tmp = processVertex(currentLine,3);
              }

			  if(currentLine[0] == 'f') {
				  const char* line = currentLine.c_str();
				  char cpy[256] = {0};
				  char* chunk;
				  strcpy(cpy,line);
				  chunk = strtok(cpy," ");

				  chunk = strtok(0,"//");
				  int a = atoi(chunk);
				  chunk = strtok(0," ");
				  chunk = strtok(0,"//");
				  int b = atoi(chunk);
				  chunk = strtok(0," ");
				  chunk = strtok(0,"//");
				  int c = atoi(chunk);

				  mesh.push_back(Triangle(vertexList[a-1],vertexList[b-1],vertexList[c-1]));
			  }
          }
          // Finished with reading the file, close it
          myFile.close();
      }
      
      
      // Create the triangles as the file is loaded
      // Dont work, not sure where the error is
      //mesh = createTriangel(vertexList);
      
      // Temp, just to check. This workd perfect.
      //mesh.push_back(Triangle(Vector3D(0,0,0),Vector3D(-150,0,0),Vector3D(0,150,0)));
      
      return mesh;
}


// Convert a string into a vertex
Vector3D processVertex(std::string line, int beginIdx)
{
         std::string vertexString[3];
         int x = 0;
         
         for (int i = beginIdx; i <= line.size(); i++)
         {
             if ( line[i] == ' ')
             {
                  x++;
             }
             else
             {
                 // add to same cord
                 vertexString[x].push_back(line[i]);
             }
         }
         
         
         // Convert the string into a number
         Vector3D vertex( (float)atof(vertexString[0].c_str()),
                          (float)atof(vertexString[1].c_str()),
                          (float)atof(vertexString[2].c_str())
                          );
         
         return vertex;
}

// Convert the vertex cords into triangles
std::vector<Triangle> createTriangel(std::vector<Vector3D> list)
{
    std::vector<Triangle> triangleList;
    
    for (int i = 0; i < list.size(); i= i + 3 )
    {
        triangleList.push_back(Triangle( list[i + 0],
                                         list[i + 1],
                                         list[i + 2] 
                                         ));
    }
    
    return triangleList;
}
