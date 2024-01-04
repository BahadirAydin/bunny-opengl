#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

GLuint gProgram[4];
int gWidth, gHeight;

GLint modelingMatrixLoc[3];
GLint viewingMatrixLoc[3];
GLint projectionMatrixLoc[3];
GLint eyePosLoc[3];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

int activeProgramIndex = 0;

struct Vertex {
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) {}
    GLfloat x, y, z;
};

struct Texture {
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) {}
    GLfloat u, v;
};

struct Normal {
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) {}
    GLfloat x, y, z;
};

struct Face {
    Face(int v[], int t[], int n[]) {
        vIndex[0] = v[0];
        vIndex[1] = v[1];
        vIndex[2] = v[2];
        tIndex[0] = t[0];
        tIndex[1] = t[1];
        tIndex[2] = t[2];
        nIndex[0] = n[0];
        nIndex[1] = n[1];
        nIndex[2] = n[2];
    }
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
    GLuint Advance;     // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

int gVertexDataSizeInBytesBoard, gNormalDataSizeInBytesBoard;
int gVertexDataSizeinBytesCheckpoint, gNormalDataSizeInBytesCheckpoint;

bool ParseObj(const string &fileName) {
    fstream myfile;

    // Open the input
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open()) {
        string curLine;

        while (getline(myfile, curLine)) {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2) {
                if (curLine[0] == 'v') {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    } else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    } else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                } else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
                    char c;
                    int vIndex[3], nIndex[3], tIndex[3];
                    str >> vIndex[0];
                    str >> c >> c; // consume "//"
                    str >> nIndex[0];
                    str >> vIndex[1];
                    str >> c >> c; // consume "//"
                    str >> nIndex[1];
                    str >> vIndex[2];
                    str >> c >> c; // consume "//"
                    str >> nIndex[2];

                    assert(vIndex[0] == nIndex[0] && vIndex[1] == nIndex[1] &&
                           vIndex[2] == nIndex[2]); // a limitation for now

                    // make indices start from 0
                    for (int c = 0; c < 3; ++c) {
                        vIndex[c] -= 1;
                        nIndex[c] -= 1;
                        tIndex[c] -= 1;
                    }

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                } else {
                    cout << "Ignoring unidentified line in obj file: "
                         << curLine << endl;
                }
            }

            // data += curLine;
            if (!myfile.eof()) {
                // data += "\n";
            }
        }

        myfile.close();
    } else {
        return false;
    }

    /*
    for (int i = 0; i < gVertices.size(); ++i)
    {
            Vector3 n;

            for (int j = 0; j < gFaces.size(); ++j)
            {
                    for (int k = 0; k < 3; ++k)
                    {
                            if (gFaces[j].vIndex[k] == i)
                            {
                                    // face j contains vertex i
                                    Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
                                                      gVertices[gFaces[j].vIndex[0]].y,
                                                      gVertices[gFaces[j].vIndex[0]].z);

                                    Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
                                                      gVertices[gFaces[j].vIndex[1]].y,
                                                      gVertices[gFaces[j].vIndex[1]].z);

                                    Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
                                                      gVertices[gFaces[j].vIndex[2]].y,
                                                      gVertices[gFaces[j].vIndex[2]].z);

                                    Vector3 ab = b - a;
                                    Vector3 ac = c - a;
                                    Vector3 normalFromThisFace =
    (ab.cross(ac)).getNormalized(); n += normalFromThisFace;
                            }

                    }
            }

            n.normalize();

            gNormals.push_back(Normal(n.x, n.y, n.z));
    }
    */

    assert(gVertices.size() == gNormals.size());

    return true;
}

bool ReadDataFromFile(const string &fileName, ///< [in]  Name of the shader file
                      string &data) ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open()) {
        string curLine;

        while (getline(myfile, curLine)) {
            data += curLine;
            if (!myfile.eof()) {
                data += "\n";
            }
        }

        myfile.close();
    } else {
        return false;
    }

    return true;
}

GLuint createVS(const char *shaderName) {
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource)) {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar *shader = (const GLchar *)shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    return vs;
}

GLuint createFS(const char *shaderName) {
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource)) {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar *shader = (const GLchar *)shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    return fs;
}

