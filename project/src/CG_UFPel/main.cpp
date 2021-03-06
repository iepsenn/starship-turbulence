#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <ctime>
#include <iostream>

//#include <SFML/Audio.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void generateObstacles(int nObstacles, vector<glm::mat4>& obstacles);
void drawObstacles(vector<glm::mat4>& obstacles, Model rock, Shader ourShader);
void moveObstacles(vector<glm::mat4>& obstacles, int turn);
void destroyObstacles(float z, vector<glm::mat4>& obstacles, int& points);
void checkColisions(glm::mat4 model, vector<glm::mat4>& obstacles);

//global variables
float z ;
float x ;
float y ;
int timeTotal  = 5; //velocidade de deslocamento dos objetos
int depthMax  = 15; //deslocamento total no eixo z que os objetos poderao estar
int depthMin =  10;
int xMax  = 1; // max 8
int yMax  = 1; // max 4
int points;
bool end_ = false;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{

    //ISoundEngine *SoundEngine = createIrrKlangDevice();

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Space Turbulence", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader(FileSystem::getPath("resources/cg_ufpel.vs").c_str(), FileSystem::getPath("resources/cg_ufpel.fs").c_str());

    // load models
    // -----------
    //Model ourModel(FileSystem::getPath("resources/objects/EnterpriseNCC1701D/Enterprise NCC 1701 D/enterprise1701d.obj"));
    Model ourModel(FileSystem::getPath("resources/objects/Flying Car/HN 48 Flying Car/HN 48 Flying Car.obj"));
    Model background(FileSystem::getPath("resources/objects/space/space.obj"));
    Model rock(FileSystem::getPath("resources/objects/rock/rock.obj"));

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



    //#include <ctime>
    srand(time(NULL));  //para gerar objetos na tela
    //cout << (rand() %10) + 1;

    float turn = 0.05;

    vector<glm::mat4> obstacles;

    int difficulty = 1;
    bool generate = true;
    bool flag = false;
    int generateFlag = 1;
    //char move;
    //int countMove = 1;


    glm::mat4 model;
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.008f, 0.008f, 0.008f));	// it's a bit too big for our scene, so scale it down

    glm::mat4 space;
    space = glm::translate(space, glm::vec3(0.0f, 0.0f, -16.0f));
    space = glm::rotate(space, glm::radians(90.0f), glm::vec3(0, 1, 0));
    space = glm::scale(space, glm::vec3(20.0f, 20.0f, 20.0f));

    /*sf::Music music;
    sf::Music endGame;

    if (!music.openFromFile("resources/sounds/topgear.wav"))
        return -1; // error

    if (!endGame.openFromFile("resources/sounds/end_game.wav"))
        return -1; // error
        */
    //music.play();
    //music.setLoop(true);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)){
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        //turn = (difficulty/timeTotal)/difficulty;
        //turn = 0.05;
        /*
        currentFrame   ----   ?
        timeTotal     -----   depthTotal
        */

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        //glm::mat4 model;
        //model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // translate it down so it's at the center of the scene
        //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
        //model = glm::scale(model, glm::vec3(0.008f, 0.008f, 0.008f));	// it's a bit too big for our scene, so scale it down
        //if(move=='l') { model = glm::translate(model, glm::vec3(0.0f, turn*countMove, 0.0f)); }
        //if(move=='r') { model = glm::translate(model, glm::vec3(0.0f, (-turn)*countMove, 0.0f)); }

        ourShader.setMat4("model", space);
        background.Draw(ourShader);

        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        //game preparation
        if(generate){ generateObstacles(difficulty, obstacles); generate=false; }
        drawObstacles(obstacles, rock, ourShader);

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { flag = !flag; }

        if (flag) {
            //the game begins
            //translate de obstacles (move our starship)
            for(int i=0; i < (int)obstacles.size(); ++i) {
                float translate =  obstacles[i][3][2] + turn;
                obstacles[i][3][2] = translate;
           }
           int curPoints = points;
           //check end game
           checkColisions(model, obstacles);
           if(end_) { /*music.stop(); endGame.play();*/ break; } //exit(0); }
           //check when obstacles get out the plan
           destroyObstacles(model[3][2], obstacles, points);
           //increase the difficulty and generate more obstacles
           if(points > curPoints) { ++generateFlag; generate = true; if(points%3 == 0 ){ turn += 0.01; } } //check if the obstacles are destroyed
           if(generateFlag%7 == 0) { ++difficulty; ++generateFlag; }
           if(obstacles.size() == 7) { difficulty = 1; }
           //see the colisions
        }
        //cout << "number of obstacles: " << obstacles.size() << endl;

        //moves
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            model[3][1] += turn;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            model[3][1] -= turn;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            model[3][0] -= turn;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            model[3][0] += turn;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cout << endl << endl << endl;
    cout << "you did " << points << " points" << endl;
    cout << endl << endl;
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------

    //SoundEngine->drop(); // delete engine

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


