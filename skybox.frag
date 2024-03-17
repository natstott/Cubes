
#version 450 core
             uniform samplerCube uTexture;
             in vec3 vTexCoord;
             out vec4 Color;

             void main()
             {
                Color = texture(uTexture, vTexCoord);//+vec4(1.0,1.0,0.0,0.0);
             }