void initShaders() {
    // Create the programs

    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();
    gProgram[3] = glCreateProgram();

    // Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl");
    GLuint fs1 = createFS("frag.glsl");

    GLuint vs2 = createVS("vert2.glsl");
    GLuint fs2 = createFS("frag2.glsl");

    GLuint vs3 = createVS("vert3.glsl");
    GLuint fs3 = createFS("frag3.glsl");

    GLuint vs4 = createVS("vert_text.glsl");
    GLuint fs4 = createFS("frag_text.glsl");

    glAttachShader(gProgram[0], vs1);
    glAttachShader(gProgram[0], fs1);

    glAttachShader(gProgram[1], vs2);
    glAttachShader(gProgram[1], fs2);

    glAttachShader(gProgram[2], vs3);
    glAttachShader(gProgram[2], fs3);

    glAttachShader(gProgram[3], vs4);
    glAttachShader(gProgram[3], fs4);

    glBindAttribLocation(gProgram[3], 2, "vertex");  // ????

    // Link the programs

    glLinkProgram(gProgram[0]);
    GLint status;
    glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        cout << "Program link failed" << endl;
        exit(-1);
    }

    glLinkProgram(gProgram[1]);
    glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        cout << "Program link failed" << endl;
        exit(-1);
    }

    glLinkProgram(gProgram[2]);
    glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        cout << "Program link failed" << endl;
        exit(-1);
    }

    glLinkProgram(gProgram[3]);
    glGetProgramiv(gProgram[3], GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        cout << "Program link failed 3" << endl;
        exit(-1);
    }

    for (int i = 0; i < 3; ++i) {
        modelingMatrixLoc[i] =
            glGetUniformLocation(gProgram[i], "modelingMatrix");
        viewingMatrixLoc[i] =
            glGetUniformLocation(gProgram[i], "viewingMatrix");
        projectionMatrixLoc[i] =
            glGetUniformLocation(gProgram[i], "projectionMatrix");
        eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
    }
}

void initVBO() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat *normalData = new GLfloat[gNormals.size() * 3];
    GLuint *indexData = new GLuint[gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices.size(); ++i) {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;

        minX = std::min(minX, gVertices[i].x);
        maxX = std::max(maxX, gVertices[i].x);
        minY = std::min(minY, gVertices[i].y);
        maxY = std::max(maxY, gVertices[i].y);
        minZ = std::min(minZ, gVertices[i].z);
        maxZ = std::max(maxZ, gVertices[i].z);
    }

    for (int i = 0; i < gNormals.size(); ++i) {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    for (int i = 0; i < gFaces.size(); ++i) {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }

    glBufferData(GL_ARRAY_BUFFER,
                 gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes,
                    gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData,
                 GL_STATIC_DRAW);

    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(gVertexDataSizeInBytes));
}

