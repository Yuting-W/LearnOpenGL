#ifndef MODEL_H
#define MODEL_H
#include<glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <texture/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/mesh.h>
#include <learnopengl/shader_s.h>

#include <string>
extern unsigned int TextureFromFile(const char* path, const std::string& directory);

class Model 
{
	public:
		Model(const char* path)
		{
			loadModel(path);
		}
		void Draw(Shader &shader)
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				meshes[i].Draw(shader);
			}
		}


	private:
		std::vector<Mesh> meshes;
		std::vector<Texture> textures_loaded;
		std::string directory;
		void loadModel(std::string path)
		{
			//import scene
			Assimp::Importer import;
			const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);  //melloc in readfile!! 
											//aiProcess_Triangulate : 如果模型不是（全部）由三角形组成，它需要将模型所有的图元形状变换为三角形
											//aiProcess_FlipUVs : 翻转y轴的纹理坐标， opengl大多数图像是反的
											//aiProcess_GenNormals: 如果模型不包含法向量的话，就为每个顶点创建法线。
											//aiProcess_SplitLargeMeshes: 将比较大的网格分割成更小的子网格，如果你的渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
											//aiProcess_OptimizeMeshes: aiProcess_OptimizeMeshes
			//check scene
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
				return;
			}
			directory = path.substr(0, path.find_last_of('/'));

			//
			processNode(scene->mRootNode, scene);
		}

		void processNode(aiNode *node, const aiScene *scene)
		{
			//process mesh 
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; // nod->meshes[i] is the index of mesh in scene->mMeshes
				meshes.push_back(processMesh(mesh, scene));
			}
			//after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				processNode(node->mChildren[i], scene);
			}

		}

		Mesh processMesh(aiMesh* mesh, const aiScene* scene)
		{
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<Texture> textures;
			//process vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				glm::vec3 vector;
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;

				// normals
				if (mesh->HasNormals())
				{
					vector.x = mesh->mNormals[i].x;
					vector.y = mesh->mNormals[i].y;
					vector.z = mesh->mNormals[i].z;
					vertex.Normal = vector;
				}
				if (mesh->mTextureCoords[0])  // assimp allows multiple texture coordinates in a mesh, we only care about the first set of texture coordinates
				{
					vector.x = mesh->mTextureCoords[0][i].x;
					vector.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = glm::vec2(vector.x, vector.y);
					//// tangent
					//vector.x = mesh->mTangents[i].x;
					//vector.y = mesh->mTangents[i].y;
					//vector.z = mesh->mTangents[i].z;
					//vertex.Tangent = vector;
					//// bitangent
					//vector.x = mesh->mBitangents[i].x;
					//vector.y = mesh->mBitangents[i].y;
					//vector.z = mesh->mBitangents[i].z;
					//vertex.Bitangent = vector;
				}
				else
				{
					vertex.TexCoords = glm::vec2(0, 0);
				}
				vertices.push_back(vertex);
			}

			//process indices
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			//process texture
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
					aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				std::vector<Texture> specularMaps = loadMaterialTextures(material,
					aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			}
			return Mesh(vertices, indices, textures);
		}

		std::vector<Texture> loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName)
		{
			std::vector<Texture> textures;
			for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
			{
				aiString str;
				material->GetTexture(type, i, &str);
				bool skip = false;
				for (unsigned int j = 0; j < textures_loaded.size(); j++)
				{
					if (std::strcmp(textures_loaded[j].path.c_str(), str.C_Str()) == 0) // == 0 equal 
					{
						textures.push_back(textures_loaded[j]);
						skip = true;
						break;
					}
				}
				if (!skip)
				{
					Texture texture;
					texture.id = TextureFromFile(str.C_Str(), directory);
					texture.type = typeName;
					texture.path = str.C_Str();
					textures.push_back(texture);
					textures_loaded.push_back(texture);
				}
				
			}
			return textures;
		}
		


			



	
};


#endif // !1


