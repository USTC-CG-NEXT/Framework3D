#pragma once
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>

#include <array>
#include <optional>
#include <unordered_map>

#include "GLFW/glfw3.h"
#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
// A camera with position and orientation. Methods for moving it come from
// derived classes.
class BaseCamera {
   public:
    virtual void KeyboardUpdate(int key, int scancode, int action, int mods)
    {
    }
    virtual void MousePosUpdate(double xpos, double ypos)
    {
    }
    virtual void MouseButtonUpdate(int button, int action, int mods)
    {
    }
    virtual void MouseScrollUpdate(double xoffset, double yoffset)
    {
    }
    virtual void JoystickButtonUpdate(int button, bool pressed)
    {
    }
    virtual void JoystickUpdate(int axis, float value)
    {
    }
    virtual void Animate(float deltaT)
    {
    }
    virtual ~BaseCamera() = default;

    void SetMoveSpeed(float value)
    {
        m_MoveSpeed = value;
    }
    void SetRotateSpeed(float value)
    {
        m_RotateSpeed = value;
    }

    [[nodiscard]] const pxr::GfMatrix4f& GetWorldToViewMatrix() const
    {
        return m_MatWorldToView;
    }
    [[nodiscard]] const pxr::GfMatrix4f& GetTranslatedWorldToViewMatrix() const
    {
        return m_MatTranslatedWorldToView;
    }
    [[nodiscard]] const pxr::GfVec3f& GetPosition() const
    {
        return m_CameraPos;
    }
    [[nodiscard]] const pxr::GfVec3f& GetDir() const
    {
        return m_CameraDir;
    }
    [[nodiscard]] const pxr::GfVec3f& GetUp() const
    {
        return m_CameraUp;
    }

   protected:
    // This can be useful for derived classes while not necessarily public,
    // i.e., in a third person camera class, public clients cannot direct the
    // gaze point.
    void BaseLookAt(
        pxr::GfVec3f cameraPos,
        pxr::GfVec3f cameraTarget,
        pxr::GfVec3f cameraUp = pxr::GfVec3f{ 0.f, 1.f, 0.f });
    void UpdateWorldToView();

    pxr::GfMatrix4f m_MatWorldToView = pxr::GfMatrix4f(1.0f);
    pxr::GfMatrix4f m_MatTranslatedWorldToView = pxr::GfMatrix4f(1.0f);

    pxr::GfVec3f m_CameraPos = pxr::GfVec3f(0.f);              // in worldspace
    pxr::GfVec3f m_CameraDir = pxr::GfVec3f(1.f, 0.f, 0.f);    // normalized
    pxr::GfVec3f m_CameraUp = pxr::GfVec3f(0.f, 1.f, 0.f);     // normalized
    pxr::GfVec3f m_CameraRight = pxr::GfVec3f(0.f, 0.f, 1.f);  // normalized

    float m_MoveSpeed = 1.f;      // movement speed in units/second
    float m_RotateSpeed = .005f;  // mouse sensitivity in radians/pixel
};

class FirstPersonCamera : public BaseCamera {
   public:
    void KeyboardUpdate(int key, int scancode, int action, int mods) override;
    void MousePosUpdate(double xpos, double ypos) override;
    void MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(float deltaT) override;
    void AnimateSmooth(float deltaT);

    void LookAt(
        pxr::GfVec3f cameraPos,
        pxr::GfVec3f cameraTarget,
        pxr::GfVec3f cameraUp = pxr::GfVec3f{ 0.f, 1.f, 0.f });
    void LookTo(
        pxr::GfVec3f cameraPos,
        pxr::GfVec3f cameraDir,
        pxr::GfVec3f cameraUp = pxr::GfVec3f{ 0.f, 1.f, 0.f });

   private:
    std::pair<bool, pxr::GfMatrix3f> AnimateRoll(
        pxr::GfMatrix3f initialRotation);
    std::pair<bool, pxr::GfVec3f> AnimateTranslation(float deltaT);
    void UpdateCamera(
        pxr::GfVec3f cameraMoveVec,
        pxr::GfMatrix3f cameraRotation);

    pxr::GfVec2f mousePos;
    pxr::GfVec2f mousePosPrev;
    // fields used only for AnimateSmooth()
    pxr::GfVec2f mousePosDamp;
    bool isMoving = false;

    typedef enum {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        MoveForward,
        MoveBackward,

        YawRight,
        YawLeft,
        PitchUp,
        PitchDown,
        RollLeft,
        RollRight,

        SpeedUp,
        SlowDown,

        KeyboardControlCount,
    } KeyboardControls;

    typedef enum {
        Left,
        Middle,
        Right,

        MouseButtonCount,
        MouseButtonFirst = Left,
    } MouseButtons;

