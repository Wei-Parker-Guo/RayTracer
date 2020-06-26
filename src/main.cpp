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
#include <thread>

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

//memory allocators & threads
#include "raypool.h"
#include "render_thread.h"

//pipelines
#include "geometry.h"     //geometry pipeline
#include "lights.h"       //illumination pipeline
#include "cameras.h"      //viewport pipeline
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

const float PI = 3.1415926;

FILE* fp = fopen("logs.txt", "w");
string scene_file_dir = "defaultScene"; //the default scene directory to render

//scene objects
vector<Mesh*> meshes;
vector<Light*> lights;
vector<Camera*> cams;

//ray parameters
float set_hfov = 54.43; //horizontal fov of the camera, default to maya standard default 54.43
int samples_per_pixel = 2; //sample monte carlo rays per pixel width, defaulted to 2, actual sample number is its square
int ray_pool_page_size = 64; //page size for the ray allocator
int thread_n = ceil(fast_sqrt(thread::hardware_concurrency())); //concurrent thread numbers when rendering the scene (set to match cpu core amounts), actual thread num is this num squared and plus some
int max_ray_bounce = 3; //maximum bounce time of a ray

Rasterizer rasterizer = Rasterizer(Width_global, Height_global); //a single rasterizer pipeline (TODO: enable multi-pipeline rendering for quicker rerender)
//rasterizer parameters
int prog_disp_span = 100; //block of pixels to progressively display

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

//function to retireve a node's global transform matrix in a scene
void retrieve_node_gtrans(aiMatrix4x4 out, const aiScene* scene, const char* node_name) {
    aiNode* root_node = scene->mRootNode;
    aiNode* this_node = root_node->FindNode(node_name);
    //traverse through hierachy to get all transform matrices
    vector<aiMatrix4x4> trans_matrices;
    while (this_node->mParent != NULL) {
        trans_matrices.push_back(this_node->mTransformation);
        this_node = this_node->mParent;
    }
    //multiply the transform matrix
    while (!trans_matrices.empty()) {
        out *= trans_matrices.back();
        trans_matrices.pop_back();
    }
}

//function to load a scene
bool load_scene(const string& dir) {
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
        aiProcess_GenSmoothNormals | //smooth the normals
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenBoundingBoxes |
        aiProcess_PreTransformVertices |
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
        aiMatrix4x4 cam_mat;
        cam->GetCameraMatrix(cam_mat);
        //retrieve_node_gtrans(cam_mat, scene, cam->mName.C_Str());
        //store
        Camera* new_cam = new Camera(cam, cam_mat);
        cams.push_back(new_cam);
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

    //select the first cam to use for now [TODO]: enable user to switch camera to render and provide a default if not read
    Camera* use_cam = cams[0];

    //*************************************************
    // RENDER EVERY PIXEL within multi-threaded blocks
    //*************************************************
    
    //progressively display blocks, making sure we have cutoffs on edge cases where determined block length is overbound
    const int scr_width = rasterizer.getWidth();
    const int scr_height = rasterizer.getHeight();
    for (int i = 0; i < scr_width; i += prog_disp_span) {
        for (int j = 0; j < scr_height; j += prog_disp_span) {
            //figure out parameters for each thread
            int cbx = i + prog_disp_span; //cutoff indicator x
            int cby = j + prog_disp_span; //cutoff indicator y
            const int img_width = prog_disp_span - cbx / scr_width * (cbx % scr_width);
            const int img_height = prog_disp_span - cby / scr_height * (cby % scr_height);
            int block_len;
            if (img_width > img_height) block_len = img_height / thread_n;
            else block_len = img_width / thread_n;

            //construct and run threads to render each divided block
            vector<thread> render_threads;
            for (int startX = i; startX < i + img_width; startX += block_len) {
                for (int startY = j; startY < j + img_height; startY += block_len) {
                    int cbi = startX + block_len; //cutoff indicator i
                    int ci = i + img_width; //cutoff i
                    int cbj = startY + block_len; //cutoff indicator j
                    int cj = j + img_height; //cutoff j
                    const int endX = startX + block_len - cbi / ci * (cbi % ci);
                    const int endY = startY + block_len - cbj / cj * (cbj % cj);
                    render_threads.push_back(thread(RenderThread(), &rasterizer, meshes, use_cam,
                        startX, startY, endX, endY,
                        ray_pool_page_size, set_hfov, samples_per_pixel, max_ray_bounce));
                }
            }

            //finish all threads
            for (thread& th : render_threads) {
                th.join();
            }
            display(window);
        }
    }

    //***************************
    // FINAL RESULTS HANDLING
    //***************************

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
    set_display_height,
    set_horizontal_fov,
    set_spp,
    set_bounce,
    set_pixel_blk_size
};

token_code get_token_code(string const& token){
    if (token == "-dispw") return token_code::set_display_width;
    if (token == "-disph") return token_code::set_display_height;
    if (token == "-hfov") return token_code::set_horizontal_fov;
    if (token == "-spp") return token_code::set_spp;
    if (token == "-bounce") return token_code::set_bounce;
    if (token == "-mpbs") return token_code::set_pixel_blk_size;
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
        case token_code::set_horizontal_fov:
            set_hfov = stof(tokens[1]);
            break;
        case token_code::set_spp:
            samples_per_pixel = stoi(tokens[1]);
            break;
        case token_code::set_bounce:
            max_ray_bounce = stoi(tokens[1]);
            break;
        case token_code::set_pixel_blk_size:
            prog_disp_span = stoi(tokens[1]);
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

    //close the log file
    fclose(fp);

    return 0;
}
