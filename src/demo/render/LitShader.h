#pragma once

// One sun shining down at an angle, a soft sky light so the far side isn't black, a small
// shine, and fog that fades distant things into the sky. Runs on the GPU for every pixel.
static const char* LitVertexShader = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
void main()
{
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragNormal   = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    gl_Position  = mvp * vec4(vertexPosition, 1.0);
}
)";

static const char* LitFragmentShader = R"(
#version 330
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform int  isFloor;                  // 1 while drawing the floor: a 1 m checkerboard
uniform float washOut;                 // 0 = normal, towards 1 = faded to near white (sleeping bodies)
out vec4 finalColor;

const vec3 Sky = vec3(226.0, 231.0, 238.0) / 255.0;

void main()
{
    vec3 base = texture(texture0, fragTexCoord).rgb * colDiffuse.rgb;
    if (isFloor == 1)
    {
        float tile = mod(floor(fragPosition.x) + floor(fragPosition.z), 2.0);
        base = mix(vec3(0.86, 0.88, 0.91), vec3(0.77, 0.79, 0.83), tile);
    }

    vec3 n       = normalize(fragNormal);
    vec3 toLight = normalize(vec3(0.4, 1.0, 0.3));
    vec3 toEye   = normalize(viewPos - fragPosition);

    float sky   = mix(0.35, 0.55, n.y * 0.5 + 0.5);                  // brighter on top faces
    float sun   = max(dot(n, toLight), 0.0) * 0.65;
    float shine = pow(max(dot(reflect(-toLight, n), toEye), 0.0), 32.0) * 0.18;
    vec3  color = base * (sky + sun) + vec3(shine);
    color = mix(color, vec3(0.95, 0.96, 0.98), washOut);

    float fog = smoothstep(40.0, 110.0, length(viewPos - fragPosition));
    finalColor = vec4(mix(color, Sky, fog), colDiffuse.a);
}
)";