    const std::unordered_map<int, int> keyboardMap = {
        { GLFW_KEY_Q, KeyboardControls::MoveDown },
        { GLFW_KEY_E, KeyboardControls::MoveUp },
        { GLFW_KEY_A, KeyboardControls::MoveLeft },
        { GLFW_KEY_D, KeyboardControls::MoveRight },
        { GLFW_KEY_W, KeyboardControls::MoveForward },
        { GLFW_KEY_S, KeyboardControls::MoveBackward },
        { GLFW_KEY_LEFT, KeyboardControls::YawLeft },
        { GLFW_KEY_RIGHT, KeyboardControls::YawRight },
        { GLFW_KEY_UP, KeyboardControls::PitchUp },
        { GLFW_KEY_DOWN, KeyboardControls::PitchDown },
        { GLFW_KEY_Z, KeyboardControls::RollLeft },
        { GLFW_KEY_C, KeyboardControls::RollRight },
        { GLFW_KEY_LEFT_SHIFT, KeyboardControls::SpeedUp },
        { GLFW_KEY_RIGHT_SHIFT, KeyboardControls::SpeedUp },
        { GLFW_KEY_LEFT_CONTROL, KeyboardControls::SlowDown },
        { GLFW_KEY_RIGHT_CONTROL, KeyboardControls::SlowDown },
    };

    const std::unordered_map<int, int> mouseButtonMap = {
        { GLFW_MOUSE_BUTTON_LEFT, MouseButtons::Left },
        { GLFW_MOUSE_BUTTON_MIDDLE, MouseButtons::Middle },
        { GLFW_MOUSE_BUTTON_RIGHT, MouseButtons::Right },
    };

    std::array<bool, KeyboardControls::KeyboardControlCount> keyboardState = {
        false
    };
    std::array<bool, MouseButtons::MouseButtonCount> mouseButtonState = {
        false
    };
};

class ThirdPersonCamera : public BaseCamera {
   public:
    void KeyboardUpdate(int key, int scancode, int action, int mods) override;
    void MousePosUpdate(double xpos, double ypos) override;
    void MouseButtonUpdate(int button, int action, int mods) override;
    void MouseScrollUpdate(double xoffset, double yoffset) override;
    void JoystickButtonUpdate(int button, bool pressed) override;
    void JoystickUpdate(int axis, float value) override;
    void Animate(float deltaT) override;

    pxr::GfVec3f GetTargetPosition() const
    {
        return m_TargetPos;
    }
    void SetTargetPosition(pxr::GfVec3f position)
    {
        m_TargetPos = position;
    }

    float GetDistance() const
    {
        return m_Distance;
    }
    void SetDistance(float distance)
    {
        m_Distance = distance;
    }

    float GetRotationYaw() const
    {
        return m_Yaw;
    }
    float GetRotationPitch() const
    {
        return m_Pitch;
    }
    void SetRotation(float yaw, float pitch);

    float GetMaxDistance() const
    {
        return m_MaxDistance;
    }
    void SetMaxDistance(float value)
    {
        m_MaxDistance = value;
    }

    void SetView(const pxr::GfFrustum& view);

    void LookAt(pxr::GfVec3f cameraPos, pxr::GfVec3f cameraTarget);
    void LookTo(
        pxr::GfVec3f cameraPos,
        pxr::GfVec3f cameraDir,
        std::optional<float> targetDistance = std::optional<float>());

   private:
    void AnimateOrbit(float deltaT);
    void AnimateTranslation(const pxr::GfMatrix3f& viewMatrix);

    // View parameters to derive translation amounts
    pxr::GfMatrix4f m_ProjectionMatrix = pxr::GfMatrix4f(1.0f);
    pxr::GfMatrix4f m_InverseProjectionMatrix = pxr::GfMatrix4f(1.0f);
    pxr::GfVec2f m_ViewportSize = pxr::GfVec2f(0.0f);

    pxr::GfVec2f m_MousePos = pxr::GfVec2f(0.0f);
    pxr::GfVec2f m_MousePosPrev = pxr::GfVec2f(0.0f);

    pxr::GfVec3f m_TargetPos = pxr::GfVec3f(0.0f);
    float m_Distance = 30.f;

    float m_MinDistance = 0.f;
    float m_MaxDistance = std::numeric_limits<float>::max();

    float m_Yaw = 0.f;
    float m_Pitch = 0.f;

    float m_DeltaYaw = 0.f;
    float m_DeltaPitch = 0.f;
    float m_DeltaDistance = 0.f;

    typedef enum {
        HorizontalPan,

        KeyboardControlCount,
    } KeyboardControls;

    const std::unordered_map<int, int> keyboardMap = {
        { GLFW_KEY_LEFT_ALT, KeyboardControls::HorizontalPan },
    };

    typedef enum {
        Left,
        Middle,
        Right,

        MouseButtonCount
    } MouseButtons;

    std::array<bool, KeyboardControls::KeyboardControlCount> keyboardState = {
        false
    };
    std::array<bool, MouseButtons::MouseButtonCount> mouseButtonState = {
        false
    };
};

USTC_CG_NAMESPACE_CLOSE_SCOPE