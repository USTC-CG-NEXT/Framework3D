#include "free_camera.hpp"

#include <pxr/base/gf/matrix4d.h>

#include <algorithm>
#include <cmath>

#include "widgets/api.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
void BaseCamera::UpdateWorldToView()
{
    auto m_MatTranslatedWorldToView = pxr::GfMatrix4d(
        m_CameraRight[0],
        m_CameraUp[0],
        -m_CameraDir[0],
        0.0,
        m_CameraRight[1],
        m_CameraUp[1],
        -m_CameraDir[1],
        0.0,
        m_CameraRight[2],
        m_CameraUp[2],
        -m_CameraDir[2],
        0.0,
        0.0,
        0.0,
        0.0,
        1.0);
    m_MatWorldToView =
        (pxr::GfMatrix4d().SetIdentity().SetTranslate(-m_CameraPos) *
         m_MatTranslatedWorldToView);

    auto xform_op = GetTransformOp();
    if (!xform_op) {
        xform_op = AddTransformOp();
    }
    // Set the transform op
    xform_op.Set(m_MatWorldToView.GetInverse());
}

BaseCamera::BaseCamera(const pxr::UsdGeomCamera& camera) : pxr::UsdGeomCamera(camera)
{
    pxr::UsdGeomXformOp transform_op = camera.GetTransformOp();
    if (transform_op) {
        pxr::GfMatrix4d transform_mat;
        transform_op.Get(&transform_mat);

        m_CameraPos = transform_mat.ExtractTranslation();
        m_CameraDir =
            -transform_mat.ExtractRotation().TransformDir(pxr::GfVec3d(0.0, 0.0, 1.0));
        m_CameraUp =
            transform_mat.ExtractRotation().TransformDir(pxr::GfVec3d(0.0, 1.0, 0.0));
        m_CameraRight =
            transform_mat.ExtractRotation().TransformDir(pxr::GfVec3d(1.0, 0.0, 0.0));
        m_MatWorldToView = transform_mat.GetInverse();
    }
    GetPrim().GetAttribute(pxr::TfToken("move_speed")).Get(&m_MoveSpeed);
}

void BaseCamera::BaseLookAt(
    pxr::GfVec3d cameraPos,
    pxr::GfVec3d cameraTarget,
    pxr::GfVec3d cameraUp)
{
    this->m_CameraPos = cameraPos;
    this->m_CameraDir = (cameraTarget - cameraPos).GetNormalized();
    this->m_CameraUp = cameraUp.GetNormalized();
    this->m_CameraRight =
        pxr::GfCross(this->m_CameraDir, this->m_CameraUp).GetNormalized();
    this->m_CameraUp =
        pxr::GfCross(this->m_CameraRight, this->m_CameraDir).GetNormalized();

    UpdateWorldToView();
}

FirstPersonCamera::FirstPersonCamera(const pxr::UsdGeomCamera& camera)
    : BaseCamera(camera)
{
}

void FirstPersonCamera::KeyboardUpdate(int key, int scancode, int action, int mods)
{
    if (keyboardMap.find(key) == keyboardMap.end()) {
        return;
    }

    auto cameraKey = keyboardMap.at(key);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        keyboardState[cameraKey] = true;
    }
    else {
        keyboardState[cameraKey] = false;
    }
}

void FirstPersonCamera::MousePosUpdate(double xpos, double ypos)
{
    mousePos = { double(xpos), double(ypos) };
}

void FirstPersonCamera::MouseButtonUpdate(int button, int action, int mods)
{
    if (mouseButtonMap.find(button) == mouseButtonMap.end()) {
        return;
    }

    auto cameraButton = mouseButtonMap.at(button);
    if (action == GLFW_PRESS) {
        mouseButtonState[cameraButton] = true;
    }
    else {
        mouseButtonState[cameraButton] = false;
    }
}

void FirstPersonCamera::LookAt(
    pxr::GfVec3d cameraPos,
    pxr::GfVec3d cameraTarget,
    pxr::GfVec3d cameraUp)
{
    // Make the base method public.
    BaseLookAt(cameraPos, cameraTarget, cameraUp);
}

