#pragma once

#include <Mesh.h>
#include <Model.h>
#include <texture.h>

#include <string>

class FlatMesh : public Mesh
{
public:
    FlatMesh()
    {
        this->vertices.resize(verticesRaw.size()/7);
        std::cout << "vert size is " << this->vertices.size() << "\n";
        
        //TODO(darius) UB and stuff
        for(int i = 0; i < verticesRaw.size(); i += 7)
        {
            vertices[i / 7] = {};
            vertices[i / 7].Position = glm::vec3(verticesRaw[i], verticesRaw[i+1], verticesRaw[i+2]);
            vertices[i / 7].Normal = glm::vec3(verticesRaw[i+3], verticesRaw[i+4], 0);
            vertices[i / 7].TexCoords = glm::vec2(verticesRaw[i+5], verticesRaw[i+6]);
        }

        this->indices = indicesRaw;
        this->textures = {};

        this -> setupMesh();
    } 

    void setTexture(std::string path, std::string name)
    {
        Texture texture(TextureFromFile(name.c_str(), path.c_str(), false, false), name, "texture_diffuse");
        textures.push_back(texture);
    }

    void DrawRaw(Shader& shader, size_t sdrp)
    {
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 pos = { 3,3,3 };
        glm::vec3 scale = { 1,1,1 };

        model = glm::translate(model, pos);
        model = glm::scale(model, scale);

        shader.setMat4("model", model);
        vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);//IMPORTANT
    }

    //TODO(darius) instanced rendering here
    void Draw(Shader& shader, size_t count)
    {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); 
            std::string number;
            std::string name = textures[i].get_type();
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); 
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); 
            else if (name == "texture_height")
                number = std::to_string(heightNr++); 

            glUniform1i(glGetUniformLocation(shader.getShader(), (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].get_texture());
        }

        vao.bind();

        glDrawElementsInstanced(GL_TRIANGLES, indicesRaw.size(), GL_UNSIGNED_INT, 0, count);

        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }


private:

    std::vector<float> verticesRaw = {
         0.5f,  0.5f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,  0.0f, 1.0f,  
    };

    std::vector<unsigned int> indicesRaw = {
        0, 1, 3,
        1, 2, 3
    };
}; 
