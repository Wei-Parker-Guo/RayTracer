//****************************************************
// Starter code for assignment #1.  It is provided to 
// help get you started, but you are not obligated to
// use this starter code.
//****************************************************

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

//include header file for glfw library so that we can use OpenGL
#ifdef _WIN32
/* this has to be done because my system does not have FREETYPE, delete this line if platform is otherwise
STRONGLY NOT RECOMMENDED for GLFW setup */
#include <windows.h>
#endif
#include "linmath.h" //include GLFW's linear math for vector manipulation
#include "fast_math.h" //include a faster version of some math ops we wrote
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>

//include assimp for model file imports
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//shaders
#include "basic_shaders.h"
#include "layered_toon_shader.h"
#include "translucent_shader.h"
#include "sketch_shader.h"

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265f // Should be used from mathlib

using namespace std;

//****************************************************
// Global Variables
// Generally speaking, global variables should be 
// avoided, but for this small assignment we'll make
// and exception.
//****************************************************

GLfloat Translation[3] = {0.0f, 0.0f, 0.0f};
bool Auto_strech = false;
int  Width_global = 960;
int  Height_global = 540;

int  SizeX_saved_global;
int  SizeY_saved_global;

int  PosX_saved_global;
int  PosY_saved_global;

//status
bool rendering = false;

//parameters
bool using_phong = true;
bool using_WARD = false;
bool using_toon = false;
bool using_translucent = false;
bool using_sketch = false;

string scene_file_dir = "default.fbx"; //the default scene file to render

const GLFWvidmode * VideoMode_global = NULL;

//****************************************************
// Simple init function
//****************************************************

void initializeRendering()
{
    glfwInit();
}

//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************

void setPixel(float x, float y, GLfloat r, GLfloat g, GLfloat b) {
    glColor3f(r, g, b);
    glVertex2f(x+0.5, y+0.5);  
    // The 0.5 is to target pixel centers
    // Note that some OpenGL implementations have created gaps in the past.
}

//another function to draw a GL line given parameters
void setLine(float x, float y, float x1, float y1, GLfloat r, GLfloat g, GLfloat b){
    glBegin(GL_LINES);
    glColor3f(r, g, b);
    glVertex2f(x + 0.5f, y + 0.5f);
    glVertex2f(x1 + 0.5f, y1 + 0.5f);
    glEnd();
}

//****************************************************
// Draw a filled Frame with loaded scene
//****************************************************

//method to load a scene
bool load_scene(const string& file) {
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(file,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    // If the import failed, report it
    if (!scene) {
        printf(importer.GetErrorString());
        return false;
    }

    //analyze and record the data we need

    //load meshes and create geometry classes from them
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
    }

    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}

//draw a render frame by looping over each pixel on screen
void drawFrame() {

    // Start drawing a list of points
    glBegin(GL_POINTS);

    //looping over the entire screen for ray tracing, assume the screen window as the camera frame
    for (int i = 0; i < Width_global; i++) {
        for (int j = 0; j < Height_global; j++) {
                vec3 c_total = {0.0f, 0.0f, 0.0f};

                setPixel(i,j,1.0f,0.0f,0.0f);
            }
    }

    glEnd();

    //finished rendering, set status ready for another render
    rendering = false;
}