void FirstPersonCamera::LookTo(
    pxr::GfVec3d cameraPos,
    pxr::GfVec3d cameraDir,
    pxr::GfVec3d cameraUp)
{
    BaseLookAt(cameraPos, cameraPos + cameraDir, cameraUp);
}
std::pair<bool, pxr::GfVec3d> FirstPersonCamera::AnimateTranslation(double deltaT)
{
    bool cameraDirty = false;
    double moveStep = deltaT * m_MoveSpeed;
    pxr::GfVec3d cameraMoveVec(0.0);

    if (keyboardState[KeyboardControls::SpeedUp])
        moveStep *= 3.0;

    if (keyboardState[KeyboardControls::SlowDown])
        moveStep *= 0.1;

    if (keyboardState[KeyboardControls::MoveForward]) {
        cameraDirty = true;
        cameraMoveVec += m_CameraDir * moveStep;
    }

    if (keyboardState[KeyboardControls::MoveBackward]) {
        cameraDirty = true;
        cameraMoveVec += -m_CameraDir * moveStep;
    }

    if (keyboardState[KeyboardControls::MoveLeft]) {
        cameraDirty = true;
        cameraMoveVec += -m_CameraRight * moveStep;
    }

    if (keyboardState[KeyboardControls::MoveRight]) {
        cameraDirty = true;
        cameraMoveVec += m_CameraRight * moveStep;
    }

    if (keyboardState[KeyboardControls::MoveUp]) {
        cameraDirty = true;
        cameraMoveVec += m_CameraUp * moveStep;
    }

    if (keyboardState[KeyboardControls::MoveDown]) {
        cameraDirty = true;
        cameraMoveVec += -m_CameraUp * moveStep;
    }
    return std::make_pair(cameraDirty, cameraMoveVec);
}
void FirstPersonCamera::UpdateCamera(
    pxr::GfVec3d cameraMoveVec,
    pxr::GfRotation cameraRotation)
{
    m_CameraPos += cameraMoveVec;
    m_CameraDir = cameraRotation.TransformDir(m_CameraDir).GetNormalized();
    m_CameraUp = cameraRotation.TransformDir(m_CameraUp).GetNormalized();
    m_CameraRight = pxr::GfCross(m_CameraDir, m_CameraUp).GetNormalized();

    UpdateWorldToView();
}

