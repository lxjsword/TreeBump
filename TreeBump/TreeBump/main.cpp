#define WIN32_LEAN_AND_MEAN
#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h>
#include <stdio.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include "Extensions/ARB_multitexture_extension.h"
#include "Extensions/ARB_texture_cube_map_extension.h"
#include "Extensions/ARB_texture_env_combine_extension.h"
#include "Extensions/ARB_texture_env_dot3_extension.h"
#include "Image/IMAGE.h"
#include "Maths/Maths.h"
#include "Tree.h"
#include "Normalisation Cube Map.h"
#include "main.h"

// 树
Tree tree;

// Normal map
GLuint normalMap;


//Decal texture
GLuint decalTexture;

//Normalisation cube map
GLuint normalisationCubeMap;

// 世界坐标系下的光照位置
VECTOR3D worldLightPosition = VECTOR3D(20.0f, 20.0f, 20.0f);

bool drawBumps = true;
bool drawColor = true;

// 初始化
void Init(void)
{
    // 检查和启动ARB扩展
    if (!SetUpARB_multitexture() || !SetUpARB_texture_cube_map() ||
        !SetUpARB_texture_env_combine() || !SetUpARB_texture_env_dot3())
    {
        printf("Required Extension Unsupported\n");
        exit(0);
    }

    // 加载单位矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 设置着色模式
    glShadeModel(GL_SMOOTH);
    glClearColor(0.2f, 0.4f, 0.2f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // 启动深度测试
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    //glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    // 加载normal map
    IMAGE normalMapImage;
    normalMapImage.Load("t2.bmp");
    normalMapImage.ExpandPalette();

    // 创建normal map

    glGenTextures(1, &normalMap);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, normalMapImage.width, normalMapImage.height,
        0, normalMapImage.format, GL_UNSIGNED_BYTE, normalMapImage.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Load decal image
    IMAGE decalImage;
    decalImage.Load("t1.bmp");
    decalImage.ExpandPalette();

    //Convert decal image to texture
    glGenTextures(1, &decalTexture);
    glBindTexture(GL_TEXTURE_2D, decalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, decalImage.width, decalImage.height,
        0, decalImage.format, GL_UNSIGNED_BYTE, decalImage.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 创建normalisation cube map
    glGenTextures(1, &normalisationCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, normalisationCubeMap);
    GenerateNormalisationCubeMap();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// 绘制
void Display(void)
 {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0f, 0.0f, 30.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f);

    //rotate torus
    static float angle = 0.0f;
    angle += 0.1f;
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    //glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    
    // 得到逆矩阵
    MATRIX4X4 inverseModelMatrix;
    glPushMatrix();
    glLoadIdentity();
    glRotatef(-angle, 0.0f, 1.0f, 0.0f);
    glGetFloatv(GL_MODELVIEW_MATRIX, inverseModelMatrix);
    glPopMatrix();

    // 得到对象空间的光线向量
    VECTOR3D objectLightPosition = inverseModelMatrix*worldLightPosition;

    // 计算每个点到光照的向量
    for (int i = 0; i<tree.numVertices; ++i)
    {
        VECTOR3D lightVector = objectLightPosition - tree.vertices[i].position;

        //Calculate tangent space light vector
        tree.vertices[i].tangentSpaceLight.x =
            tree.vertices[i].sTangent.DotProduct(lightVector);
        tree.vertices[i].tangentSpaceLight.y =
            tree.vertices[i].tTangent.DotProduct(lightVector);
        tree.vertices[i].tangentSpaceLight.z =
            tree.vertices[i].normal.DotProduct(lightVector);
    }

    if (drawBumps)
    {
        // 绑定normal map到纹理单元0
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glEnable(GL_TEXTURE_2D);

        //Bind normalisation cube map to texture unit 1
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, normalisationCubeMap);
        glEnable(GL_TEXTURE_CUBE_MAP_ARB);
        glActiveTextureARB(GL_TEXTURE0_ARB);

        //Set vertex arrays for torus
        glVertexPointer(3, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].position);
        glEnableClientState(GL_VERTEX_ARRAY);

        //Send texture coords for normal map to unit 0
        glTexCoordPointer(2, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].s);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        //Send tangent space light vectors for normalisation to unit 1
        glClientActiveTextureARB(GL_TEXTURE1_ARB);
        glTexCoordPointer(3, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].tangentSpaceLight);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);


        //Set up texture environment to do (tex0 dot tex1)*color
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);

        glActiveTextureARB(GL_TEXTURE1_ARB);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);

        glActiveTextureARB(GL_TEXTURE0_ARB);



        //Draw torus
        glDrawElements(GL_TRIANGLES, tree.numIndices, GL_UNSIGNED_INT, tree.indices);


        //Disable textures
        glDisable(GL_TEXTURE_2D);

        glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_CUBE_MAP_ARB);
        glActiveTextureARB(GL_TEXTURE0_ARB);

        //disable vertex arrays
        glDisableClientState(GL_VERTEX_ARRAY);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glClientActiveTextureARB(GL_TEXTURE1_ARB);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);

        //Return to standard modulate texenv
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    //If we are drawing both passes, enable blending to multiply them together
    if (drawBumps && drawColor)
    {
        //Enable multiplicative blending
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        glEnable(GL_BLEND);
    }

    //Perform a second pass to color the torus
    if (drawColor)
    {
        if (!drawBumps)
        {
            glLightfv(GL_LIGHT1, GL_POSITION, VECTOR4D(objectLightPosition));
            glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
            glLightfv(GL_LIGHT1, GL_AMBIENT, black);
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
            glEnable(GL_LIGHT1);
            glEnable(GL_LIGHTING);

            glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
        }

        //Bind decal texture
        glBindTexture(GL_TEXTURE_2D, decalTexture);
        glEnable(GL_TEXTURE_2D);

        //Set vertex arrays for torus
        glVertexPointer(3, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].position);
        glEnableClientState(GL_VERTEX_ARRAY);

        glNormalPointer(GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].normal);
        glEnableClientState(GL_NORMAL_ARRAY);

        glTexCoordPointer(2, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].s);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        //Draw torus
        glDrawElements(GL_TRIANGLES, tree.numIndices, GL_UNSIGNED_INT, tree.indices);

        if (!drawBumps)
            glDisable(GL_LIGHTING);

        //Disable texture
        glDisable(GL_TEXTURE_2D);

        //disable vertex arrays
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    //Disable blending if it is enabled
    if (drawBumps && drawColor)
        glDisable(GL_BLEND);


    glFinish();
    glutSwapBuffers();
    glutPostRedisplay();
}

