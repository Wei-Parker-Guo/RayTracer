//****************************************************
// Starter code for assignment #1.  It is provided to 
// help get you started, but you are not obligated to
// use this starter code.
//****************************************************

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

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
#include <ctime>

//logger-related imp
#include <chrono>
#include <stdarg.h>

//include assimp for model file imports
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/SceneCombiner.h>

//memory allocators
#include "raypool.h"

//pipelines
#include "geometry.h"     //geometry pipeline
#include "lights.h"       //illumination pipeline
#include "rasterizer.h"   //rasterizer pipeline

//dirent for file and directories management
#include "dirent.h"

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

//define the system path divider
#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/"
#endif 

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

FILE* fp = fopen("logs.txt", "w");
string scene_file_dir = "defaultScene"; //the default scene directory to render

Rasterizer rasterizer = Rasterizer(Width_global, Height_global); //a single rasterizer pipeline (TODO: enable multi-pipeline rendering for quicker rerender)

const GLFWvidmode * VideoMode_global = NULL;

//****************************************************
// Log Stuff
//****************************************************

//function to print to a log file (logs.txt) and stdout at the same time
void logprintf(char* format, ...)
{
    va_list ap;
    va_list ap2;

    va_start(ap, format);
    va_copy(ap2, ap);

    vfprintf(fp, format, ap);
    va_end(ap);

    vprintf(format, ap2);
    va_end(ap2);
}


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

//function to retrieve all files of a certian type from the directory provided
void retrieve_files(const string& pathname, vector<string>& filenames) {
    logprintf("\n[IO Report]\n");
    DIR* dir;
    struct dirent* ent;
    dir = opendir(pathname.c_str());
    if (dir != NULL) {
        /* print all the files within directory and save the file names to paths*/
        while ((ent = readdir(dir)) != NULL) {
            char* filename = ent->d_name;
            if (ent->d_type == DT_DIR) continue;
            logprintf("Detected %s\n", filename);
            filenames.push_back(filename);
        }
        closedir(dir);
    }
    else {
        /* could not open directory */
        logprintf("Directory corrupted.\n");
    }
}

