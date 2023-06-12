#include "objectHandler.h"
#include <vector>
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <iostream>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "meshes.h"
#include <string>
using namespace std;
objectHandler::objectHandler() {
    freeID = 0;
    meshes.CreateMeshes();
};

void objectHandler::renderPlane(objectHandler::renderObject item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPlaneMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, item.texture);

    glm::mat4 model = item.location;
    glUniformMatrix4fv(item.modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(item.gProgramId, item.objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

unsigned int objectHandler::addObject(glm::mat4 location, GLuint texture, string objectType, GLuint gProgramId) {
    renderObject item(freeID++);
    item.location = location;
    item.texture = texture;
    if (objectType == "plane")
        item.mesh = 0;
    item.modelLoc = glGetUniformLocation(gProgramId, "model");
    //item.viewLoc = glGetUniformLocation(gProgramId, "view");
    //item.projLoc = glGetUniformLocation(gProgramId, "projection");
    //item.objectColorLoc = glGetUniformLocation(gProgramId, "uObjectColor");
    objectList.push_back(item);
    return item.uniqueID;
}
void objectHandler::renderAll() {
    for (auto& item : objectList) {
        switch (item.mesh)
        {
            case 0:
                renderPlane(item);
                break;
            default:
                break;
        }
    }
}
