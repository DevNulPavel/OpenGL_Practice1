#include "ObjLoader.h"
#include <cstdlib>
#include <glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace glm;


void loadObjModel(const string& modelPath, std::vector<Vertex>& resultVertexes){
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(modelPath.c_str(), aiProcess_ValidateDataStructure |
                                                                aiProcess_Triangulate |
                                                                aiProcess_OptimizeMeshes);

    // подсчет количества вершин и индексов
    uint vertsCount = 0;
    uint indexesCount = 0;
    for (int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        vertsCount += mesh->mNumVertices;
        indexesCount += mesh->mNumFaces*3;
    }

    // Временные буфферы для хранения
    // буффер вершин
    vector<vec3> vertices(vertsCount);
    // буффер нормалей
    vector<vec3> normals(vertsCount);
    // буффер текстурных координат
    vector<vec2> uvs(vertsCount);
    // буффер индексов
    uint indexes[indexesCount];

    // копируем во временные буфферы буфферы
    uint vertsOffset = 0;
    uint facesOffset = 0;
    for (uint i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];

        // вершины
        for (uint j = 0; j < mesh->mNumVertices; j++) {
            aiVector3D aiPos = mesh->HasPositions() ? mesh->mVertices[j] : aiVector3D(0.0f, 0.0f, 0.0f);
            const aiVector3D& aiNormal = mesh->mNormals[j];
            aiVector3D aiUVCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : aiVector3D(0.0f, 0.0f, 0.0f);

            // копия в буффер вершин
            vec3 vertex(aiPos.x, aiPos.y, aiPos.z);
            vertices[vertsOffset + j] = vertex;
            // копия в буффер нормалей
            vec3 normal(aiNormal.x, aiNormal.y, aiNormal.z);
            normals[vertsOffset + j] = normal;
            // копия в буффер текстурных координат
            vec2 uv(aiUVCoord.x, aiUVCoord.y);
            uvs[vertsOffset + j] = uv;
        }
        // индексы
        for (int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            assert(face.mNumIndices == 3);

            uint index1 = face.mIndices[0];
            indexes[facesOffset + j*3 + 0] = index1;
            uint index2 = face.mIndices[1];
            indexes[facesOffset + j*3 + 1] = index2;
            uint index3 = face.mIndices[2];
            indexes[facesOffset + j*3 + 2] = index3;
        }
        vertsOffset += mesh->mNumVertices;
        facesOffset += mesh->mNumFaces;
    }

    // Резервируем память
    resultVertexes.resize(indexesCount);

    // Вбиваем
    for(size_t i = 0; i < indexesCount; ++i){
        // индекс в массиве
        uint indexValue = indexes[i];

        // значения по этим индексам
        const vec3& vertePos = vertices[indexValue];
        const vec3& normalValue = normals[indexValue];
        const vec2& uv = uvs[indexValue];

        // сохраняем
        Vertex vertex(vertePos, normalValue, uv);
        resultVertexes[i] = vertex;
    }
}
