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

void objectHandler::renderTorus(objectHandler::renderObject item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gTorusMesh.vao);

    glm::mat4 model = item.location;
    glUniformMatrix4fv(item.modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, item.texture);

    //glProgramUniform4f(item.gProgramId, objectColorLoc, 0.0f, 0.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderCylinder(objectHandler::renderObject item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gCylinderMesh.vao);

    glm::mat4 model = item.location;
    glUniformMatrix4fv(item.modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //https://www.tug.org/pracjourn/2007-4/walden/color.pdf
    //glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 1.0f, 0.9f, 1.0f);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, item.texture);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

unsigned int objectHandler::addObject(glm::mat4 location, GLuint texture, string objectType, GLuint gProgramId) {
    renderObject item(freeID++);
    item.location = location;
    item.texture = texture;
    if (objectType == "plane")
        item.mesh = 0;
    else if (objectType == "torus")
        item.mesh = 1;
    else if (objectType == "cylinder")
        item.mesh = 2;
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
            case 1:
                renderTorus(item);
                break;
            case 2:
                renderCylinder(item);
                break;
            default:
                break;
        }
    }
}
objectHandler::~objectHandler() {
    meshes.DestroyMeshes();
    for (auto& item : objectList) {
        glGenTextures(1, &item.texture);
    }
}