void generateObstacles(int nObstacles,vector<glm::mat4>& obstacles){
    float x;
    float y;
    float z;

    for(int i=0;i<nObstacles;++i) {
          glm::mat4 matrixGenerator;
          int signalX = rand() % 2;
          int signalY = rand() % 2;
          //cout << "signalX: " << signalX << endl;
          z = (((5 + (rand() % ( 15 - 5 + 1 )))) * (-1));
          x = rand() % xMax;
          y = rand()% yMax;

          if(signalX==1) { x *= (-1); }
          if(signalY==1) { y *= (-1); }

          matrixGenerator = glm::translate(matrixGenerator, glm::vec3(x, y, z));
          matrixGenerator = glm::scale(matrixGenerator, glm::vec3(0.5f, 0.5f, 0.5f));

          obstacles.push_back( matrixGenerator );
    }
}


void drawObstacles(vector<glm::mat4>& obstacles, Model rock, Shader ourShader){
    for(int i=0; i < (int)obstacles.size(); ++i) {
        ourShader.setMat4("model", obstacles[i]);
        rock.Draw(ourShader);
    }
}

void moveObstacles(vector<glm::mat4>& obstacles, int turn){
    for(int i=0; i < (int)obstacles.size(); ++i) {
        float translate =  obstacles[i][3][2] + turn;
        obstacles[i][3][2] = translate;
    }

}

void destroyObstacles(float z, vector<glm::mat4>& obstacles, int& points){
    for(int i=0; i < (int)obstacles.size(); ++i) {
        if(obstacles[i][3][2] > z+2) { obstacles.erase(obstacles.begin() + i); ++points;}
    }

    if(xMax < 8) {
        if(points%5 == 0) { ++xMax; } //increase range of random
    }

    if(yMax < 4) {
        if(points%5 == 0) { ++yMax; }
    }
}

void checkColisions(glm::mat4 model, vector<glm::mat4>& obstacles){ //x+! x-1 y+1 y-1
     for(int i=0; i < (int)obstacles.size(); ++i) { //check only in z axis
         //cout << "obstacle " << obstacles[i][3][2] << endl;
         //cout << "model" <<  model[3][2] << endl;
         if(obstacles[i][3][2]+2.0f >= -0.0005f) {
           if(obstacles[i][3][0] <= model[3][0]+0.5f && obstacles[i][3][0] >= model[3][0] && obstacles[i][3][1] <= model[3][1]+0.5f && obstacles[i][3][1] >= model[3][1]) { //dir
                  end_ = true;
                  return;
            }else if(obstacles[i][3][0] >= model[3][0]-0.5f && obstacles[i][3][0] <= model[3][0] && obstacles[i][3][1] >= model[3][1]-0.5f && obstacles[i][3][1] <= model[3][1]) {//esq
                  end_ = true;
                  return;
            }if(obstacles[i][3][0] <= model[3][0]+0.5f && obstacles[i][3][0] >= model[3][0] && obstacles[i][3][1] >= model[3][1]-0.5f && obstacles[i][3][1] <= model[3][1]) { //dir
                  end_ = true;
                  return;
            }else if(obstacles[i][3][0] >= model[3][0]-0.5f && obstacles[i][3][0] <= model[3][0] && obstacles[i][3][1] <= model[3][1]+0.5f && obstacles[i][3][1] >= model[3][1]) {//esq
                  end_ = true;
                  return;
            }
         }
    }
}
