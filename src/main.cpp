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
#include "fast_math.h" //include a faster version of some math ops we wrote
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>

//logger-related imp
#include <chrono>

//include assimp for model file imports
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//memory allocators
#include "raypool.h"

//pipelines
#include "rasterizer.h"

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
bool using_translucent = false;
bool using_sketch = false;
bool suspended = false;

string scene_file_dir = "default.fbx"; //the default scene file to render

Rasterizer rasterizer = Rasterizer(Width_global, Height_global); //a single rasterizer pipeline (TODO: enable multi-pipeline rendering for quicker rerender)

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
        printf("\n[Importing Error (ASSIMP)]\n%s\n", importer.GetErrorString());
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

//draw a rendered frame by looping over each pixel on the rasterizer
void drawFrame() {

    // Start drawing a list of points
    glBegin(GL_POINTS);

    //looping over the entire rasterizer buffer and draw pixels
    for (int i = 0; i < rasterizer.getWidth(); i++) {
        for (int j = 0; j < rasterizer.getHeight(); j++) {
            vec3 c;
            rasterizer.getColor(i, j, c);
            setPixel(i, j, c[0], c[1], c[2]);
        }
    }

    glEnd();
}

//**********************************************************
// function that does the actual drawing/rendering of stuff
//**********************************************************

void display(GLFWwindow* window)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT); //clear buffer

    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"

    //----------------------- code to draw objects --------------------------
    glPushMatrix();
    glTranslatef(Translation[0], Translation[1], Translation[2]);

    drawFrame();

    glPopMatrix();

    glfwSwapBuffers(window);

}

//render the current frame and push data to rasterizer, render the realtime progress in the window specified
void renderFrame(GLFWwindow* window) {
    if (rendering) return;
    rendering = true;

    //uncomment to run tests for the ray memory allocator
    //printf("Allocator test: Creating and allocating 10000 rays with page size of 64 rays.\n");
    //RayPool ray_pool = RayPool(64);
    //for (int i = 0; i < 10000; i++) {
    //    Ray* new_r = (Ray*)malloc(sizeof(Ray));
    //    new_r->depth = i;
    //    ray_pool.push(new_r);
    //}
    //printf("Destroying all of them.\n");
    //for (int i = 0; i < 10000; i++) {
    //    Ray* r = ray_pool.pop();
    //    printf("%d", r->depth);
    //    free(r);
    //}

    //start the logger for render time
    printf("\nRendering starts.\n");
    auto time_start = std::chrono::high_resolution_clock::now();

    //resize rasterizer if necessary
    rasterizer.resize(Width_global, Height_global);
    //draw
    for (int i = 0; i < rasterizer.getWidth(); i++) {
        for (int j = 0; j < rasterizer.getHeight(); j++) {
            //still poll events for controls during rendering
            glfwPollEvents();
            while (suspended) glfwPollEvents();

            //draw to rasterizer if not suspended

            //a new color sequence for each pixel
            colorseq cs;


            //render a test graph
            //test case of rendering alternate pixels
            color c;
            if (((i % 100 < 50) && j % 100 < 50) || (!(i % 100 < 50) && j % 100 > 50)) c = { 48 / 255.0f, 49 / 255.0f, 54 / 255.0f };
            else c = { 32 / 255.0f,  33 / 255.0f,  38 / 255.0f };
            cs.push_back(c);
            rasterizer.setColor(i, j, cs);

            //display the render result by block progressively
            if (i % 100 == 0 && j % 100 == 0) display(window);
        }
    }

    //display final results
    display(window);

    //end logger and output logs
    auto time_stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start);
    printf("Rendering finished with total duration of %f seconds.\n", duration.count() / 1000.0f);

    rendering = false;
}

//****************************************************
// Keyboard inputs
//****************************************************

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
                                    
        case GLFW_KEY_ESCAPE: 
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            if (rendering && suspended) exit(0); //quit directly if suspended
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
            if (action == GLFW_RELEASE) {
                //if rendering suspend/continue renderer
                if (rendering) {
                    suspended = !suspended;
                    if (suspended) printf("Rendering suspended by user.\n");
                    else printf("Rendering resumed by user.\n");
                }
                else renderFrame(window);
            }
            break;

        default: 
            break;
    }
    
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

enum class token_code{
    not_specified,
};

token_code get_token_code(string const& token){
    return token_code::not_specified;
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
        printf("Can't open the option file, using default.\n");
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
        //display( window );

        glfwPollEvents();        
    }

    return 0;
}