GLuint gTextVBO;
void initFonts(int windowWidth, int windowHeight)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[3]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    for (GLubyte c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint gVertexAttribBufferBoard, gIndexBufferBoard;
vector<Vertex> gVerticesBoard;
vector<Normal> gNormalsBoard;
vector<Face> gFacesBoard;
vector<Texture> gTexturesBoard;

void create_board() {
    gVerticesBoard.push_back(Vertex(-1.0f, -1.0f, 0.0f));
    gVerticesBoard.push_back(Vertex(1.0f, -1.0f, 0.0f));
    gVerticesBoard.push_back(Vertex(1.0f, 1.0f, 0.0f));
    gVerticesBoard.push_back(Vertex(-1.0f, 1.0f, 0.0f));

    gNormalsBoard.push_back(Normal(0.0f, 0.0f, 1.0f));
    gNormalsBoard.push_back(Normal(0.0f, 0.0f, 1.0f));
    gNormalsBoard.push_back(Normal(0.0f, 0.0f, 1.0f));
    gNormalsBoard.push_back(Normal(0.0f, 0.0f, 1.0f));

    gTexturesBoard.push_back(Texture(0.0f, 0.0f));
    gTexturesBoard.push_back(Texture(1.0f, 0.0f));
    gTexturesBoard.push_back(Texture(1.0f, 1.0f));
    gTexturesBoard.push_back(Texture(0.0f, 1.0f));

    gFacesBoard.push_back(
        Face(new int[3]{0, 1, 2}, new int[3]{0, 1, 2}, new int[3]{0, 1, 2}));
    gFacesBoard.push_back(
        Face(new int[3]{0, 2, 3}, new int[3]{0, 2, 3}, new int[3]{0, 2, 3}));
}

void initVBOBoard() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBufferBoard);
    glGenBuffers(1, &gIndexBufferBoard);

    assert(gVertexAttribBufferBoard > 0 && gIndexBufferBoard > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBufferBoard);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferBoard);
    create_board();

    gVertexDataSizeInBytesBoard = gVerticesBoard.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytesBoard = gNormalsBoard.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFacesBoard.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVerticesBoard.size() * 3];
    GLfloat *normalData = new GLfloat[gNormalsBoard.size() * 3];
    GLuint *indexData = new GLuint[gFacesBoard.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVerticesBoard.size(); i++) {
        vertexData[3 * i] = gVerticesBoard[i].x;
        vertexData[3 * i + 1] = gVerticesBoard[i].y;
        vertexData[3 * i + 2] = gVerticesBoard[i].z;

        minX = std::min(minX, gVerticesBoard[i].x);
        maxX = std::max(maxX, gVerticesBoard[i].x);
        minY = std::min(minY, gVerticesBoard[i].y);
        maxY = std::max(maxY, gVerticesBoard[i].y);
        minZ = std::min(minZ, gVerticesBoard[i].z);
        maxZ = std::max(maxZ, gVerticesBoard[i].z);
    }

    for (int i = 0; i < gNormalsBoard.size(); i++) {
        normalData[3 * i] = gNormalsBoard[i].x;
        normalData[3 * i + 1] = gNormalsBoard[i].y;
        normalData[3 * i + 2] = gNormalsBoard[i].z;
    }
    for (int i = 0; i < gFacesBoard.size(); i++) {
        indexData[3 * i] = gFacesBoard[i].vIndex[0];
        indexData[3 * i + 1] = gFacesBoard[i].vIndex[1];
        indexData[3 * i + 2] = gFacesBoard[i].vIndex[2];
    }
    glBufferData(GL_ARRAY_BUFFER,
                 gVertexDataSizeInBytesBoard + gNormalDataSizeInBytesBoard, 0,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytesBoard,
                    vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytesBoard,
                    gNormalDataSizeInBytesBoard, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData,
                 GL_STATIC_DRAW);

    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(gVertexDataSizeInBytes));
}

GLuint gVertexAttribBufferCheckpoint, gIndexBufferCheckpoint;
vector<Vertex> gVerticesCheckpoint;
vector<Normal> gNormalsCheckpoint;
vector<Face> gFacesCheckpoint;
vector<Texture> gTexturesCheckpoint;

