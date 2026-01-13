#include "SceneLoader.h"
#include "../scene/Scene.h"
#include "../scene/SceneObject.h"


#include <tiny_gltf.h>
#include <glad/glad.h>
#include <iostream>
#include <cassert>


GLuint LoadGLTextureFromImage(const tinygltf::Image& image)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = GL_RGBA;
    if (image.component == 3)
        format = GL_RGB;

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        image.width,
        image.height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        image.image.data()
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}
GLuint CreateBlackTexture()
{
    GLuint tex;
    unsigned char black[4] = { 0, 0, 0, 255 };

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, black);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return tex;
}



Scene SceneLoader::loadGLTF(const std::string& path)
{
    Scene scene;

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // Obsługa zarówno .gltf (ASCII) jak i .glb (binary)
    bool ret;
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    } else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }

    if (!warn.empty())
        std::cout << "[GLTF WARN] " << warn << std::endl;
    if (!err.empty())
        std::cerr << "[GLTF ERR] " << err << std::endl;

    if (!ret)
    {
        std::cerr << "Failed to load glTF\n";
        return scene;
    }

    std::cout << "[GLTF] Loaded\n";
    std::cout << "[GLTF] Meshes: " << model.meshes.size() << std::endl;
    std::cout << "[GLTF] Images: " << model.images.size() << std::endl;
    std::cout << "[GLTF] Materials: " << model.materials.size() << std::endl;
    
    // Oblicz bounding box z accessorów
    float minX = 1e9, minY = 1e9, minZ = 1e9;
    float maxX = -1e9, maxY = -1e9, maxZ = -1e9;
    for (const auto& mesh : model.meshes) {
        for (const auto& prim : mesh.primitives) {
            if (prim.attributes.count("POSITION")) {
                const auto& acc = model.accessors[prim.attributes.at("POSITION")];
                if (acc.minValues.size() >= 3 && acc.maxValues.size() >= 3) {
                    minX = std::min(minX, (float)acc.minValues[0]);
                    minY = std::min(minY, (float)acc.minValues[1]);
                    minZ = std::min(minZ, (float)acc.minValues[2]);
                    maxX = std::max(maxX, (float)acc.maxValues[0]);
                    maxY = std::max(maxY, (float)acc.maxValues[1]);
                    maxZ = std::max(maxZ, (float)acc.maxValues[2]);
                }
            }
        }
    }
    float centerX = (minX + maxX) / 2, centerY = (minY + maxY) / 2, centerZ = (minZ + maxZ) / 2;
    float sizeX = maxX - minX, sizeY = maxY - minY, sizeZ = maxZ - minZ;
    std::cout << "[GLTF] Bounding box: (" << minX << ", " << minY << ", " << minZ << ") to (" << maxX << ", " << maxY << ", " << maxZ << ")\n";
    std::cout << "[GLTF] Center: (" << centerX << ", " << centerY << ", " << centerZ << "), Size: " << std::max({sizeX, sizeY, sizeZ}) << "\n";
    
    std::cout << "[GLTF] Processing meshes..." << std::flush;

    int meshCount = 0;
    for (const tinygltf::Mesh& mesh : model.meshes)
    {
        for (const tinygltf::Primitive& prim : mesh.primitives)
        {
            SceneObject obj;

            glGenVertexArrays(1, &obj.vao);
            glBindVertexArray(obj.vao);

            
            const auto& posAccessor =
                model.accessors[prim.attributes.at("POSITION")];
            const auto& posView =
                model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer =
                model.buffers[posView.buffer];

            glGenBuffers(1, &obj.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, obj.vbo);
            glBufferData(
                GL_ARRAY_BUFFER,
                posAccessor.count * 3 * sizeof(float),
                posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset,
                GL_STATIC_DRAW
            );

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            
            if (prim.attributes.count("NORMAL"))
            {
                const auto& nAccessor =
                    model.accessors[prim.attributes.at("NORMAL")];
                const auto& nView =
                    model.bufferViews[nAccessor.bufferView];
                const auto& nBuffer =
                    model.buffers[nView.buffer];

                glGenBuffers(1, &obj.nbo);
                glBindBuffer(GL_ARRAY_BUFFER, obj.nbo);
                glBufferData(
                    GL_ARRAY_BUFFER,
                    nAccessor.count * 3 * sizeof(float),
                    nBuffer.data.data() + nView.byteOffset + nAccessor.byteOffset,
                    GL_STATIC_DRAW
                );

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            }

           
            if (prim.attributes.count("TEXCOORD_0"))
            {
                const auto& uvAccessor =
                    model.accessors[prim.attributes.at("TEXCOORD_0")];
                const auto& uvView =
                    model.bufferViews[uvAccessor.bufferView];
                const auto& uvBuffer =
                    model.buffers[uvView.buffer];

                glGenBuffers(1, &obj.tbo);
                glBindBuffer(GL_ARRAY_BUFFER, obj.tbo);
                glBufferData(
                    GL_ARRAY_BUFFER,
                    uvAccessor.count * 2 * sizeof(float),
                    uvBuffer.data.data() + uvView.byteOffset + uvAccessor.byteOffset,
                    GL_STATIC_DRAW
                );

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
            }

         
            if (prim.indices >= 0)
            {
                const auto& iAccessor =
                    model.accessors[prim.indices];
                const auto& iView =
                    model.bufferViews[iAccessor.bufferView];
                const auto& iBuffer =
                    model.buffers[iView.buffer];

                glGenBuffers(1, &obj.ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ebo);
                glBufferData(
                    GL_ELEMENT_ARRAY_BUFFER,
                    iAccessor.count *
                        tinygltf::GetComponentSizeInBytes(iAccessor.componentType),
                    iBuffer.data.data() + iView.byteOffset + iAccessor.byteOffset,
                    GL_STATIC_DRAW
                );

                obj.indexCount = static_cast<unsigned int>(iAccessor.count);
                obj.indexType = iAccessor.componentType;
            }

            glBindVertexArray(0);


            if (prim.material >= 0)
            {
                const auto& mat = model.materials[prim.material];
                const auto& pbr = mat.pbrMetallicRoughness;

                // Albedo
                if (pbr.baseColorTexture.index >= 0)
                {
                    const auto& tex = model.textures[pbr.baseColorTexture.index];
                    const auto& img = model.images[tex.source];
                    obj.albedoTexture = LoadGLTextureFromImage(img);
                }

                // Normal
                if (mat.normalTexture.index >= 0)
                {
                    const auto& tex = model.textures[mat.normalTexture.index];
                    const auto& img = model.images[tex.source];
                    obj.normalTexture = LoadGLTextureFromImage(img);
                }

                // Metallic-Roughness
                if (pbr.metallicRoughnessTexture.index >= 0)
                {
                    const auto& tex = model.textures[pbr.metallicRoughnessTexture.index];
                    const auto& img = model.images[tex.source];
                    obj.metallicRoughnessTexture = LoadGLTextureFromImage(img);
                }

                // -------------------------
                // EMISSIVE TEXTURE
                // -------------------------
                static GLuint blackTexture = CreateBlackTexture();
                obj.emissiveTexture = blackTexture;
                obj.emissiveStrength = 0.0f;

                // emissive texture
                if (mat.emissiveTexture.index >= 0)
                {
                    const auto& tex = model.textures[mat.emissiveTexture.index];
                    const auto& img = model.images[tex.source];
                    obj.emissiveTexture = LoadGLTextureFromImage(img);
                }

                // emissive strength (KHR extension)
                if (mat.extensions.count("KHR_materials_emissive_strength"))
                {
                    const auto& ext =
                        mat.extensions.at("KHR_materials_emissive_strength");

                    obj.emissiveStrength =
                        ext.Get("emissiveStrength").Get<double>();
                }
                else if (!mat.emissiveFactor.empty())
                {
                    // fallback
                    obj.emissiveStrength = 1.0f;
                }
                // emissive color
                if (!mat.emissiveFactor.empty())
                {
                    obj.emissiveColor = glm::vec3(
                        mat.emissiveFactor[0],
                        mat.emissiveFactor[1],
                        mat.emissiveFactor[2]
                    );
                }
                else
                {
                    obj.emissiveColor = glm::vec3(1.0f);
                }


            }

            scene.objects.push_back(obj);
        }
        meshCount++;
    }

    std::cout << " done" << std::endl;
    std::cout << "[GLTF] Scene objects: "
              << scene.objects.size() << std::endl;

    return scene;
}
