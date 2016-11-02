#ifndef PHOSEONFE300_H
#define PHOSEONFE300_H

#include <glm/glm.hpp>
#include <vector>

class PhoseonFE300
{
public:
    PhoseonFE300();
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> leds;
    glm::vec3 lightDir = glm::vec3(0, 0, -1);
    float Intensity;
};

#endif // PHOSEONFE300_H