std::pair<bool, pxr::GfRotation> FirstPersonCamera::AnimateRoll(
    pxr::GfRotation initialRotation)
{
    bool cameraDirty = false;
    pxr::GfRotation cameraRotation = initialRotation;
    if (keyboardState[KeyboardControls::RollLeft] ||
        keyboardState[KeyboardControls::RollRight]) {
        double roll =
            double(keyboardState[KeyboardControls::RollLeft]) * -m_RotateSpeed * 2.0 +
            double(keyboardState[KeyboardControls::RollRight]) * m_RotateSpeed * 2.0;

        cameraRotation = pxr::GfRotation(m_CameraDir, roll) * cameraRotation;
        cameraDirty = true;
    }
    return std::make_pair(cameraDirty, cameraRotation);
}
void FirstPersonCamera::Animate(double deltaT)
{
    // track mouse delta
    pxr::GfVec2d mouseMove = mousePos - mousePosPrev;
    mousePosPrev = mousePos;

    bool cameraDirty = false;
    pxr::GfRotation cameraRotation = pxr::GfRotation().SetIdentity();

    // handle mouse rotation first
    // this will affect the movement vectors in the world matrix, which we use
    // below
    if (mouseButtonState[MouseButtons::Left] &&
        (mouseMove[0] != 0 || mouseMove[1] != 0)) {
        double yaw = m_RotateSpeed * mouseMove[0];
        double pitch = m_RotateSpeed * mouseMove[1];

        cameraRotation =
            pxr::GfRotation(pxr::GfVec3d(0.0, 0.0, 1.0), -yaw) * cameraRotation;
        cameraRotation = pxr::GfRotation(m_CameraRight, -pitch) * cameraRotation;

        cameraDirty = true;
    }

    // handle keyboard roll next
    auto rollResult = AnimateRoll(cameraRotation);
    cameraDirty |= rollResult.first;
    cameraRotation = rollResult.second;

    // handle translation
    auto translateResult = AnimateTranslation(deltaT);
    cameraDirty |= translateResult.first;
    const pxr::GfVec3d& cameraMoveVec = translateResult.second;

    if (cameraDirty) {
        UpdateCamera(cameraMoveVec, cameraRotation);
    }
}
void FirstPersonCamera::AnimateSmooth(double deltaT)
{
    const double c_DampeningRate = 7.5;
    double dampenWeight = exp(-c_DampeningRate * deltaT);

    pxr::GfVec2d mouseMove(0.0, 0.0);
    if (mouseButtonState[MouseButtons::Left]) {
        if (!isMoving) {
            isMoving = true;
            mousePosPrev = mousePos;
        }

        mousePosDamp[0] = pxr::GfLerp(dampenWeight, mousePos[0], mousePosPrev[0]);
        mousePosDamp[1] = pxr::GfLerp(dampenWeight, mousePos[1], mousePosPrev[1]);

        // track mouse delta
        mouseMove = mousePosDamp - mousePosPrev;
        mousePosPrev = mousePosDamp;
    }
    else {
        isMoving = false;
    }

    bool cameraDirty = false;
    pxr::GfRotation cameraRotation;

    // handle mouse rotation first
    // this will affect the movement vectors in the world matrix, which we use
    // below
    if (mouseMove[0] != 0 || mouseMove[1] != 0) {
        double yaw = m_RotateSpeed * mouseMove[0];
        double pitch = m_RotateSpeed * mouseMove[1];

        cameraRotation =
            pxr::GfRotation(pxr::GfVec3d(0.0, 1.0, 0.0), -yaw) * cameraRotation;
        cameraRotation = pxr::GfRotation(m_CameraRight, -pitch) * cameraRotation;

        cameraDirty = true;
    }

    // handle keyboard roll next
    auto rollResult = AnimateRoll(cameraRotation);
    cameraDirty |= rollResult.first;
    cameraRotation = rollResult.second;

    // handle translation
    auto translateResult = AnimateTranslation(deltaT);
    cameraDirty |= translateResult.first;
    const pxr::GfVec3d& cameraMoveVec = translateResult.second;

    if (cameraDirty) {
        UpdateCamera(cameraMoveVec, cameraRotation);
    }
}

void FirstPersonCamera::MouseScrollUpdate(double xoffset, double yoffset)
{
    GetPrim().GetAttribute(pxr::TfToken("move_speed")).Get(&m_MoveSpeed);
    m_MoveSpeed = std::clamp(m_MoveSpeed * (yoffset > 0 ? 1.05 : 1.0 / 1.05), 0.1, 500.0);
    using namespace pxr;
    GetPrim()
        .CreateAttribute(TfToken("move_speed"), SdfValueTypeNames->Double)
        .Set(m_MoveSpeed);
}

void ThirdPersonCamera::KeyboardUpdate(int key, int scancode, int action, int mods)
{
    if (keyboardMap.find(key) == keyboardMap.end()) {
        return;
    }

    auto cameraKey = keyboardMap.at(key);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        keyboardState[cameraKey] = true;
    }
    else {
        keyboardState[cameraKey] = false;
    }
}

void ThirdPersonCamera::MousePosUpdate(double xpos, double ypos)
{
    m_MousePos = pxr::GfVec2d(double(xpos), double(ypos));
}

void ThirdPersonCamera::MouseButtonUpdate(int button, int action, int mods)
{
    const bool pressed = (action == GLFW_PRESS);

    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mouseButtonState[MouseButtons::Left] = pressed;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            mouseButtonState[MouseButtons::Middle] = pressed;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mouseButtonState[MouseButtons::Right] = pressed;
            break;
        default: break;
    }
}

