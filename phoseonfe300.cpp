#include "phoseonfe300.h"
#include <QDebug>

/*
 * from the data sheet
mm	%
0	100
2	75
4	62
6	50
8	42
10	36
12	30
14	27
16	25
18	23
20	22
 * */

#define LINES 12
#define X (77.1f)
#define Y (29.0f)
#define Z (122.0f)
#define EMITTING_X (70.0f)
#define EMITTING_Y (10.0f)
#define LED_COUNT (25)
#define WATT_PER_CM_QUBIC (5.0f)

PhoseonFE300::PhoseonFE300() :
    vertices(LINES * 2)
{
    //TODO remove LINES, and just add 'push_back()'
    size_t i = 0;
    vertices[i++] = glm::vec3(-X / 2, +Y / 2, 0);
    vertices[i++] = glm::vec3(-X / 2, -Y / 2, 0);

    vertices[i++] = glm::vec3(+X / 2, +Y / 2, 0);
    vertices[i++] = glm::vec3(+X / 2, -Y / 2, 0);

    vertices[i++] = glm::vec3(-X / 2, -Y / 2, 0);
    vertices[i++] = glm::vec3(+X / 2, -Y / 2, 0);

    vertices[i++] = glm::vec3(-X / 2, +Y / 2, 0);
    vertices[i++] = glm::vec3(+X / 2, +Y / 2, 0);

    vertices[i++] = glm::vec3(-X / 2, +Y / 2, Z);
    vertices[i++] = glm::vec3(-X / 2, -Y / 2, Z);

    vertices[i++] = glm::vec3(+X / 2, +Y / 2, Z);
    vertices[i++] = glm::vec3(+X / 2, -Y / 2, Z);

    vertices[i++] = glm::vec3(-X / 2, -Y / 2, Z);
    vertices[i++] = glm::vec3(+X / 2, -Y / 2, Z);

    vertices[i++] = glm::vec3(-X / 2, +Y / 2, Z);
    vertices[i++] = glm::vec3(+X / 2, +Y / 2, Z);

    vertices[i++] = glm::vec3(-X / 2, -Y / 2, 0);
    vertices[i++] = glm::vec3(-X / 2, -Y / 2, Z);

    vertices[i++] = glm::vec3(-X / 2, +Y / 2, 0);
    vertices[i++] = glm::vec3(-X / 2, +Y / 2, Z);

    vertices[i++] = glm::vec3(+X / 2, -Y / 2, 0);
    vertices[i++] = glm::vec3(+X / 2, -Y / 2, Z);

    vertices[i++] = glm::vec3(+X / 2, +Y / 2, 0);
    vertices[i++] = glm::vec3(+X / 2, +Y / 2, Z);

    for (i = 0; i < LED_COUNT; i++) {
        glm::vec3 p;
        p.x = (i+1) * EMITTING_X / (LED_COUNT+1) - EMITTING_X / 2;
        p.y = 0;
        p.z = 0;
        leds.push_back(p);
    }

    Intensity = WATT_PER_CM_QUBIC * (EMITTING_X / 10.0f) * (EMITTING_Y / 10.0f) / LED_COUNT;
}
