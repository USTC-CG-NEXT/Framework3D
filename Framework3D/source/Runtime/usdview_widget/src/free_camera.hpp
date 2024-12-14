#pragma once

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix3d.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/quatd.h>
#include <pxr/base/gf/rotation.h>  // Include GfRotation
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4d.h>

#include <optional>
#include <unordered_map>

#include "widgets/api.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "pxr/usd/usdGeom/camera.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class BaseCamera : public pxr::UsdGeomCamera {
   public:
    BaseCamera() = default;

    BaseCamera(const pxr::UsdGeomCamera& camera);

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
    virtual void JoystickUpdate(int axis, double value)
    {
    }
    virtual void Animate(double deltaT)
    {
    }
    virtual ~BaseCamera() = default;

    void SetMoveSpeed(double value)
    {
        m_MoveSpeed = value;
    }
    void SetRotateSpeed(double value)
    {
        m_RotateSpeed = value;
    }

    [[nodiscard]] const pxr::GfMatrix4d& GetWorldToViewMatrix() const
    {
        return m_MatWorldToView;
    }
    [[nodiscard]] const pxr::GfVec3d& GetPosition() const
    {
        return m_CameraPos;
    }
    [[nodiscard]] const pxr::GfVec3d& GetDir() const
    {
        return m_CameraDir;
    }
    [[nodiscard]] const pxr::GfVec3d& GetUp() const
    {
        return m_CameraUp;
    }

   protected:
    void BaseLookAt(
        pxr::GfVec3d cameraPos,
        pxr::GfVec3d cameraTarget,
        pxr::GfVec3d cameraUp = pxr::GfVec3d{ 0.0, 1.0, 0.0 });
    void UpdateWorldToView();

    pxr::GfMatrix4d m_MatWorldToView = pxr::GfMatrix4d(1.0);

    pxr::GfVec3d m_CameraPos = pxr::GfVec3d(0.0);
    pxr::GfVec3d m_CameraDir = pxr::GfVec3d(1.0, 0.0, 0.0);
    pxr::GfVec3d m_CameraUp = pxr::GfVec3d(0.0, 1.0, 0.0);
    pxr::GfVec3d m_CameraRight = pxr::GfVec3d(0.0, 0.0, 1.0);

    double m_MoveSpeed = 1;
    double m_RotateSpeed = .05;
};

class FirstPersonCamera : public BaseCamera {
   public:
    FirstPersonCamera() = default;

    explicit FirstPersonCamera(const pxr::UsdGeomCamera& camera);

    void KeyboardUpdate(int key, int scancode, int action, int mods) override;
    void MousePosUpdate(double xpos, double ypos) override;
    void MouseButtonUpdate(int button, int action, int mods) override;
    void Animate(double deltaT) override;
    void AnimateSmooth(double deltaT);

    void MouseScrollUpdate(double xoffset, double yoffset) override;

    void LookAt(
        pxr::GfVec3d cameraPos,
        pxr::GfVec3d cameraTarget,
        pxr::GfVec3d cameraUp = pxr::GfVec3d{ 0.0, 1.0, 0.0 });
    void LookTo(
        pxr::GfVec3d cameraPos,
        pxr::GfVec3d cameraDir,
        pxr::GfVec3d cameraUp = pxr::GfVec3d{ 0.0, 1.0, 0.0 });

   private:
    std::pair<bool, pxr::GfRotation> AnimateRoll(
        pxr::GfRotation initialRotation);
    std::pair<bool, pxr::GfVec3d> AnimateTranslation(double deltaT);
    void UpdateCamera(
        pxr::GfVec3d cameraMoveVec,
        pxr::GfRotation cameraRotation);

    pxr::GfVec2d mousePos;
    pxr::GfVec2d mousePosPrev;
    pxr::GfVec2d mousePosDamp;
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
    void JoystickUpdate(int axis, double value) override;
    void Animate(double deltaT) override;

    pxr::GfVec3d GetTargetPosition() const
    {
        return m_TargetPos;
    }
    void SetTargetPosition(pxr::GfVec3d position)
    {
        m_TargetPos = position;
    }

    double GetDistance() const
    {
        return m_Distance;
    }
    void SetDistance(double distance)
    {
        m_Distance = distance;
    }

    double GetRotationYaw() const
    {
        return m_Yaw;
    }
    double GetRotationPitch() const
    {
        return m_Pitch;
    }
    void SetRotation(double yaw, double pitch);

    double GetMaxDistance() const
    {
        return m_MaxDistance;
    }
    void SetMaxDistance(double value)
    {
        m_MaxDistance = value;
    }

    void SetView(const pxr::GfFrustum& view);

    void LookAt(pxr::GfVec3d cameraPos, pxr::GfVec3d cameraTarget);
    void LookTo(
        pxr::GfVec3d cameraPos,
        pxr::GfVec3d cameraDir,
        std::optional<double> targetDistance = std::optional<double>());
    void CartesianToSpherical(
        const pxr::GfVec3d& cartesian,
        double& azimuth,
        double& elevation,
        double& length);

   private:
    void AnimateOrbit(double deltaT);
    void AnimateTranslation(const pxr::GfMatrix3d& viewMatrix);

    pxr::GfMatrix4d m_ProjectionMatrix = pxr::GfMatrix4d(1.0);
    pxr::GfMatrix4d m_InverseProjectionMatrix = pxr::GfMatrix4d(1.0);
    pxr::GfVec2d m_ViewportSize = pxr::GfVec2d(0.0);

    pxr::GfVec2d m_MousePos = pxr::GfVec2d(0.0);
    pxr::GfVec2d m_MousePosPrev = pxr::GfVec2d(0.0);

    pxr::GfVec3d m_TargetPos = pxr::GfVec3d(0.0);
    double m_Distance = 30.0;

    double m_MinDistance = 0.0;
    double m_MaxDistance = std::numeric_limits<double>::max();

    double m_Yaw = 0.0;
    double m_Pitch = 0.0;

    double m_DeltaYaw = 0.0;
    double m_DeltaPitch = 0.0;
    double m_DeltaDistance = 0.0;

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
