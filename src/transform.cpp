#include "transform.h"
#include <iostream>
#include <iomanip>

glm::mat4 Transform::poseToMatrix(const PosePacket& pose) {
    glm::vec3 position(pose.posX, pose.posY, pose.posZ);
    glm::vec3 rotationDegrees(pose.rotXdeg, pose.rotYdeg, pose.rotZdeg);
    return createTransform(position, rotationDegrees);
}

glm::mat4 Transform::createTransform(const glm::vec3& position, 
                                     const glm::vec3& rotationDegrees) {
    // Convert degrees to radians
    glm::vec3 rotationRadians = glm::radians(rotationDegrees);
    
    // Create rotation matrix from Euler angles (XYZ order)
    // Note: GLM uses Tait-Bryan angles (Y-X-Z intrinsic rotations)
    glm::mat4 rotationMatrix = glm::eulerAngleYXZ(rotationRadians.y, 
                                                   rotationRadians.x, 
                                                   rotationRadians.z);
    
    // Create translation matrix
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    
    // Combine: first rotate, then translate
    // Transform = Translation * Rotation
    return translationMatrix * rotationMatrix;
}

glm::vec3 Transform::transformPoint(const glm::mat4& transform, 
                                    const glm::vec3& localPoint) {
    // Convert to homogeneous coordinates (w=1 for points)
    glm::vec4 homogeneous(localPoint, 1.0f);
    
    // Apply transformation
    glm::vec4 worldHomogeneous = transform * homogeneous;
    
    // Convert back to 3D
    return glm::vec3(worldHomogeneous);
}

glm::vec3 Transform::transformLidarPoint(const glm::mat4& transform, 
                                         const LidarPoint& point) {
    glm::vec3 localPoint(point.x, point.y, point.z);
    return transformPoint(transform, localPoint);
}

std::vector<glm::vec3> Transform::transformLidarPoints(
    const glm::mat4& transform,
    const std::vector<LidarPoint>& localPoints) {
    
    std::vector<glm::vec3> worldPoints;
    worldPoints.reserve(localPoints.size());
    
    for (const auto& point : localPoints) {
        worldPoints.push_back(transformLidarPoint(transform, point));
    }
    
    return worldPoints;
}

glm::vec3 Transform::getPosition(const glm::mat4& transform) {
    // Position is in the last column of the 4x4 matrix
    return glm::vec3(transform[3]);
}

glm::vec3 Transform::getRotationDegrees(const glm::mat4& transform) {
    // Extract rotation matrix (upper-left 3x3)
    glm::mat3 rotationMatrix(transform);
    
    // Extract Euler angles (assuming YXZ order as used in createTransform)
    float x, y, z;
    glm::extractEulerAngleYXZ(glm::mat4(rotationMatrix), y, x, z);
    
    // Convert radians to degrees
    return glm::degrees(glm::vec3(x, y, z));
}

void Transform::printMatrix(const glm::mat4& mat, const char* name) {
    std::cout << name << ":" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    for (int i = 0; i < 4; i++) {
        std::cout << "  [";
        for (int j = 0; j < 4; j++) {
            std::cout << std::setw(8) << mat[j][i];
            if (j < 3) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}