//****************************************************
// Keyboard inputs
//****************************************************

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
                    
        // Hint on making up/down left/right work: the variable Translation [0] and [1].
                
        case GLFW_KEY_ESCAPE: 
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break; 
        case GLFW_KEY_Q:      
            glfwSetWindowShouldClose(window, GLFW_TRUE); 
            break;
        case GLFW_KEY_LEFT :
            Translation[0]-=1.0f;
            break;
        case GLFW_KEY_RIGHT:
            Translation[0]+=1.0f;
            break;
        case GLFW_KEY_UP   :
            Translation[1]+=1.0f;
            break;
        case GLFW_KEY_DOWN :
            Translation[1]-=1.0f;
            break;
        case GLFW_KEY_F:
            if (action) {
                Auto_strech = !Auto_strech;                 
                if (Auto_strech){
                    glfwGetWindowPos(window, &PosX_saved_global, &PosY_saved_global);
                    glfwGetWindowSize(window, &SizeX_saved_global, &SizeY_saved_global);
                    glfwSetWindowSize(window, VideoMode_global->width, VideoMode_global->height);
                    glfwSetWindowPos(window, 0, 0);
                }else{
                    glfwSetWindowSize(window, SizeX_saved_global, SizeY_saved_global);
                    glfwSetWindowPos(window, PosX_saved_global, PosY_saved_global);
                }
            }
            break;
        case GLFW_KEY_SPACE:
            if (rendering) break;
            rendering = true;
            drawFrame();
            break;

        default: 
            break;
    }
    
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************

void display( GLFWwindow* window )
{   
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"
    
    //----------------------- code to draw objects --------------------------
    glPushMatrix();
    glTranslatef (Translation[0], Translation[1], Translation[2]);
    
    glPopMatrix();
    
    glfwSwapBuffers(window);
    
}

//****************************************************
// function that is called when window is resized
//***************************************************

void size_callback(GLFWwindow* window, int width, int height)
{
    // The width and height arguments are not used
    // because they are not the size of the window 
    // in pixels.

    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the 
    // framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);
    
    glViewport(0, 0, Width_global, Height_global);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);
    
    display(window);
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************

enum token_code{
    not_specified
};

token_code get_token_code(string const& token){
    return not_specified;
}

//function to record rgb of a given cmd token
void record_token_rgb(vec3 r, const vector<string> tokens){
    r[0] = stof(tokens[1]);
    r[1] = stof(tokens[2]);
    r[2] = stof(tokens[3]);
}

void read_cmd_tokens(const vector<string> tokens){

    switch (get_token_code(tokens[0])) {
        default:
            break;
    }

}

//method to check if a file exists
bool file_exists(const char* filename) {
    ifstream infile(filename);
    return infile.good();
}

int main(int argc, char *argv[]) {

    //take user input of scene file and options file
    char buffer[32];
    printf("Enter Scene Folder (max 32 chars): ");
    scanf(" %32s", buffer);
    printf("Initialized using scene folder [%s]\n", buffer);
    //check if file exists
    if (!file_exists(buffer)) {
        printf("Can't open the scene file, using default.\n");
    }
    else scene_file_dir = buffer;

    char buffer1[32];
    printf("Enter Option File (max 32 chars): ");
    scanf(" %32s", buffer1);
    printf("Initialized using option file [%s]\n", buffer1);

    //open option file and record variables
    ifstream input_stream(buffer1);
    if(!input_stream){
        printf("Can't open the option file, using default.");
    }
    
    //read lines for values
    vector<string> text;
    string line;
    while(getline(input_stream, line)) text.push_back(line);
    input_stream.close();
    for(int i=0; i<text.size();i++){
        string cmd = text[i];
        vector<string> split_cmds;
        stringstream ss(cmd);
        string token;
        while(getline(ss, token, ' ')) split_cmds.push_back(token);
        read_cmd_tokens(split_cmds);
    }

    //load scene
    load_scene(scene_file_dir);

    //This initializes glfw
    initializeRendering();
    
    GLFWwindow* window = glfwCreateWindow( Width_global, Height_global, "RayTracer", NULL, NULL );
    if ( !window )
    {
        cerr << "Error on window creating" << endl;
        glfwTerminate();
        return -1;
    }
    
    VideoMode_global = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if ( !VideoMode_global )
    {
        cerr << "Error on getting monitor" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent( window );

    size_callback(window, 0, 0);
    
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);
                
    while( !glfwWindowShouldClose( window ) ) // main loop to draw object again and again
    {
        display( window );

        glfwPollEvents();        
    }

    return 0;
}
