#pragma once
#include <vector>
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <string>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "meshes.h"
class objectHandler
{
    struct renderObject
    {
        const unsigned int uniqueID;
        glm::mat4 location;
        int mesh;
        GLuint texture;
        GLuint gProgramId;
        GLuint modelLoc;
        GLuint objectColorLoc;
        renderObject(unsigned int uniqueID) : uniqueID(uniqueID) {};
    };
    Meshes meshes;
    unsigned int freeID;
    std::vector<renderObject> objectList;
    void renderPlane(renderObject item);
public:
    std::vector<renderObject> getObjectList(){ return objectList; }
    unsigned int addObject(glm::mat4 location, GLuint texture, std::string objectType, GLuint gProgramId);
    void renderAll();
	objectHandler();
};

