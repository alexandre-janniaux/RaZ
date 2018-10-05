#include "RaZ/Math/Transform.hpp"
#include "RaZ/Render/Camera.hpp"
#include "RaZ/Render/Light.hpp"
#include "RaZ/Render/Mesh.hpp"
#include "RaZ/Render/RenderSystem.hpp"

namespace Raz {

RenderSystem::RenderSystem(unsigned int windowWidth, unsigned int windowHeight,
                           const std::string& windowTitle) : m_window(windowWidth, windowHeight, windowTitle) {
  m_camera.addComponent<Camera>(windowWidth, windowHeight);
  m_camera.addComponent<Transform>();

  m_acceptedComponents.setBit(Component::getId<Mesh>());
  m_acceptedComponents.setBit(Component::getId<Light>());
}

void RenderSystem::linkEntity(const EntityPtr& entity) {
  System::linkEntity(entity);

  if (entity->hasComponent<Mesh>())
    entity->getComponent<Mesh>().load(m_program);

  if (entity->hasComponent<Light>())
    updateLights();
}

void RenderSystem::update(float deltaTime) {
  auto& camera       = m_camera.getComponent<Camera>();
  auto& camTransform = m_camera.getComponent<Transform>();

  Mat4f viewProjMat;

  if (camTransform.hasUpdated()) {
    const Mat4f& viewMat = camera.computeViewMatrix(camTransform.getRotation().inverse(),
                                                    camTransform.computeTranslationMatrix(true));
    camera.computeInverseViewMatrix();
    viewProjMat = camera.getProjectionMatrix() * viewMat;

    sendCameraMatrices(viewProjMat);

    camTransform.setUpdated(false);
  } else {
    viewProjMat = camera.getProjectionMatrix() * camera.getViewMatrix();
  }

  for (auto& entity : m_entities) {
    if (entity->isEnabled()) {
      if (entity->hasComponent<Mesh>() && entity->hasComponent<Transform>()) {
        const auto modelMat = entity->getComponent<Transform>().computeTransformMatrix();

        m_program.sendUniform("uniModelMatrix", modelMat);
        m_program.sendUniform("uniMvpMatrix", viewProjMat * modelMat);

        entity->getComponent<Mesh>().draw(m_program);
      }
    }
  }

  m_window.run(deltaTime);
}

void RenderSystem::sendCameraMatrices(const Mat4f& viewProjMat) const {
  const auto& camera   = m_camera.getComponent<Camera>();
  const auto& camTrans = m_camera.getComponent<Transform>();
  const Vec3f& camPos  = camTrans.getPosition();

  m_cameraUbo.bind();
  sendViewMatrix(camera.getViewMatrix());
  sendInverseViewMatrix(camera.getInverseViewMatrix());
  sendProjectionMatrix(camera.getProjectionMatrix());
  sendInverseProjectionMatrix(camera.getInverseProjectionMatrix());
  sendViewProjectionMatrix(viewProjMat);
  sendCameraPosition(camPos);

  m_program.sendUniform("uniCameraPos", camPos);
}

void RenderSystem::sendCameraMatrices() const {
  const auto& camera = m_camera.getComponent<Camera>();
  sendCameraMatrices(camera.getProjectionMatrix() * camera.getViewMatrix());
}

void RenderSystem::updateLight(const Entity* entity, std::size_t lightIndex) const {
  m_program.use();

  const std::string strBase = "uniLights[" + std::to_string(lightIndex) + "].";

  const std::string posStr    = strBase + "position";
  const std::string colorStr  = strBase + "color";
  const std::string energyStr = strBase + "energy";
  const std::string angleStr  = strBase + "angle";

  const auto& lightComp = entity->getComponent<Light>();
  Vec4f homogeneousPos(entity->getComponent<Transform>().getPosition(), 1.f);

  if (lightComp.getType() == LightType::DIRECTIONAL) {
    homogeneousPos[3] = 0.f;
    m_program.sendUniform(strBase + "direction", lightComp.getDirection());
  }

  m_program.sendUniform(posStr,    homogeneousPos);
  m_program.sendUniform(colorStr,  lightComp.getColor());
  m_program.sendUniform(energyStr, lightComp.getEnergy());
  m_program.sendUniform(angleStr,  lightComp.getAngle());
}

void RenderSystem::updateLights() const {
  std::size_t lightCount = 0;

  for (const auto& entity : m_entities) {
    if (entity->hasComponent<Light>()) {
      updateLight(entity, lightCount);
      ++lightCount;
    }
  }

  m_program.sendUniform("uniLightCount", lightCount);
}

void RenderSystem::updateShaders() const {
  m_program.updateShaders();
  sendCameraMatrices();
  updateLights();

  for (auto& entity : m_entities) {
    if (entity->hasComponent<Mesh>())
      entity->getComponent<Mesh>().load(m_program);
  }
}

} // namespace Raz
