#include "widget3d.h"
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <string>
#include <QTimer>
#include <glm/gtc/matrix_transform.hpp>

#include "PhoseonFE300.h"

#define SURFACE_SZ 200

std::string ReadAll(const QString& path) {
    QFile f(path);
    f.open(QFile::ReadOnly);
    return f.readAll().toStdString();
}

Widget3D::Widget3D()
{
    surfaceVert = ReadAll(":/shaders/surface.vert");
    surfaceFrag = ReadAll(":/shaders/surface.frag");

    lightSourceVert = ReadAll(":/shaders/lightsource.vert");
    lightSourceFrag = ReadAll(":/shaders/lightsource.frag");

    setUVDegrees(22);
    setUVOffset(38);
    setUVHeight(22);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget3D::timerEllapesed);
    timer->start(100);
}

Widget3D::~Widget3D()
{
    //makeCurrent();
    glDeleteProgram(surfaceProgramID);
    glDeleteProgram(lightSourceProgramID);
    if (surfacePosBuffer != 0) {
        glDeleteBuffers(1, &surfacePosBuffer);
        surfacePosBuffer = 0;
    }
    if (lightSourcePosBuffer != 0) {
        glDeleteBuffers(1, &lightSourcePosBuffer);
        lightSourcePosBuffer = 0;
    }
}

void Widget3D::setUvType(const UVType &value)
{
    uvType = value;
}

void Widget3D::setUVHeight(float z)
{
    uvCenter.z = z;
    generateLightSourcePoints();
    renderLater();
}

void Widget3D::setUVOffset(float x)
{
    uvCenter.x = x;
    generateLightSourcePoints();
    renderLater();
}

void Widget3D::setUVDegrees(float deg)
{
    uvDegrees = deg;
    uvDir.x = -std::sin(glm::radians(deg));
    uvDir.z = -std::cos(glm::radians(deg));
    uvDir = glm::normalize(uvDir);
    generateLightSourcePoints();
    renderLater();
}

void Widget3D::setShieldRadius(float rad)
{
    shieldRadius = rad;
    renderLater();
}

void Widget3D::setShieldHeight(float z)
{
    shieldHeight = z;
    renderLater();
}

void Widget3D::compileSurfaceProgram(std::string vert, std::string frag)
{
    surfaceVert = vert;
    surfaceFrag = frag;
    bool ok = createProgram(surfaceProgramID, surfaceVert.data(), surfaceFrag.data());
    if (!ok) {
        qWarning() << "failed to create program" << vert.data() << frag.data();
    }
}

void Widget3D::compilelightSourceProgram(std::string vert, std::string frag)
{
    lightSourceVert = vert;
    lightSourceFrag = frag;
    bool ok = createProgram(surfaceProgramID, lightSourceVert.data(), lightSourceFrag.data());
    if (!ok) {
        qWarning() << "failed to create program" << vert.data() << frag.data();
    }
}

void Widget3D::timerEllapesed()
{
    if (perspective) {
        viewingDegree += 1;
    }
    else {
        viewingDegree = 90;
    }

    renderNow();
}

std::string Widget3D::getSurfaceFrag() const
{
    return surfaceFrag;
}

void Widget3D::setSurfaceFrag(const std::string &value)
{
    colorIntesitySetupNeeded = true;
    surfaceFrag = value;
}

void Widget3D::SetPerspective(bool p)
{
    perspective = p;
    timerEllapesed();
    if (perspective) {
        timer->start();
    }
    else {
        timer->stop();
        calcMap();
    }
}

float Widget3D::IntensityFromColor(GLubyte color)
{
    return phoseon.Intensity * color / 255.0f / 2;
}

