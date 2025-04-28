#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
struct Camera {
    glm::vec3 _position = glm::vec3(0.f);
    glm::vec3 _front = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 _up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 _right = glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 _world_up = glm::vec3(0.f, 1.f, 0.f);
    float _yaw = -90.f;
    float _pitch = 0.f;
    float _movement_speed = 2.5f;
    float _mouse_sensitivity = 0.1f;
    float _zoom = 45.f;

      float _mouse_last_x = NAN;
    float _mouse_last_y = NAN;

    glm::mat4 view_matrix() const
    {
        return glm::lookAt(_position, _position + _front, _up);
    }

    void on_keyboard_move(glm::vec3 delta, float deltaTime)
    {
        float velocity = _movement_speed * deltaTime;
        _position += delta * velocity;
    }

    void on_mouse_scroll(float yoffset)
    {
        _zoom -= yoffset;
    }

    void on_mouse_move(float x, float y)
    {
        if (std::isnan(_mouse_last_x) || std::isnan(_mouse_last_y))
        {
            _mouse_last_x = x;
            _mouse_last_y = y;
        }
        const float xoffset = (x - _mouse_last_x) * _mouse_sensitivity;
        const float yoffset = (_mouse_last_y - y) * _mouse_sensitivity;
        _mouse_last_x = x;
        _mouse_last_y = y;
        _yaw += xoffset;
        _pitch += yoffset;
        force_refresh();
    }

    void force_refresh()
    {
        _pitch = std::clamp(_pitch, -89.0f, 89.0f);

        _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front.y = sin(glm::radians(_pitch));
        _front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front = glm::normalize(_front);
        _right = glm::normalize(glm::cross(_front, _world_up));
        _up = glm::normalize(glm::cross(_right, _front));
    }
};