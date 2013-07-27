#pragma once
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glload/gl_4_3.hpp>

#include <VertexBuffer.h>
#include <VertexAttributeArray.h>
#include <Shader.h>
#include "ProgramWithGLM.hpp"

#include <string>
#include <vector>
#include <memory>

class Model {
    std::unique_ptr<const aiScene, decltype(&aiReleaseImport)> scene;
    //std::unique_ptr<const aiScene, void (*)(const aiScene*)> scene; works
    gldr::VertexBuffer<> vbo, texVbo, normVbo;
    gldr::VertexAttributeArray vao;
    std::shared_ptr<ProgramGLM> shader;
    unsigned vertexCount;

    template<typename Functor>
    void recursiveIterate (Functor&& f, const aiNode* node, const aiScene* scene) {
        f(node, scene);
        for (int n = 0; n < node->mNumChildren; ++n) {
            recursiveIterate(std::forward<Functor>(f), node->mChildren[n], scene);
        }
    }

    void rebuildData () {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;

        auto addVertex = [&vertices, &normals, &texCoords](const aiNode* node, const aiScene* scene) {
            aiMatrix4x4 m = node->mTransformation;

            // update transform
            //aiTransposeMatrix4(&m);
            //gl::ushMatrix();
            //gl::ultMatrixf((float*)&m);

            // draw all meshes assigned to this node
            for (unsigned n = 0; n < node->mNumMeshes; ++n) {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];

                //apply_material(scene->mMaterials[mesh->mMaterialIndex]);

                /*if(mesh->mNormals == NULL) {
                    gl::isable(gl::LIGHTING);
                } else {
                    gl::nable(gl::LIGHTING);
                }*/

                for (unsigned t = 0; t < mesh->mNumFaces; ++t) {
                    const aiFace* face = &mesh->mFaces[t];
                    GLenum face_mode;

                    if (face->mNumIndices != 3)
                        throw std::runtime_error("Non-triangulised mesh provided!");

                    //gl::egin(face_mode);

                    // TEMP : non indexed mode on.
                    bool haveNormals = mesh->mNormals != nullptr;
                    // texture coordinates and colors can have multiple layers
                    bool haveTexcoords = mesh->mTextureCoords[0] != nullptr;
                    //if(mesh->mColors[0] != NULL)

                    if (!haveNormals)
                        throw std::runtime_error("Model without normals provided!");
                    if (!haveTexcoords)
                        throw std::runtime_error("Model without texture coords provided!");

                    for(unsigned i = 0; i < face->mNumIndices; i++) {
                        int index = face->mIndices[i];
                        
                        if (haveNormals) {
                            normals.push_back(glm::vec3(
                                mesh->mNormals[index].x,
                                mesh->mNormals[index].y,
                                mesh->mNormals[index].z
                            ));
                        }
                        
                        if (haveTexcoords) {
                            texCoords.push_back(glm::vec2(
                                mesh->mTextureCoords[0][index].x,
                                mesh->mTextureCoords[0][index].y
                            ));
                        }

                        vertices.push_back(glm::vec3(
                            mesh->mVertices[index].x,
                            mesh->mVertices[index].y,
                            mesh->mVertices[index].z
                        ));
                    }
                }
            }
        };

        recursiveIterate(addVertex, scene->mRootNode, scene.get());

        vbo.data(vertices);
        normVbo.data(normals);
        texVbo.data(texCoords);
        vertexCount = vertices.size();
    }

public:

    Model(std::string const& path, std::shared_ptr<ProgramGLM> program)
        : scene(aiImportFile(path.c_str(),aiProcessPreset_TargetRealtime_MaxQuality), aiReleaseImport)
        , vbo(gldr::VertexBuffer<>::Usage::STATIC_DRAW)
        , vertexCount(0u)
        , shader(std::move(program))
    {
        rebuildData();
        vao.enableAttributeArray(0);
        vao.directVertexAttribOffset(vbo, 0, 3, gldr::VertexAttributeType::Float, false, 0, 0);
        vao.enableAttributeArray(1);
        vao.directVertexAttribOffset(texVbo, 1, 2, gldr::VertexAttributeType::Float, false, 0, 0);
        vao.enableAttributeArray(2);
        vao.directVertexAttribOffset(normVbo, 2, 3, gldr::VertexAttributeType::Float, false, 0, 0);
    }

    std::shared_ptr<ProgramGLM>& getShader() { return shader; }

    void draw() {
        shader->bind();
        vao.bind();
        gl::DrawArrays(gl::TRIANGLES, 0, vertexCount);
    }
};