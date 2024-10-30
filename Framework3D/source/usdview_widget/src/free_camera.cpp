#include "free_camera.hpp"

#include <optional>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/math.h>

#include "USTC_CG.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

void BaseCamera::UpdateWorldToView()
{
    m_MatTranslatedWorldToView =
        pxr::GfMatrix4f(m_CameraRight, m_CameraUp, m_CameraDir, pxr::GfVec3f(0.f));
    m_MatWorldToView = pxr::GfMatrix4f().SetTranslate(-m_CameraPos) * m_MatTranslatedWorldToView;
}

void BaseCamera::BaseLookAt(
    pxr::GfVec3f cameraPos,
    pxr::GfVec3f cameraTarget,
    pxr::GfVec3f cameraUp)
{
    this->m_CameraPos = cameraPos;
    this->m_CameraDir = pxr::GfNormalize(cameraTarget - cameraPos);
    this->m_CameraUp = pxr::GfNormalize(cameraUp);
    this->m_CameraRight = pxr::GfNormalize(pxr::GfCross(this->m_CameraDir, this->m_CameraUp));
    this->m_CameraUp = pxr::GfNormalize(pxr::GfCross(this->m_CameraRight, this->m_CameraDir));

    UpdateWorldToView();
}

void FirstPersonCamera::KeyboardUpdate(
    int key,
    int scancode,
    int action,
    int mods)
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
    mousePos = { float(xpos), float(ypos) };
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
    pxr::GfVec3f cameraPos,
    pxr::GfVec3f cameraTarget,
    pxr::GfVec3f cameraUp)
{
    // Make the base method public.
    BaseLookAt(cameraPos, cameraTarget, cameraUp);
}

void FirstPersonCamera::LookTo(
    pxr::GfVec3f cameraPos,
    pxr::GfVec3f cameraDir,
    pxr::GfVec3f cameraUp)
{
    BaseLookAt(cameraPos, cameraPos + cameraDir, cameraUp);
}

std::pair<bool, pxr::GfVec3f> FirstPersonCamera::AnimateTranslation(float deltaT)
{
    bool cameraDirty = false;
    float moveStep = deltaT * m_MoveSpeed;
    pxr::GfVec3f cameraMoveVec(0.f);

    if (keyboardState[KeyboardControls::SpeedUp])
        moveStep *= 3.f;

    if (keyboardState[KeyboardControls::SlowDown])
        moveStep *= .1f;

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
    pxr::GfVec3f cameraMoveVec,
    pxr::GfMatrix4f cameraRotation)
{
    m_CameraPos += cameraMoveVec;
    m_CameraDir = pxr::GfNormalize(cameraRotation.TransformDir(m_CameraDir));
    m_CameraUp = pxr::GfNormalize(cameraRotation.TransformDir(m_CameraUp));
    m_CameraRight = pxr::GfNormalize(pxr::GfCross(m_CameraDir, m_CameraUp));

    UpdateWorldToView();
}

std::pair<bool, pxr::GfMatrix4f> FirstPersonCamera::AnimateRoll(pxr::GfMatrix4f initialRotation)
{
    bool cameraDirty = false;
    pxr::GfMatrix4f cameraRotation = initialRotation;
    if (keyboardState[KeyboardControls::RollLeft] ||
        keyboardState[KeyboardControls::RollRight]) {
        float roll = float(keyboardState[KeyboardControls::RollLeft]) *
                         -m_RotateSpeed * 2.0f +
                     float(keyboardState[KeyboardControls::RollRight]) *
                         m_RotateSpeed * 2.0f;

        cameraRotation = pxr::GfMatrix4f().SetRotate(pxr::GfRotation(m_CameraDir, roll)) * cameraRotation;
        cameraDirty = true;
    }
    return std::make_pair(cameraDirty, cameraRotation);
}

void FirstPersonCamera::Animate(float deltaT)
{
    // track mouse delta
    pxr::GfVec2f mouseMove = mousePos - mousePosPrev;
    mousePosPrev = mousePos;

    bool cameraDirty = false;
    pxr::GfMatrix4f cameraRotation(1.0f);

    // handle mouse rotation first
    // this will affect the movement vectors in the world matrix, which we use
    // below
    if (mouseButtonState[MouseButtons::Left] &&
        (mouseMove[0] != 0 || mouseMove[1] != 0)) {
        float yaw = m_RotateSpeed * mouseMove[0];
        float pitch = m_RotateSpeed * mouseMove[1];

        cameraRotation = pxr::GfMatrix4f().SetRotate(pxr::GfRotation(pxr::GfVec3f(0.f, 1.f, 0.f), -yaw));
        cameraRotation = pxr::GfMatrix4f().SetRotate(pxr::GfRotation(m_CameraRight, -pitch)) * cameraRotation;

        cameraDirty = true;
    }

    // handle keyboard roll next
    auto rollResult = AnimateRoll(cameraRotation);
    cameraDirty |= rollResult.first;
    cameraRotation = rollResult.second;

    // handle translation
    auto translateResult = AnimateTranslation(deltaT);
    cameraDirty |= translateResult.first;
    const pxr::GfVec3f& cameraMoveVec = translateResult.second;

    if (cameraDirty) {
        UpdateCamera(cameraMoveVec, cameraRotation);
    }
}

