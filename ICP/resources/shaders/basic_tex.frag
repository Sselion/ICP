#version 330 core

in vec2 texcoord;
uniform vec4 u_diffuse_color;
uniform vec4 ColorChange;
uniform bool isChange;
uniform sampler2D tex0; // texture unit from C++
out vec4 FragColor; // final output

void main()
{
    vec4 texColor;
    if(isChange){
        texColor =  u_diffuse_color * (ColorChange - texture(tex0,texcoord));
        texColor.a = texture(tex0,texcoord).a;
    }
    else{
        texColor =  u_diffuse_color * texture(tex0,texcoord);
    }
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}