/*
void Init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);

    glLoadIdentity();
    gluLookAt(0.0f, 0.0f, 20.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f);

    // 加载normal map
    IMAGE normalMapImage;
    normalMapImage.Load("stelae.bmp");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  normalMapImage.ExpandPalette();

    // 创建normal map

    glGenTextures(1, &normalMap);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, normalMapImage.width, normalMapImage.height,
        0, normalMapImage.format, GL_UNSIGNED_BYTE, normalMapImage.data);
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, normalMap);

    glVertexPointer(3, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].position);
    glEnableClientState(GL_VERTEX_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, sizeof(TreeVertex), &tree.vertices[0].s);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glDrawElements(GL_TRIANGLES, tree.numIndices, GL_UNSIGNED_INT, tree.indices);

    //glBegin(GL_QUADS);
    //glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
    //glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
    //glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
    //glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);
    //glEnd();


    glFinish();
    glutSwapBuffers();
    glutPostRedisplay();
}
*/

// 窗口大小改变
void Reshape(int w, int h)
{
    if (h == 0)
        h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, 1.0f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
}

// 键盘响应
void Keyboard(unsigned char key, int x, int y)
{
    // ESC退出
    if (key == 27)
        exit(0);

    //'1' draws both passes
    if (key == '1')
    {
        drawBumps = true;
        drawColor = true;
    }

    //'2' draws only bumps
    if (key == '2')
    {
        drawBumps = true;
        drawColor = false;
    }

    //'3' draws only color
    if (key == '3')
    {
        drawBumps = false;
        drawColor = true;
    }

    // 'W'线框模式绘制
    if (key == 'W' || key == 'w')
        glPolygonMode(GL_FRONT, GL_LINE);

    // 'F'填充模式绘制
    if (key == 'F' || key == 'f')
        glPolygonMode(GL_FRONT, GL_FILL);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Tree Bumpmapping");

    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();
    return 0;
}