void FirstPersonCamera::AnimateSmooth(float deltaT)
{
    const float c_DampeningRate = 7.5f;
    float dampenWeight = exp(-c_DampeningRate * deltaT);

    pxr::GfVec2f mouseMove(0, 0);
    if (mouseButtonState[MouseButtons::Left]) {
        if (!isMoving) {
            isMoving = true;
            mousePosPrev = mousePos;
        }

        mousePosDamp[0] = pxr::GfLerp(mousePos[0], mousePosPrev[0], dampenWeight);
        mousePosDamp[1] = pxr::GfLerp(mousePos[1], mousePosPrev[1], dampenWeight);

        // track mouse delta
        mouseMove = mousePosDamp - mousePosPrev;
        mousePosPrev = mousePosDamp;
    }
    else {
        isMoving = false;
    }

    bool cameraDirty = false;
    pxr::GfMatrix4f cameraRotation(1.0f);

    // handle mouse rotation first
    // this will affect the movement vectors in the world matrix, which we use
    // below
    if (mouseMove[0] || mouseMove[1]) {
        float yaw = m_RotateSpeed * mouseMove[0];
        float pitch = m_RotateSpeed * mouseMove[1];

        cameraRotation = pxr::GfMatrix4f().SetRotate(pxr::GfRotation(pxr::GfVec3f(0.f, 1.f, 0.f), -yaw));
        cameraRotation = pxr::GfMatrix4f().SetRotate(pxr::GfRotation(m_CameraRight, -pitch)) * cameraRotation;

        cameraDirty = true;
    }

    // handle keyboard roll next
    auto rollResult = AnimateRoll(cameraRotation);
    cameraDirty |= rollResult.first;
    cameraRotation = rollResult.second;

    // handle translation
    auto translateResult = AnimateTranslation(deltaT);
    cameraDirty |= translateResult.first;
    const pxr::GfVec3f& cameraMoveVec = translateResult.second;

    if (cameraDirty) {
        UpdateCamera(cameraMoveVec, cameraRotation);
    }
}

void ThirdPersonCamera::KeyboardUpdate(
    int key,
    int scancode,
    int action,
    int mods)
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
    m_MousePos = pxr::GfVec2f(float(xpos), float(ypos));
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
    const float scrollFactor = 1.15f;
    m_Distance = pxr::GfClamp(
        m_Distance * (yoffset < 0 ? scrollFactor : 1.0f / scrollFactor),
        m_MinDistance,
        m_MaxDistance);
}

void ThirdPersonCamera::JoystickUpdate(int axis, float value)
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

void ThirdPersonCamera::SetRotation(float yaw, float pitch)
{
    m_Yaw = yaw;
    m_Pitch = pitch;
}

void ThirdPersonCamera::SetView(const engine::PlanarView& view)
{
    m_ProjectionMatrix = view.GetProjectionMatrix(false);
    m_InverseProjectionMatrix = view.GetInverseProjectionMatrix(false);
    auto viewport = view.GetViewport();
    m_ViewportSize = pxr::GfVec2f(viewport.width(), viewport.height());
}

void ThirdPersonCamera::AnimateOrbit(float deltaT)
{
    if (mouseButtonState[MouseButtons::Left]) {
        pxr::GfVec2f mouseMove = m_MousePos - m_MousePosPrev;
        float rotateSpeed = m_RotateSpeed;

        m_Yaw -= rotateSpeed * mouseMove[0];
        m_Pitch += rotateSpeed * mouseMove[1];
    }

    const float ORBIT_SENSITIVITY = 1.5f;
    const float ZOOM_SENSITIVITY = 40.f;
    m_Distance += ZOOM_SENSITIVITY * deltaT * m_DeltaDistance;
    m_Yaw += ORBIT_SENSITIVITY * deltaT * m_DeltaYaw;
    m_Pitch += ORBIT_SENSITIVITY * deltaT * m_DeltaPitch;

    m_Distance = pxr::GfClamp(m_Distance, m_MinDistance, m_MaxDistance);

    m_Pitch = pxr::GfClamp(m_Pitch, pxr::GfHalfPi<float>() * -1.0f, pxr::GfHalfPi<float>());

    m_DeltaDistance = 0;
    m_DeltaYaw = 0;
    m_DeltaPitch = 0;
}