//function to load a scene
bool load_scene(vector<Mesh*>& meshes, vector<Light*> lights, const string& dir) {
    //Create an instance of the Importer class
    Assimp::Importer importer;

    //********************************
    // FILE SYSTEM STUFF GOES HERE
    //********************************

    //find all scene files
    vector<string> scene_paths;
    retrieve_files(dir, scene_paths);

    //import all scenes in the input folder
    vector<string> scene_names;
    for (int i = 0; i < scene_paths.size(); i++) {
        string filename = "";
        filename += dir;
        filename += PATH_SEPARATOR;
        filename += string(scene_paths[i]);

        scene_names.push_back(filename);
    }

    //prompt the user to choose scenes if there are multiples [TO-DO: Option of merge, though it's not actually related to the renderer]
    string scene_name;
    if (scene_names.size() >= 2) {
        logprintf("\nMultiple scenes detected, choose the scene to render by index:\n");
        for (int i = 0; i < scene_names.size(); i++) {
            logprintf("[%d] %s\n", i, scene_names[i].c_str());
        }
        int index = -1;
        while (index < 0 || index >= scene_names.size()) scanf("%d", &index);
        logprintf("Chosen Scene %s to render.\n", scene_names[index].c_str());
        scene_name = scene_names[index];
    }
    else scene_name = scene_names[0];

    //analyze and record the data we need
    //we will have tangent, triangles, vertices optimiaztion, forced explicit uv mapping, aabb bounding box after this
    const aiScene* scene = importer.ReadFile(scene_name,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenBoundingBoxes |
        aiProcess_GenUVCoords |
        aiProcess_SortByPType);

    //If the import failed, report it
    if (!scene) {
        logprintf("\n[Importing Error (ASSIMP)]\n%s\n", importer.GetErrorString());
        return false;
    }
    else logprintf("Loaded Scene %s\n", scene_name.c_str());

    //*******************************
    // LOADING STUFF GOES BELOW
    //*******************************

    //load meshes and create geometry classes from them
    logprintf("\nDetected Meshes:\n\n");
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        //store
        Mesh* new_mesh = new Mesh(mesh);
        meshes.push_back(new_mesh);
        logprintf("%s\n", mesh->mName.C_Str()); //log the names of the meshes inside
    }

    //load lights
    logprintf("\nDetected Lights:\n\n");
    for (int i = 0; i < scene->mNumLights; i++) {
        aiLight* light = scene->mLights[i];
        //store
        Light* new_light;
        if (light->mType == aiLightSourceType::aiLightSource_DIRECTIONAL) new_light = new DirectLight(light); //directional light
        lights.push_back(new_light);
        logprintf("%s\n", light->mName.C_Str());
    }

    //load cameras
    logprintf("\nDetected Cameras:\n\n");
    for (int i = 0; i < scene->mNumCameras; i++) {
        aiCamera* cam = scene->mCameras[i];
        logprintf("%s\n", cam->mName.C_Str());
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
    //logprintf("Allocator test: Creating and allocating 10000 rays with page size of 64 rays.\n");
    //RayPool ray_pool = RayPool(64);
    //for (int i = 0; i < 10000; i++) {
    //    Ray* new_r = (Ray*)malloc(sizeof(Ray));
    //    new_r->depth = i;
    //    ray_pool.push(new_r);
    //}
    //logprintf("Destroying all of them.\n");
    //for (int i = 0; i < 10000; i++) {
    //    Ray* r = ray_pool.pop();
    //    logprintf("%d", r->depth);
    //    free(r);
    //}

    //start the logger for render time
    auto time_start = std::chrono::high_resolution_clock::now();
    time_t time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    logprintf("\n[Rendering Report]\nTime: %sRendering starts.\n", std::ctime(&time_now));

    //resize rasterizer if necessary
    rasterizer.resize(Width_global, Height_global);
    logprintf("Resolution: %d X %d\n", Width_global, Height_global);

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
    logprintf("Rendering finished with total duration of %f seconds.\n", duration.count() / 1000.0f);

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
                    if (suspended) logprintf("Rendering suspended by user.\n");
                    else logprintf("Rendering resumed by user.\n");
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

enum class token_code {
    not_specified,
    set_display_width,
    set_display_height
};

token_code get_token_code(string const& token){
    if (token == "-dispw") return token_code::set_display_width;
    if (token == "-disph") return token_code::set_display_height;
    return token_code::not_specified;
}

void read_cmd_tokens(const vector<string> tokens){

    switch (get_token_code(tokens[0])) {
        case token_code::set_display_width:
            Width_global = stoi(tokens[1]);
            break;
        case token_code::set_display_height:
            Height_global = stoi(tokens[1]);
            break;
        default:
            break;
    }

}

//method to check if a folder exists
//reference: https://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
bool folder_exists(const char* pathname) {
    struct stat info;

    if (stat(pathname, &info) != 0)
        logprintf("Cannot access %s\n", pathname);
    else if (info.st_mode & S_IFDIR) { // S_ISDIR() doesn't exist on my windows 
        return true;
    }
    else
        logprintf("%s is no directory\n", pathname);
    return false;
}

int main(int argc, char *argv[]) {

    //take user input of scene file and options file
    char buffer[32];
    logprintf("Enter Scene Folder (max 32 chars): ");
    scanf(" %32s", buffer);
    logprintf("Initialized using scene folder [%s]\n", buffer);
    //check if directory exists
    if (!folder_exists(buffer)) {
        logprintf("Can't open the scene file, using default.\n");
    }
    else scene_file_dir = buffer;

    char buffer1[32];
    logprintf("Enter Option File (max 32 chars): ");
    scanf(" %32s", buffer1);
    logprintf("Initialized using option file [%s]\n", buffer1);

    //open option file and record variables
    ifstream input_stream(buffer1);
    if(!input_stream){
        logprintf("Can't open the option file, using default.\n");
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
    vector<Mesh*> meshes;
    vector<Light*> lights;
    load_scene(meshes, lights, scene_file_dir);

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

    //close the log file
    fclose(fp);

    return 0;
}
