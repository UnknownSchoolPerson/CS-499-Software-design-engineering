#pragma once
#include <vector>
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <string>
#include <map>
#include <windows.h>
#include <shobjidl.h> 

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "meshes.h"
class objectHandler
{
public: enum meshShape;
private:
    std::map<std::string, meshShape> strToMesh = {
        {"plane", Plane},
        {"torus", Torus},
        {"cylinder", Cylinder},
        {"sphere", Sphere},
        {"taperedcylinder", TaperedCylinder},
        {"cone", Cone},
        {"prism", Prism},
        {"pyramid", Pyramid},
        {"pyramid3", Pyramid3},
        {"box", Box}
    };
    std::map<GLuint, int> texRef;
    struct renderObject
    {
        const unsigned int uniqueID;
        glm::mat4 location;
        meshShape mesh;
        GLuint renderTexture;
        GLuint origTexture;
        GLuint gProgramId;
        GLuint modelLoc;
        GLuint objectColorLoc;
#pragma warning(suppress : 26495)
        renderObject(unsigned int uniqueID) : uniqueID(uniqueID) {};
    };
    Meshes meshes;
    unsigned int freeID;
    std::vector<renderObject*> objectList;
    void refTex(GLuint tex);
    void decRefTex(GLuint tex);
    void renderPlane(renderObject* item);
    void renderTorus(renderObject* item);
    void renderCylinder(renderObject* item);
    void renderSphere(renderObject* item);
    void renderTaperedCylinder(renderObject* item);
    void renderCone(renderObject* item);
    void renderPrism(renderObject* item);
    void renderPyramid(renderObject* item);
    void renderPyramid3(renderObject* item);
    void renderBox(renderObject* item);
public:
    enum meshShape
    {
        Plane,
        Torus,
        Cylinder,
        Sphere,
        TaperedCylinder,
        Cone,
        Prism,
        Pyramid,
        Pyramid3,
        Box
    };
    //std::vector<renderObject*>* getObjectList(){ return &objectList; }
    GLuint* getObjectTex(size_t index);
    size_t getObjectListSize() { return objectList.size(); }
    meshShape* getObjectType(size_t index) { return &(objectList[index]->mesh); };
    glm::mat4* getObjectMatrix(size_t index) { return &(objectList[index]->location); }
    GLuint getOrigTexture(size_t index) { return objectList[index]->origTexture; }
    unsigned int addObject(glm::mat4 location, GLuint texture, std::string objectType, GLuint gProgramId);
    void renderAll();
    void bindTexture(GLuint newTex, size_t selectedObject);
    void deleteObject(size_t selectedObject);
	objectHandler();
    ~objectHandler();
};