void ThirdPersonCamera::AnimateTranslation(const pxr::GfMatrix4f& viewMatrix)
{
    // If the view parameters have never been set, we can't translate
    if (m_ViewportSize[0] <= 0.f || m_ViewportSize[1] <= 0.f)
        return;

    if (m_MousePos == m_MousePosPrev)
        return;

    if (mouseButtonState[MouseButtons::Middle]) {
        pxr::GfVec4f oldClipPos(0.f, 0.f, m_Distance, 1.f);
        oldClipPos = oldClipPos * m_ProjectionMatrix;
        oldClipPos /= oldClipPos[3];
        oldClipPos[0] = 2.f * (m_MousePosPrev[0]) / m_ViewportSize[0] - 1.f;
        oldClipPos[1] = 1.f - 2.f * (m_MousePosPrev[1]) / m_ViewportSize[1];
        pxr::GfVec4f newClipPos = oldClipPos;
        newClipPos[0] = 2.f * (m_MousePos[0]) / m_ViewportSize[0] - 1.f;
        newClipPos[1] = 1.f - 2.f * (m_MousePos[1]) / m_ViewportSize[1];

        pxr::GfVec4f oldViewPos = oldClipPos * m_InverseProjectionMatrix;
        oldViewPos /= oldViewPos[3];
        pxr::GfVec4f newViewPos = newClipPos * m_InverseProjectionMatrix;
        newViewPos /= newViewPos[3];

        pxr::GfVec2f viewMotion = oldViewPos.GetVec2() - newViewPos.GetVec2();

        m_TargetPos -= viewMotion[0] * viewMatrix.GetRow3(0).GetVec3();

        if (keyboardState[KeyboardControls::HorizontalPan]) {
            pxr::GfVec3f horizontalForward = pxr::GfVec3f(viewMatrix.GetRow3(2)[0], 0.f, viewMatrix.GetRow3(2)[2]);
            float horizontalLength = horizontalForward.GetLength();
            if (horizontalLength == 0.f)
                horizontalForward = pxr::GfVec3f(viewMatrix.GetRow3(1)[0], 0.f, viewMatrix.GetRow3(1)[2]);
            horizontalForward.Normalize();
            m_TargetPos += viewMotion[1] * horizontalForward * 1.5f;
        }
        else
            m_TargetPos += viewMotion[1] * viewMatrix.GetRow3(1).GetVec3();
    }
}

void ThirdPersonCamera::Animate(float deltaT)
{
    AnimateOrbit(deltaT);

    pxr::GfQuatf orbit = pxr::GfQuatf::GetRotationFromEuler(pxr::GfVec3f(m_Pitch, m_Yaw, 0));

    const auto targetRotation = orbit.GetMatrix();
    AnimateTranslation(targetRotation);

    const pxr::GfVec3f vectorToCamera = -m_Distance * targetRotation.GetRow3(2).GetVec3();

    const pxr::GfVec3f camPos = m_TargetPos + vectorToCamera;

    m_CameraPos = camPos;
    m_CameraRight = -targetRotation.GetRow3(0).GetVec3();
    m_CameraUp = targetRotation.GetRow3(1).GetVec3();
    m_CameraDir = targetRotation.GetRow3(2).GetVec3();
    UpdateWorldToView();

    m_MousePosPrev = m_MousePos;
}

void ThirdPersonCamera::LookAt(pxr::GfVec3f cameraPos, pxr::GfVec3f cameraTarget)
{
    pxr::GfVec3f cameraDir = cameraTarget - cameraPos;

    float azimuth, elevation, dirLength;
    pxr::GfCartesianToSpherical(cameraDir, &azimuth, &elevation, &dirLength);

    SetTargetPosition(cameraTarget);
    SetDistance(dirLength);
    azimuth = -(azimuth + pxr::GfHalfPi<float>());
    SetRotation(azimuth, elevation);
}

void ThirdPersonCamera::LookTo(
    pxr::GfVec3f cameraPos,
    pxr::GfVec3f cameraDir,
    std::optional<float> targetDistance)
{
    float azimuth, elevation, dirLength;
    pxr::GfCartesianToSpherical(-cameraDir, &azimuth, &elevation, &dirLength);
    cameraDir /= dirLength;

    float const distance = targetDistance.value_or(GetDistance());
    SetTargetPosition(cameraPos + cameraDir * distance);
    SetDistance(distance);
    azimuth = -(azimuth + pxr::GfHalfPi<float>());
    SetRotation(azimuth, elevation);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE