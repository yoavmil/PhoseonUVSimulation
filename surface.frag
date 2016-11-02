#version 100
precision highp float;

uniform vec3 lights_pos[100];
uniform vec3 lights_dir[100];
uniform float lights_pwr[100];
uniform int lightCount;
uniform float shield_radius;
uniform float shield_height;
uniform float color_factor;

varying vec3 p;

float powerFromDistance(float dist)
{
    return (14.0 / (14.0 + dist)) * (14.0 / (14.0 + dist));
    //"5.0" is 100% W/cm^2
    //the formula is l1/l2 = (d2/d1)^2
    //from phoseon data sheet,
    //intesity at distance = 0 mm is 100%
    //intesity at distance = 6 mm is 50%
    //we need to find the number R, which means the theoretical light source,
    //as if the LED wasn't culimated
    //solve 100/50 = ((R+6)/(R))^2 ==> R = 14 mm
    //therefor, to find the intensity, L, at 'dist'
    // L = 100% * (R / (R+dist))^2
    //"/ 50.0" is to normalize
}

float powerFormRadian(float radian)
{
    //TODO the opening radian is ~ 2 * atan2(5,14) = 0.68604788084140
    //because the sphere opening window is 10 mm,
    //and the distance to the theoretical light source is 14
    return clamp(((0.7 - radian) / 0.1), 0.0, 1.0);
}

void main()
{
    float blue = 0.0;
    for (int i = 0; i < lightCount; i++)
    {
        vec3 delta = lights_pos[i] - p;
        vec3 positionAtShieldHeight = p + (delta * shield_height / delta.z);
        if (length(positionAtShieldHeight.xy) > shield_radius)
        {
            vec3 virtualLightPos = lights_pos[i] - lights_dir[i] * 14.0;
            vec3 virtualDelta = virtualLightPos - p;
            float dist = length(delta);
            float dotProduct = dot(normalize(-virtualDelta), lights_dir[i]);
            float radian = acos(dotProduct);
            blue += powerFromDistance(dist) * powerFormRadian(radian);
        }
    }

    gl_FragColor = vec4(0, 0, clamp(color_factor * blue, 0.0, 1.0), 1.0);
}
