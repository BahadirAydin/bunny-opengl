#version 330 core

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
    float offset = 100;
    float scale = 0.3;

    bool x = (int((fragWorldPos.x + offset) * scale) % 2) == 1;
    bool y = (int((fragWorldPos.y + offset) * scale) % 2) == 1;
    bool z = (int((fragWorldPos.z + offset) * scale) % 2) == 1;

    bool xorXY = x != y;
    if(xorXY != z) {
        fragColor = vec4(0.0039,0.121,0.023,1);
    }
    else {
        fragColor = vec4(0.898,0.909,0.784,1);
    }
}
