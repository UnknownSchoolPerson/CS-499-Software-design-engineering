#include "objectHandler.h"
#include <vector>
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <iostream>
#include <windows.h>
#include <shobjidl.h> 

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "meshes.h"
#include <string>
//#include <glm/gtx/string_cast.hpp> // https://gist.github.com/donaldmunro/38841d72c65a1c32f2bf83a4a00a2c9a
using namespace std;
objectHandler::objectHandler() {
    freeID = 0;
    meshes.CreateMeshes();
};

void objectHandler::renderPlane(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPlaneMesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(item.gProgramId, item.objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderTorus(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gTorusMesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(item.gProgramId, objectColorLoc, 0.0f, 0.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderCylinder(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gCylinderMesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //https://www.tug.org/pracjourn/2007-4/walden/color.pdf
    //glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 1.0f, 0.9f, 1.0f);


    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderSphere(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gSphereMesh.vao);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderTaperedCylinder(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 1.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    //// Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderCone(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gConeMesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 1.0f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_STRIP, 36, 108);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderPrism(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPrismMesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.0f, 0.5f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPrismMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderPyramid(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPyramid4Mesh.vao);
    //// Model matrix: transformations are applied right-to-left order
    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.0f, 0.5f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderPyramid3(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPyramid3Mesh.vao);

    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.5f, 0.5f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid3Mesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void objectHandler::renderBox(objectHandler::renderObject* item) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);
    GLint objectColorLoc = glGetUniformLocation(item->gProgramId, "objectColor");
    glm::mat4 model = item->location;
    glUniformMatrix4fv(item->modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(item.gProgramId, objectColorLoc, 0.5f, 0.5f, 0.0f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    //// Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

unsigned int objectHandler::addObject(glm::mat4 location, GLuint texture, string objectType, GLuint gProgramId) {
    // Create new object and bind everything
    renderObject* item = new renderObject(freeID++);
    item->location = location;
    item->renderTexture = texture;
    item->origTexture = texture;
    refTex(texture);
    // lowercase texture name to find
    for (int i = 0; i < objectType.length(); i++)
        objectType[i] = tolower(objectType[i]);
    if (!strToMesh.count(objectType))
        throw invalid_argument("Invalid objectType");
    // Assign object mesh
    item->mesh = strToMesh[objectType];
    // Create model matrix
    item->modelLoc = glGetUniformLocation(gProgramId, "model");
    //item.viewLoc = glGetUniformLocation(gProgramId, "view");
    //item.projLoc = glGetUniformLocation(gProgramId, "projection");
    //item.objectColorLoc = glGetUniformLocation(gProgramId, "uObjectColor");
    // Add to list
    objectList.push_back(item);
    return item->uniqueID;
}

GLuint* objectHandler::getObjectTex(size_t index)
{
    return &(objectList[index]->renderTexture);
}

void objectHandler::renderAll() {
    for (auto& item : objectList) {
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, item->renderTexture);
        switch (item->mesh)
        {
            case Plane:
                renderPlane(item);
                break;
            case Torus:
                renderTorus(item);
                break;
            case Cylinder:
                renderCylinder(item);
                break;
            case Sphere:
                renderSphere(item);
                break;
            case TaperedCylinder:
                renderTaperedCylinder(item);
                break;
            case Cone:
                renderCone(item);
                break;
            case Prism:
                renderPrism(item);
                break;
            case Pyramid:
                renderPyramid(item);
                break;
            case Pyramid3:
                renderPyramid3(item);
                break;
            case Box:
                renderBox(item);
                break;
            default:
                break;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void objectHandler::bindTexture(GLuint newTex, size_t selectedObject) {
    // Reference new texture
    refTex(newTex);
    // Remove old texture
    decRefTex(objectList[selectedObject]->origTexture);
    // Assign new texture
    objectList[selectedObject]->origTexture = newTex;
    objectList[selectedObject]->renderTexture = newTex;
    return;
}

void objectHandler::refTex(GLuint tex) {
    // find the texture in the vector
    // If it is there add to it
    // If it isn't create it
    auto it = texRef.find(tex);
    if (it != texRef.end())
    {
        texRef[tex]++;
    }
    else {
        texRef[tex] = 1;
    }
}

void objectHandler::decRefTex(GLuint tex) {
    // Remove reference from texture
    texRef[tex]--;
    // If nothing refers to it, free it
    if (texRef[tex] == 0)
    {
        cout << "texture# " << tex << " was deleted." << endl;
        glGenTextures(1, &tex);
    }
}

void objectHandler::deleteObject(size_t selectedObject) {
    // Get the object at index
    auto item = objectList.at(selectedObject);
    // Remove it and it's texture
    objectList.erase(objectList.begin() + selectedObject);
    decRefTex(item->origTexture);
}

objectHandler::~objectHandler() {
    meshes.DestroyMeshes();
    for (auto& item : objectList) {
        glGenTextures(1, &item->origTexture);
    }
    //cout << "called" << endl;
}
