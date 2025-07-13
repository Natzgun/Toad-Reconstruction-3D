#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ModelLoader.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <ostream>
#include <vector>

// GLobal Variables
// Shaders
unsigned int vertexShader, fragmentShader, shaderProgram;


unsigned int VBO, VAO;


float rotationX = 0.0f, rotationY = 0.0f;
glm::vec3 cameraPos = glm::vec3(250.0f, 200.0f, 600.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;

float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Vertex Buffer obejects

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\0";

// const char *vertexShaderSource = "#version 330 core\n"
//   "layout (location = 0) in vec3 aPos;\n"
//   "void main() {\n"
//   "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
//   "}\0";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 color;\n"
    "void main() {\n"
    "    FragColor = vec4(color, 1.0);\n"
    "}\0";

void loadShaders() {

  /* ----------- SETUP SHADERS -----------*/
  /* Crea un object shader que se va identificar con un unsigned int, y luego
   * cramos el shader, el argumento es el tipo de shader, por lo que este object
   * shader sera un vertex shader */
  vertexShader = glCreateShader(GL_VERTEX_SHADER);

  /*Ahora enlazamos el codigo del shader con shader object (vertex shader en
   * este caso) y luego compilamos el shader, el segundo argumento es apra
   * indicar cuantos string le vamos a pasar en este caso solo 1, el ultimo
   * argument es un arreglo de longitudes en este caso NULL hace referencia a
   * que encontraremos un \0 al final*/
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  /* Revisamos si la compilacion del shader fue exitosa */
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  /*La funcion principal de un Fragment shader es calcular el color de salida de
   * los pixeles, el FS siembre generará un color anaranjado*/

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  // Verificamos que no haya error en la compilacion
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  /*Un Shader Program es la version final enlazada de varios shaders combinados,
   * asi que creamos uno*/
  shaderProgram = glCreateProgram();

  /*Ahora adjuntamos los shaders previamente compilados  al Program Object*/
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
  }

  /* Podemos activar este Program Object con glUseProgram(shaderProgram), y no
   * olvidemos eliminar los shaders ya vinculados, podemos ponerlo al final del
   * codigo */

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void setupBuffers(const std::vector<glm::vec3> &vertex) {
  /* ------------ SETUP VERTEX DATA ------------*/
  /* Verices del triangulo pero estas coordenadas estan en NDC (Normalized
   * Device Corrdinates) que van desde -1.0 hasta 1.0, cualquier valor fuera de
   * este rango OpenGL no los mostrará*/

  std::vector<float> vertexData;
  vertexData.reserve(vertex.size() * 3);
  for (const auto &v : vertex) {
    vertexData.insert(vertexData.end(), {v.x, v.y, v.z});
  }

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
               vertexData.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

void render(const int vertexCount) {
  // Draw the object
  glUseProgram(shaderProgram);

  // Camera

  glm::mat4 model = glm::mat4(1.0f);

  model = glm::rotate(model,
    glm::radians(rotationX),
    glm::vec3(1.0f, 0.0f, 0.0f)
  );
  model = glm::rotate(model,
    glm::radians(rotationY),
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  // model = glm::scale(model, glm::vec3(1.0f));

  glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

  glm::mat4 proj = glm::perspective(
    glm::radians(45.0f),
    800.0f / 600.0f,
    0.1f, 1000.0f
  );

// glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 500.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
// glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                     glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                     GL_FALSE, glm::value_ptr(proj));


  glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f);
  // Aqui le decimos que use el VAO con la configuracion que ya guardamos
  // antes
  glBindVertexArray(VAO);

  glPointSize(2.0f);

  /* Dibuja primitivas utilizando el shader actualmente activo */
  /* Primer argumento, tipo primitivo de OpenGL, el segundo elemento
   * especifica el índice inicial del array de vertices, el último parametro
   * especifica cuántos verttices queremos dibujar que es 3, ya que nuestro
   * arreglo de vertices tiene exactamente 3 vertices */
  glDrawArrays(GL_POINTS, 0, vertexCount);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}


int main(int argc, char *argv[]) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(800, 600, "Full OpenGL", NULL, NULL);

  if (window == NULL) {
    std::cout << "Fail to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  /* Indicamos a GLFW que convierta el contexto de nuestra ventana en el
   * contexto principal del hilo actual*/
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Fail to initialize GLAD" << std::endl;
    return -1;
  }
  glEnable(GL_DEPTH_TEST);

  loadShaders();


  ModelLoader loader;
  std::vector<glm::vec3> vertex;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;


  loader.loadOBJ("../../images/skeletonMasks.obj", vertex, uvs, normals);

  int sizeVertex = vertex.size();
  std::cout << sizeVertex << std::endl;
  setupBuffers(vertex);

  // ahora puedes usar 'vertices' para crear tu VAO/VBO

  /* La condicion revisa en cada loop si hay una instruccion que va cerrar la
   * ventana */
  while (!glfwWindowShouldClose(window)) {

    // --- Input ---
    processInput(window);

    // -- Funciones de render ---

    // Define el color con el que se va limpiar el color buffer, osea cuando
    // limpie el color buffer del frame anterior lo va llenar con estre color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    /* Borra el contenido del color buffer osea del frame anterior y lo rellena
     * con el color definido por glClearColor*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // std::cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;

    render(sizeVertex);

    /* glfwSwapBuffers(window) intercambia el back buffer (donde OpenGL dibuja)
    con el front buffer (lo que se ve en pantalla). Durante cada frame, todo
    se dibuja primero en el back buffer, que es básicamente una imagen 2D con
    los colores de cada píxel (color buffer). Una vez que el frame está
    listo, glfwSwapBuffers lo muestra en la ventana. */
    glfwSwapBuffers(window);
    /* Verifica si un evento se ha activado, en base a ella actualiza el estado
     * de la ventana y llama a las funciones callback que yo haya registrado*/
    glfwPollEvents();
  }

  /*Limpiamos los recursos de GLFW asignados*/
  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  float cameraSpeed = 50.0f * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  /* Los primeros dos puntos establece la ubicacion de la esquina inferior
   * izquiera de la ventana, los dos ultimos define el ancho y la altura de la
   * ventana de renderizado en pixeles*/
  glViewport(0, 0, width, height);
}