void Widget3D::calcMap()
{
    int xFrom, xWidth, yFrom, yWidth;
    if (width() > height()) {
        xFrom = (width() - height()) / 2;
        xWidth = height();
        yFrom = 0;
        yWidth = height();
    }
    else {
        yFrom = (height() - width()) / 2;
        yWidth = width();
        xFrom = 0;
        xWidth = width();
    }

    vector<GLubyte> data(xWidth * yWidth * 4);
    glReadPixels(xFrom, yFrom, xWidth, yWidth, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    map.clear();
    float total = 0;
    size_t i = 0;
    for (size_t y = 0; y < yWidth; y++) {
        map.push_back(vector<float>());
        for (size_t x = 0; x < xWidth; x++) {
            map[y].push_back(data[2 + 4 * i++]);
            total += map[y].back();
        }
    }
    qDebug() << "total light" << total / 255.0 / xWidth / xWidth;
}

std::string Widget3D::getSurfaceVert() const
{
    return surfaceVert;
}

void Widget3D::setSurfaceVert(const std::string &value)
{
    surfaceVert = value;
}

bool Widget3D::checkShaderOrProgram(GLuint id)
{
    GLint log_length = 0;
    if (glIsShader(id)) {
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);
    } else if (glIsProgram(id)) {
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length);
    } else {
        qWarning() << "printlog: Not a shader or a program";
        return false;
    }

    if (log_length == 0)
        return true;

    char* log = (char*)malloc(log_length);

    if (glIsShader(id))
        glGetShaderInfoLog(id, log_length, NULL, log);
    else if (glIsProgram(id))
        glGetProgramInfoLog(id, log_length, NULL, log);

    qWarning() << log;
    free(log);

    return false;
}

void Widget3D::getSurfaceDataForGLBuffers(vector<vec3> &positions)
{
    vector<vector<vec3>> surfacePoints;
    surfacePoints.resize(SURFACE_SZ);
    for (auto& i : surfacePoints)
        i.resize(SURFACE_SZ);

    for (size_t r = 0; r < SURFACE_SZ; r++) {
        for (size_t c = 0; c < SURFACE_SZ; c++) {
            vec3& pos = surfacePoints[r][c];
            pos.x = SURFACE_SZ * ((float)c / SURFACE_SZ - .5f);
            pos.y = SURFACE_SZ * ((float)r / SURFACE_SZ - .5f);
            pos.z = 0;
        }
    }

    positions.clear();
    for (size_t r = 0; r < SURFACE_SZ - 1; r++) {
        for (size_t c = 0; c < SURFACE_SZ - 1; c++) {
            positions.push_back(surfacePoints[r+0][c+0]);
            positions.push_back(surfacePoints[r+0][c+1]);
            positions.push_back(surfacePoints[r+1][c+1]);

            positions.push_back(surfacePoints[r+0][c+0]);
            positions.push_back(surfacePoints[r+1][c+1]);
            positions.push_back(surfacePoints[r+1][c+0]);
        }
    }
}

void Widget3D::getLightsDataForGLBuffers(vector<vec3> &pos, vector<vec3> &dir)
{
    pos.clear();
    dir.clear();

    for (const LightData& ld : lights) {
        pos.push_back(ld.pos);
        dir.push_back(ld.dir);
    }
}

bool Widget3D::createProgram(GLuint &progID, const char *vert, const char *frag)
{
    if (progID != 0) {
        glDeleteProgram(progID);
        progID = 0;
    }
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShaderID, 1, &vert, NULL);
    glShaderSource(fragmentShaderID, 1, &frag, NULL);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);

    if (!checkShaderOrProgram(vertexShaderID))
        return false;

    if (!checkShaderOrProgram(fragmentShaderID))
        return false;

    progID = glCreateProgram();

    glAttachShader(progID, vertexShaderID);
    glAttachShader(progID, fragmentShaderID);

    glLinkProgram(progID);

    if (!checkShaderOrProgram(surfaceProgramID))
        return false;

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return true;
}

