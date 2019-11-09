#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    //FragColor = vec4(TexCoords.x,TexCoords.y,TexCoords.z,1);
    FragColor = texture(skybox, TexCoords);
}