void ThirdPersonCamera::MouseScrollUpdate(double xoffset, double yoffset)
{
    const double scrollFactor = 1.15f;
    m_Distance = std::clamp(
        m_Distance * (yoffset < 0 ? scrollFactor : 1.0f / scrollFactor),
        m_MinDistance,
        m_MaxDistance);
}

void ThirdPersonCamera::JoystickUpdate(int axis, double value)
{
    switch (axis) {
        case GLFW_GAMEPAD_AXIS_RIGHT_X: m_DeltaYaw = value; break;
        case GLFW_GAMEPAD_AXIS_RIGHT_Y: m_DeltaPitch = value; break;
        default: break;
    }
}

void ThirdPersonCamera::JoystickButtonUpdate(int button, bool pressed)
{
    switch (button) {
        case GLFW_GAMEPAD_BUTTON_B:
            if (pressed)
                m_DeltaDistance -= 1;
            break;
        case GLFW_GAMEPAD_BUTTON_A:
            if (pressed)
                m_DeltaDistance += 1;
            break;
        default: break;
    }
}

void ThirdPersonCamera::SetRotation(double yaw, double pitch)
{
    m_Yaw = yaw;
    m_Pitch = pitch;
}

void ThirdPersonCamera::SetView(const pxr::GfFrustum& view)
{
    m_ProjectionMatrix = view.ComputeProjectionMatrix();
    m_InverseProjectionMatrix = m_ProjectionMatrix.GetInverse();
    auto viewport = view.GetWindow();
    m_ViewportSize = viewport.GetSize();
}
void ThirdPersonCamera::AnimateOrbit(double deltaT)
{
    if (mouseButtonState[MouseButtons::Left]) {
        pxr::GfVec2d mouseMove = m_MousePos - m_MousePosPrev;
        double rotateSpeed = m_RotateSpeed;

        m_Yaw -= rotateSpeed * mouseMove[0];
        m_Pitch += rotateSpeed * mouseMove[1];
    }

    const double ORBIT_SENSITIVITY = 1.5f;
    const double ZOOM_SENSITIVITY = 40.f;
    m_Distance += ZOOM_SENSITIVITY * deltaT * m_DeltaDistance;
    m_Yaw += ORBIT_SENSITIVITY * deltaT * m_DeltaYaw;
    m_Pitch += ORBIT_SENSITIVITY * deltaT * m_DeltaPitch;

    m_Distance = std::clamp(m_Distance, m_MinDistance, m_MaxDistance);
    m_Pitch = std::clamp(m_Pitch, -M_PI / 2, M_PI / 2);
    m_Pitch = std::clamp(m_Pitch, -M_PI / 2, M_PI / 2);

    m_DeltaDistance = 0;
    m_DeltaYaw = 0;
    m_DeltaPitch = 0;
}