void Widget3D::paintSurface()
{
    glm::mat4 proj, viewmat;
    getMatrix(proj, viewmat);

    glUseProgram(surfaceProgramID);

    //get positions
    GLint projectionLocation = glGetUniformLocation(surfaceProgramID, "projection");
    GLint transformLocation = glGetUniformLocation(surfaceProgramID, "transform");
    GLint positionAtrribLocation = glGetAttribLocation(surfaceProgramID, "position");

    //set matrix
    glUniformMatrix4fv(projectionLocation, 1, false, &proj[0][0]);
    glUniformMatrix4fv(transformLocation, 1, false, &viewmat[0][0]);

    {//send lights data
        vector<vec3> pos, dir;
        getLightsDataForGLBuffers(pos, dir);

        GLint lightPosLocation = glGetUniformLocation(surfaceProgramID, "lights_pos");
        GLint lightDirLocation = glGetUniformLocation(surfaceProgramID, "lights_dir");
        GLint lightCountLocation = glGetUniformLocation(surfaceProgramID, "lightCount");

        GLint shieldRadiusLocation = glGetUniformLocation(surfaceProgramID, "shield_radius");
        GLint shieldHeightLocation = glGetUniformLocation(surfaceProgramID, "shield_height");
        GLint colorFactorLocation = glGetUniformLocation(surfaceProgramID, "color_factor");

        glUniform3fv(lightPosLocation, (GLsizei)pos.size(), &pos[0].x);
        glUniform3fv(lightDirLocation, (GLsizei)pos.size(), &dir[0].x);
        glUniform1i(lightCountLocation, (GLint)pos.size());

        glUniform1f(shieldRadiusLocation, shieldRadius);
        glUniform1f(shieldHeightLocation, shieldHeight);
        glUniform1f(colorFactorLocation, colorFactor);
    }

    //enable attribs
    glEnableVertexAttribArray(positionAtrribLocation);

    //send surface data
    glBindBuffer(GL_ARRAY_BUFFER, surfacePosBuffer);
    glVertexAttribPointer(
                positionAtrribLocation,  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                       // size
                GL_FLOAT,                // type
                GL_FALSE,                // normalized?
                0,                       // stride
                (void*)0                 // array buffer offset
                );

    //draw surface
    glDrawArrays(GL_TRIANGLES, 0, 3 * (SURFACE_SZ-1) * (SURFACE_SZ-1) * 2);
}

void Widget3D::paintLightSource()
{
    glUseProgram(lightSourceProgramID);
    //get positions
    GLint projectionLocation = glGetUniformLocation(lightSourceProgramID, "projection");
    GLint transformLocation = glGetUniformLocation(lightSourceProgramID, "transform");
    GLint colorLocation = glGetUniformLocation(lightSourceProgramID, "color");
    GLint positionAtrribLocation = glGetAttribLocation(lightSourceProgramID, "position");

    //set matrix
    glm::mat4 proj, viewmat, translate, modelView;
    getMatrix(proj, viewmat);
    glUniformMatrix4fv(projectionLocation, 1, false, &proj[0][0]);

    //set color
    glUniform3f(colorLocation, 1, 1, 0);

    //enable attribs
    glEnableVertexAttribArray(positionAtrribLocation);

    //send surface data
    glBindBuffer(GL_ARRAY_BUFFER, lightSourcePosBuffer);
    glVertexAttribPointer(
                positionAtrribLocation,  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                       // size
                GL_FLOAT,                // type
                GL_FALSE,                // normalized?
                0,                       // stride
                (void*)0                 // array buffer offset
                );

    //draw right light source
    if (uvType == DualPhoseon) {
        translate = glm::translate(glm::mat4(), uvCenter);
        translate = glm::rotate(translate, glm::radians(90.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        modelView = viewmat * translate;
        glUniformMatrix4fv(transformLocation, 1, false, &modelView[0][0]);

        //draw light source as lines
        glDrawArrays(GL_LINES, 0, (GLsizei)(phoseon.vertices.size()));

        //draw left light source
        translate = glm::translate(glm::mat4(), vec3(-uvCenter.x, 0.f, uvCenter.z));
        translate = glm::rotate(translate, glm::radians(-90.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        modelView = viewmat * translate;
        glUniformMatrix4fv(transformLocation, 1, false, &modelView[0][0]);

        //draw light source as lines
        glDrawArrays(GL_LINES, 0, (GLsizei)(phoseon.vertices.size()));
    }
    else {
        vec3 center = uvCenter;

        translate = glm::translate(glm::mat4(), uvCenter);
        translate = glm::rotate(translate, glm::radians(90.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        modelView = viewmat * translate;
        glUniformMatrix4fv(transformLocation, 1, false, &modelView[0][0]);

        //draw light source as lines
        glDrawArrays(GL_LINES, 0, (GLsizei)(phoseon.vertices.size()));

        //draw left light source
        center.x = uvCenter.x * std::cos(glm::radians(120.0f));
        center.y = uvCenter.x * std::sin(glm::radians(120.0f));
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(210.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        modelView = viewmat * translate;
        glUniformMatrix4fv(transformLocation, 1, false, &modelView[0][0]);

        //draw light source as lines
        glDrawArrays(GL_LINES, 0, (GLsizei)(phoseon.vertices.size()));

        center.x = uvCenter.x * std::cos(glm::radians(-120.0f));
        center.y = uvCenter.x * std::sin(glm::radians(-120.0f));
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(-30.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        modelView = viewmat * translate;
        glUniformMatrix4fv(transformLocation, 1, false, &modelView[0][0]);

        //draw light source as lines
        glDrawArrays(GL_LINES, 0, (GLsizei)(phoseon.vertices.size()));
    }
}

void Widget3D::getMatrix(glm::mat4 &proj, glm::mat4 &viewmat)
{
    if (perspective) {
        proj = glm::perspective(glm::radians(60.0f), (float)width() / (float) height(), 0.1f, 5000.f);
        float viewRadius = 100;
        glm::vec2 viewPoint = viewRadius * glm::vec2(
                    std::cos(glm::radians(viewingDegree)),
                    std::sin(glm::radians(viewingDegree)));
        viewmat = glm::lookAt(vec3(viewPoint.x, viewPoint.y, 300.0f), vec3(0, 0, 0), vec3(0, 0, 1));
    }
    else {
        float widgetRatio = (float)width() / (float)height();
        proj = glm::ortho(-widgetRatio * 100.0f, widgetRatio * 100.0f, -100.0f, 100.0f, 0.1f, 10.f);
        viewmat = glm::lookAt(vec3(0, 0, 1.0f), vec3(0, 0, 0), vec3(0, 1.0, 0));
    }
}

void Widget3D::setupColorIntesity()
{
    //save state
    bool _perspective = perspective;
    vec3 _uvCenter = uvCenter;
    vec3 _uvDir = uvDir;
    float _uvDegrees = uvDegrees;
    float _shieldRadius = shieldRadius;

    perspective = false;
    uvCenter.x = 0;
    uvCenter.y = 0;
    uvCenter.z = 0;
    uvDir = glm::vec3(0, 0, 0);
    uvDegrees = 0;
    shieldRadius = 0;
    generateLightSourcePoints();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintSurface();
    //normalize loop
    bool again = true;
    while (again) {
        float max = getMaxPixel();
        if (max == 0) {
            colorIntesitySetupNeeded = true;
            break;
        }
        if (max >= 1) {
            colorFactor /= 2.0f;
            again = true;
        }
        else if (max > 0) {
            colorFactor /= max;
            again = false;
        }
        else {
            colorIntesitySetupNeeded = false;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        paintSurface();
        max = getMaxPixel();
    };

    perspective = _perspective;
    uvCenter = _uvCenter;
    uvDir = _uvDir;
    uvDegrees = _uvDegrees;
    shieldRadius = _shieldRadius;
    generateLightSourcePoints();
}

void Widget3D::generateLightSourcePoints()
{
    lights.clear();

    if (uvType == DualPhoseon) {
        glm::mat4 translate, directionMat;
        glm::vec3 center = uvCenter;
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(90.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        directionMat = glm::transpose(glm::inverse(translate)); //this is called the normal matrix
        for (const vec3& l : phoseon.leds) {
            glm::vec4 translatedPosition = translate * glm::vec4(l, 1.0f);
            glm::vec4 dir = directionMat * glm::vec4(phoseon.lightDir, 0.0f);
            LightData ld;
            ld.dir = vec3(dir.x, dir.y, dir.z);
            ld.pos = vec3(translatedPosition.x, translatedPosition.y, translatedPosition.z);
            lights.push_back(ld);
        }

        //the other side
        center.x = -center.x;
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(-90.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        directionMat = glm::transpose(glm::inverse(translate)); //this is called the normal matrix
        for (const vec3& l : phoseon.leds) {
            glm::vec4 translatedPosition = translate * glm::vec4(l, 1.0f);
            glm::vec4 dir = directionMat * glm::vec4(phoseon.lightDir, 0.0f);
            LightData ld;
            ld.dir = vec3(dir.x, dir.y, dir.z);
            ld.pos = vec3(translatedPosition.x, translatedPosition.y, translatedPosition.z);
            lights.push_back(ld);
        }
    }
    else {
        glm::mat4 translate, directionMat;
        glm::vec3 center = uvCenter;
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(90.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        directionMat = glm::transpose(glm::inverse(translate)); //this is called the normal matrix
        for (const vec3& l : phoseon.leds) {
            glm::vec4 translatedPosition = translate * glm::vec4(l, 1.0f);
            glm::vec4 dir = directionMat * glm::vec4(phoseon.lightDir, 0.0f);
            LightData ld;
            ld.dir = vec3(dir.x, dir.y, dir.z);
            ld.pos = vec3(translatedPosition.x, translatedPosition.y, translatedPosition.z);
            lights.push_back(ld);
        }

        //the other side
        center.x = uvCenter.x * cos(glm::radians(120.0));
        center.y = uvCenter.x * sin(glm::radians(120.0));
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(210.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        directionMat = glm::transpose(glm::inverse(translate)); //this is called the normal matrix
        for (const vec3& l : phoseon.leds) {
            glm::vec4 translatedPosition = translate * glm::vec4(l, 1.0f);
            glm::vec4 dir = directionMat * glm::vec4(phoseon.lightDir, 0.0f);
            LightData ld;
            ld.dir = vec3(dir.x, dir.y, dir.z);
            ld.pos = vec3(translatedPosition.x, translatedPosition.y, translatedPosition.z);
            lights.push_back(ld);
        }

        //the other side
        center.x = uvCenter.x * cos(glm::radians(-120.0));
        center.y = uvCenter.x * sin(glm::radians(-120.0));
        translate = glm::translate(glm::mat4(), center);
        translate = glm::rotate(translate, glm::radians(-30.0f), vec3(0, 0, 1));
        translate = glm::rotate(translate, glm::radians(uvDegrees), vec3(1, 0, 0));
        directionMat = glm::transpose(glm::inverse(translate)); //this is called the normal matrix
        for (const vec3& l : phoseon.leds) {
            glm::vec4 translatedPosition = translate * glm::vec4(l, 1.0f);
            glm::vec4 dir = directionMat * glm::vec4(phoseon.lightDir, 0.0f);
            LightData ld;
            ld.dir = vec3(dir.x, dir.y, dir.z);
            ld.pos = vec3(translatedPosition.x, translatedPosition.y, translatedPosition.z);
            lights.push_back(ld);
        }
    }
}

float Widget3D::getMaxPixel()
{
    int xFrom, xWidth, yFrom, yWidth;
    if (width() > height()) {
        xFrom = (width() - height()) / 2;
        xWidth = height();
        yFrom = 0;
        yWidth = height();
    }
    else {
        yFrom = (height() - width()) / 2;
        yWidth = width();
        xFrom = 0;
        xWidth = width();
    }

    vector<GLubyte> data(xWidth * yWidth * 4);
    glReadPixels(xFrom, yFrom, xWidth, yWidth, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    GLubyte max = 0;
    for (size_t i = 0; i < data.size(); i += 4) {
        max = std::max(max, data[i + 2]);
    }
    return max / 255.0f;
}

void Widget3D::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    if (colorIntesitySetupNeeded) {
        colorIntesitySetupNeeded = false;
        setupColorIntesity();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    paintSurface();
    if (perspective) {
        paintLightSource();
    }
}

void Widget3D::initialize()
{
    qDebug() << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    bool ok = createProgram(surfaceProgramID, surfaceVert.data(), surfaceFrag.data());
    if (!ok) {
        QApplication::quit();
    }

    ok = createProgram(lightSourceProgramID, lightSourceVert.data(), lightSourceFrag.data());
    if (!ok) {
        QApplication::quit();
    }

    generateLightSourcePoints();

    vector<vec3> posistions;
    getSurfaceDataForGLBuffers(posistions);

    if (surfacePosBuffer != 0) {
        glDeleteBuffers(1, &surfacePosBuffer);
        surfacePosBuffer = 0;
    }

    glGenBuffers(1, &surfacePosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, surfacePosBuffer);
    glBufferData(GL_ARRAY_BUFFER, posistions.size() * sizeof(vec3), posistions.data(), GL_STATIC_DRAW);

    if (lightSourcePosBuffer != 0) {
        glDeleteBuffers(1, &lightSourcePosBuffer);
        lightSourcePosBuffer = 0;
    }
    glGenBuffers(1, &lightSourcePosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, lightSourcePosBuffer);
    glBufferData(GL_ARRAY_BUFFER, phoseon.vertices.size() * sizeof(vec3), phoseon.vertices.data(), GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}


void Widget3D::resizeEvent(QResizeEvent *)
{
    colorIntesitySetupNeeded = true;
    renderLater();
}
