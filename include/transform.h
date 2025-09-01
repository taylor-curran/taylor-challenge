#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <vector>
#include "udp_packet_structures.h"

class Transform {
public:
    // Create transform from pose packet
    static glm::mat4 poseToMatrix(const PosePacket& pose);
    
    // Create transform from position and euler angles (in degrees)
    static glm::mat4 createTransform(const glm::vec3& position, 
                                     const glm::vec3& rotationDegrees);
    
    // Transform a single point from local to world coordinates
    static glm::vec3 transformPoint(const glm::mat4& transform, 
                                    const glm::vec3& localPoint);
    
    // Transform a LiDAR point from local to world coordinates
    static glm::vec3 transformLidarPoint(const glm::mat4& transform, 
                                         const LidarPoint& point);
    
    // Transform multiple LiDAR points from local to world coordinates
    static std::vector<glm::vec3> transformLidarPoints(
        const glm::mat4& transform,
        const std::vector<LidarPoint>& localPoints);
    
    // Utility: Convert degrees to radians
    static float degreesToRadians(float degrees) {
        return glm::radians(degrees);
    }
    
    // Utility: Extract position from transform matrix
    static glm::vec3 getPosition(const glm::mat4& transform);
    
    // Utility: Extract rotation (as euler angles in degrees) from transform matrix
    static glm::vec3 getRotationDegrees(const glm::mat4& transform);
    
    // Debug: Print transform matrix
    static void printMatrix(const glm::mat4& mat, const char* name = "Matrix");
};

#endif // TRANSFORM_H