void create_checkpoint() {
    // Vertices of the 3D rectangular object
    gVerticesCheckpoint.push_back(Vertex(-0.5f, -0.5f, -0.5f)); // Vertex 0
    gVerticesCheckpoint.push_back(Vertex(0.5f, -0.5f, -0.5f));  // Vertex 1
    gVerticesCheckpoint.push_back(Vertex(0.5f, 0.5f, -0.5f));   // Vertex 2
    gVerticesCheckpoint.push_back(Vertex(-0.5f, 0.5f, -0.5f));  // Vertex 3
    gVerticesCheckpoint.push_back(Vertex(-0.5f, -0.5f, 0.5f));  // Vertex 4
    gVerticesCheckpoint.push_back(Vertex(0.5f, -0.5f, 0.5f));   // Vertex 5
    gVerticesCheckpoint.push_back(Vertex(0.5f, 0.5f, 0.5f));    // Vertex 6
    gVerticesCheckpoint.push_back(Vertex(-0.5f, 0.5f, 0.5f));   // Vertex 7

    // Normals for each face of the rectangular object
    gNormalsCheckpoint.push_back(Normal(0.0f, 0.0f, -1.0f)); // Face 0 (front)
    gNormalsCheckpoint.push_back(Normal(0.0f, 0.0f, 1.0f));  // Face 1 (back)
    gNormalsCheckpoint.push_back(Normal(0.0f, -1.0f, 0.0f)); // Face 2 (bottom)
    gNormalsCheckpoint.push_back(Normal(0.0f, 1.0f, 0.0f));  // Face 3 (top)
    gNormalsCheckpoint.push_back(Normal(-1.0f, 0.0f, 0.0f)); // Face 4 (left)
    gNormalsCheckpoint.push_back(Normal(1.0f, 0.0f, 0.0f));  // Face 5 (right)

    // Texture coordinates for each vertex of the rectangular object
    gTexturesCheckpoint.push_back(Texture(0.0f, 0.0f));
    gTexturesCheckpoint.push_back(Texture(1.0f, 0.0f));
    gTexturesCheckpoint.push_back(Texture(1.0f, 1.0f));
    gTexturesCheckpoint.push_back(Texture(0.0f, 1.0f));
    gTexturesCheckpoint.push_back(Texture(0.0f, 0.0f));
    gTexturesCheckpoint.push_back(Texture(1.0f, 0.0f));
    gTexturesCheckpoint.push_back(Texture(1.0f, 1.0f));
    gTexturesCheckpoint.push_back(Texture(0.0f, 1.0f));

    // Faces of the rectangular object
    gFacesCheckpoint.push_back(
        Face(new int[3]{0, 1, 2}, new int[3]{0, 1, 2}, new int[3]{0, 1, 2}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{0, 2, 3}, new int[3]{0, 2, 3}, new int[3]{0, 2, 3}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{4, 5, 6}, new int[3]{4, 5, 6}, new int[3]{4, 5, 6}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{4, 6, 7}, new int[3]{4, 6, 7}, new int[3]{4, 6, 7}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{0, 4, 7}, new int[3]{0, 4, 7}, new int[3]{0, 4, 7}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{0, 7, 3}, new int[3]{0, 7, 3}, new int[3]{0, 7, 3}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{1, 5, 6}, new int[3]{1, 5, 6}, new int[3]{1, 5, 6}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{1, 6, 2}, new int[3]{1, 6, 2}, new int[3]{1, 6, 2}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{3, 2, 6}, new int[3]{3, 2, 6}, new int[3]{3, 2, 6}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{3, 6, 7}, new int[3]{3, 6, 7}, new int[3]{3, 6, 7}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{0, 1, 5}, new int[3]{0, 1, 5}, new int[3]{0, 1, 5}));
    gFacesCheckpoint.push_back(
        Face(new int[3]{0, 5, 4}, new int[3]{0, 5, 4}, new int[3]{0, 5, 4}));
}

void initVBOCheckpoint() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBufferCheckpoint);
    glGenBuffers(1, &gIndexBufferCheckpoint);

    assert(gVertexAttribBufferCheckpoint > 0 && gIndexBufferCheckpoint > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBufferCheckpoint);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferCheckpoint);
    create_checkpoint();

    gVertexDataSizeinBytesCheckpoint =
        gVerticesCheckpoint.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytesCheckpoint =
        gNormalsCheckpoint.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFacesCheckpoint.size() * 3 * sizeof(GLuint);
    GLfloat *vertexData = new GLfloat[gVerticesCheckpoint.size() * 3];
    GLfloat *normalData = new GLfloat[gNormalsCheckpoint.size() * 3];
    GLuint *indexData = new GLuint[gFacesCheckpoint.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVerticesCheckpoint.size(); i++) {
        vertexData[3 * i] = gVerticesCheckpoint[i].x;
        vertexData[3 * i + 1] = gVerticesCheckpoint[i].y;
        vertexData[3 * i + 2] = gVerticesCheckpoint[i].z;

        minX = std::min(minX, gVerticesCheckpoint[i].x);
        maxX = std::max(maxX, gVerticesCheckpoint[i].x);
        minY = std::min(minY, gVerticesCheckpoint[i].y);
        maxY = std::max(maxY, gVerticesCheckpoint[i].y);
        minZ = std::min(minZ, gVerticesCheckpoint[i].z);
        maxZ = std::max(maxZ, gVerticesCheckpoint[i].z);
    }

    for (int i = 0; i < gNormalsCheckpoint.size(); i++) {
        normalData[3 * i] = gNormalsCheckpoint[i].x;
        normalData[3 * i + 1] = gNormalsCheckpoint[i].y;
        normalData[3 * i + 2] = gNormalsCheckpoint[i].z;
    }
    for (int i = 0; i < gFacesCheckpoint.size(); i++) {
        indexData[3 * i] = gFacesCheckpoint[i].vIndex[0];
        indexData[3 * i + 1] = gFacesCheckpoint[i].vIndex[1];
        indexData[3 * i + 2] = gFacesCheckpoint[i].vIndex[2];
    }
    glBufferData(GL_ARRAY_BUFFER,
                 gVertexDataSizeinBytesCheckpoint +
                     gNormalDataSizeInBytesCheckpoint,
                 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeinBytesCheckpoint,
                    vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeinBytesCheckpoint,
                    gNormalDataSizeInBytesCheckpoint, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData,
                 GL_STATIC_DRAW);

    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init() {
    ParseObj("bunny.obj");
    // ParseObj("bunny.obj");

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(1000, 800);
    initVBO();
    initVBOBoard();
    initVBOCheckpoint();
}