void ThirdPersonCamera::AnimateTranslation(const pxr::GfMatrix3d& viewMatrix)
{
    // If the view parameters have never been set, we can't translate
    if (m_ViewportSize[0] <= 0.0 || m_ViewportSize[1] <= 0.0)
        return;

    if (m_MousePos == m_MousePosPrev)
        return;

    if (mouseButtonState[MouseButtons::Middle]) {
        pxr::GfVec4d oldClipPos(0.0, 0.0, m_Distance, 1.0);
        oldClipPos = oldClipPos * m_ProjectionMatrix;
        oldClipPos /= oldClipPos[3];
        oldClipPos[0] = 2.0 * (m_MousePosPrev[0]) / m_ViewportSize[0] - 1.0;
        oldClipPos[1] = 1.0 - 2.0 * (m_MousePosPrev[1]) / m_ViewportSize[1];
        pxr::GfVec4d newClipPos = oldClipPos;
        newClipPos[0] = 2.0 * (m_MousePos[0]) / m_ViewportSize[0] - 1.0;
        newClipPos[1] = 1.0 - 2.0 * (m_MousePos[1]) / m_ViewportSize[1];

        pxr::GfVec4d oldViewPos = oldClipPos * m_InverseProjectionMatrix;
        oldViewPos /= oldViewPos[3];
        pxr::GfVec4d newViewPos = newClipPos * m_InverseProjectionMatrix;
        newViewPos /= newViewPos[3];

        pxr::GfVec2d viewMotion(
            oldViewPos[0] - newViewPos[0], oldViewPos[1] - newViewPos[1]);

        m_TargetPos -= viewMotion[0] * viewMatrix.GetRow(0);

        if (keyboardState[KeyboardControls::HorizontalPan]) {
            pxr::GfVec3d horizontalForward =
                pxr::GfVec3d(viewMatrix.GetRow(2)[0], 0.0, viewMatrix.GetRow(2)[2]);
            double horizontalLength = horizontalForward.GetLength();
            if (horizontalLength == 0.0)
                horizontalForward =
                    pxr::GfVec3d(viewMatrix.GetRow(1)[0], 0.0, viewMatrix.GetRow(1)[2]);
            horizontalForward.Normalize();
            m_TargetPos += viewMotion[1] * horizontalForward * 1.5;
        }
        else
            m_TargetPos += viewMotion[1] * viewMatrix.GetRow(1);
    }
}

void ThirdPersonCamera::Animate(double deltaT)
{
    AnimateOrbit(deltaT);

    pxr::GfQuatd orbit = pxr::GfQuatd::GetIdentity();
    orbit.SetReal(cos(m_Pitch / 2.0) * cos(m_Yaw / 2.0));
    orbit.SetImaginary(pxr::GfVec3d(
        sin(m_Pitch / 2.0) * cos(m_Yaw / 2.0),
        cos(m_Pitch / 2.0) * sin(m_Yaw / 2.0),
        sin(m_Pitch / 2.0) * sin(m_Yaw / 2.0)));

    const pxr::GfMatrix3d targetRotation = pxr::GfMatrix3d().SetRotate(orbit);
    AnimateTranslation(targetRotation);

    const pxr::GfVec3d vectorToCamera = -m_Distance * targetRotation.GetRow(2);

    const pxr::GfVec3d camPos = m_TargetPos + vectorToCamera;

    m_CameraPos = camPos;
    m_CameraRight = -targetRotation.GetRow(0);
    m_CameraUp = targetRotation.GetRow(1);
    m_CameraDir = targetRotation.GetRow(2);
    UpdateWorldToView();

    m_MousePosPrev = m_MousePos;
}
void ThirdPersonCamera::LookAt(pxr::GfVec3d cameraPos, pxr::GfVec3d cameraTarget)
{
    pxr::GfVec3d cameraDir = cameraTarget - cameraPos;

    double azimuth, elevation, dirLength;
    CartesianToSpherical(cameraDir, azimuth, elevation, dirLength);

    SetTargetPosition(cameraTarget);
    SetDistance(dirLength);
    azimuth = -(azimuth + M_PI / 2.0f);
    SetRotation(azimuth, elevation);
}

void ThirdPersonCamera::LookTo(
    pxr::GfVec3d cameraPos,
    pxr::GfVec3d cameraDir,
    std::optional<double> targetDistance)
{
    double azimuth, elevation, dirLength;
    CartesianToSpherical(-cameraDir, azimuth, elevation, dirLength);
    cameraDir /= dirLength;

    double const distance = targetDistance.value_or(GetDistance());
    SetTargetPosition(cameraPos + cameraDir * distance);
    SetDistance(distance);
    azimuth = -(azimuth + M_PI / 2.0f);
    SetRotation(azimuth, elevation);
}

void ThirdPersonCamera::CartesianToSpherical(
    const pxr::GfVec3d& cartesian,
    double& azimuth,
    double& elevation,
    double& length)
{
    length = cartesian.GetLength();
    elevation = std::asin(cartesian[1] / length);
    azimuth = std::atan2(cartesian[2], cartesian[0]);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE