#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    std::cout << "LiDAR Visualization System - Build Test" << std::endl;
    
    // Test GLFW initialization
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Test GLM
    glm::vec3 testVector(1.0f, 2.0f, 3.0f);
    glm::mat4 identity = glm::mat4(1.0f);
    std::cout << "GLM test vector: (" 
              << testVector.x << ", " 
              << testVector.y << ", " 
              << testVector.z << ")" << std::endl;
    
    std::cout << "Build configuration successful!" << std::endl;
    
    glfwTerminate();
    return 0;
}