glm::vec3 bunnyStart(0, -5, -10);
glm::vec3 pos(0, -5, -10);
glm::vec3 checkpointPos(0, -2, -80);

glm::vec3 bunnyPos(0, -5, -8);
float speed = 0.1;
float acceleration = 0.001;
glm::vec3 bunnyJumpDir(0, 0.2, 0);
glm::vec3 bunnyZDir(0, 0, -1);
float bunnyMaxHeight = -2.2;
bool increase = true;
vector<glm::vec3> checkpoint_dirs = {glm::vec3(0, 0, 0), glm::vec3(-6, 0, 0),
                                     glm::vec3(6, 0, 0)};

int goodCheckpointIndex = 0;

int score = 0;

const float TIME_STEP = 0.05;
const float THRESHOLD = 3;
bool isCollided(const glm::vec3 &bunnyPos, const glm::vec3 &checkpointPos) {
    for (float i = 0; i < 1; i += TIME_STEP) {
        glm::vec3 pos = bunnyPos + i * bunnyZDir * speed;
        float x = pos.x - checkpointPos.x;
        float y = pos.y - checkpointPos.y;
        float z = pos.z - checkpointPos.z;
        float dist = sqrt(x * x + y * y + z * z);
        if (dist <= THRESHOLD) {
            return true;
        }
    }
    return false;
}

void moveBunny(bool left) {
    if (left) {
        bunnyPos.x -= 1.1;
    } else {
        bunnyPos.x += 1.1;
    }
    if (bunnyPos.x > 6.5) {
        bunnyPos.x = 6.5;
    }
    if (bunnyPos.x < -6.5) {
        bunnyPos.x = -6.5;
    }
}

void update() {
    if (increase) {
        bunnyPos += bunnyJumpDir;
        if (bunnyPos.y >= bunnyMaxHeight) {
            increase = false;
        }
    } else {
        bunnyPos -= bunnyJumpDir;
        if (bunnyPos.y <= -5) {
            increase = true;
        }
    }
    bunnyPos += bunnyZDir * speed;
    eyePos = glm::vec3(0, 2, bunnyPos.z + 10);
    viewingMatrix = glm::lookAt(eyePos, glm::vec3(0, -5, bunnyPos.z - 10),
                                glm::vec3(0, 1, 0));
    if (eyePos.z <= checkpointPos.z ||
        isCollided(bunnyPos,
                   checkpointPos + checkpoint_dirs[goodCheckpointIndex])) {
        checkpointPos.z = eyePos.z - 80;
        goodCheckpointIndex = rand() % 3;
    }
}

