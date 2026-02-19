#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;

        this->cameraUpDirection = glm::normalize(cameraUp);

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        if (glm::length(this->cameraFrontDirection) < 0.0001f) {
            this->cameraFrontDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        }

        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
        this->cameraUpDirection = glm::normalize(glm::cross(this->cameraRightDirection, this->cameraFrontDirection));
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD) {
            cameraPosition += cameraFrontDirection * speed;
        }
        if (direction == MOVE_BACKWARD) {
            cameraPosition -= cameraFrontDirection * speed;
        }
        if (direction == MOVE_RIGHT) {
            cameraPosition += cameraRightDirection * speed;
        }
        if (direction == MOVE_LEFT) {
            cameraPosition -= cameraRightDirection * speed;
        }

        cameraTarget = cameraPosition + cameraFrontDirection;

        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 f = glm::normalize(cameraFrontDirection);

        float currentYaw = glm::degrees(atan2(f.z, f.x));
        float currentPitch = glm::degrees(asin(glm::clamp(f.y, -1.0f, 1.0f)));

        currentYaw += yaw;
        currentPitch += pitch;

        if (currentPitch > 89.0f) currentPitch = 89.0f;
        if (currentPitch < -89.0f) currentPitch = -89.0f;

        glm::vec3 newFront;
        newFront.x = cos(glm::radians(currentYaw)) * cos(glm::radians(currentPitch));
        newFront.y = sin(glm::radians(currentPitch));
        newFront.z = sin(glm::radians(currentYaw)) * cos(glm::radians(currentPitch));
        cameraFrontDirection = glm::normalize(newFront);

        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

        cameraTarget = cameraPosition + cameraFrontDirection;
    }
}
