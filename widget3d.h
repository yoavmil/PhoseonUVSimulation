#ifndef WIDGET3D_H
#define WIDGET3D_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "openglwindow.h"
#include "phoseonfe300.h"

class QTimer;
using glm::vec3;
using std::vector;

class Widget3D : public OpenGLWindow
{
    Q_OBJECT
public:
    explicit Widget3D();
    ~Widget3D();

    enum UVType {
        DualPhoseon, TripplePhoseon
    };

    UVType uvType = DualPhoseon;

    void setUvType(const UVType &value);

    float UVHeight() { return uvCenter.z; }
    float UVOffset() { return uvCenter.x; }
    float UVDegrees() { return uvDegrees; }
    float ShieldRadius() { return shieldRadius; }
    float ShieldHeight() { return shieldHeight; }

    std::string getSurfaceVert() const;
    void setSurfaceVert(const std::string &value);

    std::string getSurfaceFrag() const;
    void setSurfaceFrag(const std::string &value);

    void SetPerspective(bool p);

    const vector<vector<float>>& GetMap() { return map; }

    float IntensityFromColor(GLubyte color);

public slots:
    void setUVHeight(float z);
    void setUVOffset(float x);
    void setUVDegrees(float deg);
    void setShieldRadius(float rad);
    void setShieldHeight(float z);    
    void compileSurfaceProgram(std::string vert, std::string frag);
    void compilelightSourceProgram(std::string vert, std::string frag);

protected slots:
    void timerEllapesed();

protected:
    //gl stuff
    GLuint surfaceProgramID = 0;
    GLuint surfacePosBuffer = 0;
    std::string surfaceVert, surfaceFrag;
    GLuint lightSourceProgramID = 0;
    GLuint lightSourcePosBuffer = 0;
    std::string lightSourceVert, lightSourceFrag;
    QTimer* timer;
    float viewingDegree = 0.0;
    bool perspective = true;
    float colorFactor = 1.0f;
    bool colorIntesitySetupNeeded = true;
    bool checkShaderOrProgram(GLuint id);
    void getSurfaceDataForGLBuffers(vector<vec3>& positions);
    void getLightsDataForGLBuffers(vector<vec3>& pos, vector<vec3>& dir);
    bool createProgram(GLuint& progID, const char* vert, const char* frag);
    void paintSurface();
    void paintLightSource();
    void getMatrix(glm::mat4& proj, glm::mat4& viewmat);
    void setupColorIntesity();

    //UV params
    PhoseonFE300 phoseon;
    vec3 uvCenter = vec3(40, 0, 20);
    vec3 uvDir = glm::normalize(vec3(0, 0, 0) - uvCenter);
    struct LightData {
        vec3 pos, dir;
    };
    vector<LightData> lights;
    float shieldHeight = 3.5;
    float shieldRadius = 10;
    float uvDegrees;

    void generateLightSourcePoints();

private:
    float getMaxPixel();
    void calcMap();
    vector<vector<float>> map;
    // OpenGLWindow interface
public:
    virtual void render() override;
    virtual void initialize() override;

    // QWindow interface
protected:
    virtual void resizeEvent(QResizeEvent *) override;
};

#endif // WIDGET3D_H