void renderBunny() {
    activeProgramIndex = 0;
    update();

    glm::mat4 matT = glm::translate(glm::mat4(1.0), bunnyPos);
    glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(1.1, 1.1, 1.1));
    glm::mat4 matR = glm::rotate<float>(glm::mat4(1.0), (-90.0 / 180.) * M_PI,
                                        glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matR2 = glm::rotate<float>(glm::mat4(1.0), (-10 / 180.) * M_PI,
                                         glm::vec3(1.0, 0.0, 0.0));
    modelingMatrix = matT * matS * matR2 * matR;

    glUseProgram(gProgram[activeProgramIndex]);
    glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE,
                       glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE,
                       glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE,
                       glm::value_ptr(modelingMatrix));
    glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(gVertexDataSizeInBytes));
    glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void renderCheckpoint() {
    activeProgramIndex = 2;
    GLuint checkpointColorLoc =
        glGetUniformLocation(gProgram[2], "checkpointColor");
    for (int i = 0; i < 3; i++) {
        glm::mat4 matT =
            glm::translate(glm::mat4(1.0), checkpointPos + checkpoint_dirs[i]);
        glm::mat4 matS = glm::scale(glm::mat4(1.0), glm::vec3(1, 4, 0.8));
        modelingMatrix = matT * matS;

        glm::vec3 checkpointColor;
        if (i == goodCheckpointIndex) {
            checkpointColor = glm::vec3(1, 1, 0);
        } else {
            checkpointColor = glm::vec3(1, 0, 0);
        }

        glUseProgram(gProgram[activeProgramIndex]);
        glUniform3fv(checkpointColorLoc, 1, glm::value_ptr(checkpointColor));
        glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE,
                           glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE,
                           glm::value_ptr(viewingMatrix));
        glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE,
                           glm::value_ptr(modelingMatrix));
        glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

        glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBufferCheckpoint);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferCheckpoint);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(gVertexDataSizeinBytesCheckpoint));
        glDrawElements(GL_TRIANGLES, gFacesCheckpoint.size() * 3,
                       GL_UNSIGNED_INT, 0);
    }
}

glm::vec3 boardDir(0, 0, -10);
void renderBoard() {
    activeProgramIndex = 1;
    pos += boardDir;
    glm::mat4 matT = glm::translate(glm::mat4(1.0), pos);
    glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(10, 10, 100000));
    glm::mat4 matR = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI,
                                        glm::vec3(1.0, 0.0, 0.0));
    modelingMatrix = matT * matS * matR;

    glUseProgram(gProgram[activeProgramIndex]);
    glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE,
                       glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE,
                       glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc[1], 1, GL_FALSE,
                       glm::value_ptr(modelingMatrix));
    glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBufferBoard);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferBoard);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(gVertexDataSizeInBytes));
    glDrawElements(GL_TRIANGLES, gFacesBoard.size() * 3, GL_UNSIGNED_INT, 0);
}

void resetGame() {
    bunnyPos = bunnyStart;
    pos = glm::vec3(0, -5, -10);
    goodCheckpointIndex = rand() % 3;
    checkpointPos = glm::vec3(0, -2, -80);
    speed = 0.08;
    acceleration = 0.0005;
}

void display() {
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    for (int i = 0; i < 3; i++) {
        if (i != goodCheckpointIndex &&
            isCollided(bunnyPos, checkpointPos + checkpoint_dirs[i])) {
            resetGame();
            break;
        }
    }

    speed += acceleration;
    renderBoard();
    renderBunny();
    renderCheckpoint();
}

void reshape(GLFWwindow *window, int w, int h) {
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    // Use perspective projection
    float fovyRad = (float)(90.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

    // Assume default camera position and orientation (camera is at
    // (0, 0, 0) with looking at -z direction and its up vector pointing
    // at +y direction)
    //
    // viewingMatrix = glm::mat4(1);
    viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0),
                                glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1),
                                glm::vec3(0, 1, 0));
}

void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        moveBunny(true);
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        moveBunny(false);
    }
}

void mainLoop(GLFWwindow *window) {
    while (!glfwWindowShouldClose(window)) {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc,
         char **argv) // Create Main Function For Bringing It All Together
{
    GLFWwindow *window;
    if (!glfwInit()) {
        exit(-1);
    }

    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this if
    // on MacOS

    int width = 1000, height = 800;
    window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo,
           (const char *)glGetString(
               GL_RENDERER));    // Use strcpy_s on Windows, strcpy on Linux
    strcat(rendererInfo, " - "); // Use strcpy_s on Windows, strcpy on Linux
    strcat(rendererInfo,
           (const char *)glGetString(
               GL_VERSION)); // Use strcpy_s on Windows, strcpy on Linux
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
