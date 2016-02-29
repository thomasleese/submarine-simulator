#include <cmath>

#include <bullet/btBulletDynamicsCommon.h>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QCamera>
#include <Qt3DCore/QCameraLens>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QLookAtTransform>
#include <Qt3DCore/QScaleTransform>
#include <Qt3DCore/QRotateTransform>
#include <Qt3DCore/QTranslateTransform>
#include <Qt3DCore/QAspectEngine>

#include <Qt3DRenderer/QPhongMaterial>
#include <Qt3DRenderer/QSphereMesh>
#include <Qt3DRenderer/QTorusMesh>

#include <Qt3DInput/QInputAspect>

#include <Qt3DRenderer/QRenderAspect>
#include <Qt3DRenderer/QFrameGraph>
#include <Qt3DRenderer/QForwardRenderer>
#include <Qt3DRenderer/QPhongMaterial>

#include <Qt3DRenderer/QCylinderMesh>
#include <Qt3DRenderer/QSphereMesh>
#include <Qt3DRenderer/QTorusMesh>
#include <Qt3DRenderer/QWindow>

#include <QVector2D>

#include <QtMath>
#include <QtDebug>

#include "fluid.h"
#include "forcearrow.h"

#include "submarine.h"

btVector3 qtVector2btVector(QVector3D v) {
    return btVector3(v.x(), v.y(), v.z());
}

float wrapAngle(float angle) {
    while (angle > M_PI) {
        angle -= M_PI * 2;
    }

    while (angle < -M_PI) {
        angle += M_PI * 2;
    }

    return angle;
}

Submarine::Submarine(Fluid *fluid, QObject *parent) :
    QObject(parent),
    m_fluid(fluid),
    m_shape(0),
    m_body(0),
    m_entity(0),
    m_length(0),
    m_width(0),
    m_height(0),
    m_mass(0),
    m_dragCoefficient(0),
    m_liftCoefficientSlope(0),
    m_spinningDragCoefficient(0),
    m_hasHorizontalFins(false),
    m_horizontalFinsArea(0),
    m_horizontalFinsLiftCoefficientSlope(0),
    m_horizontalFinsDragCoefficient(0),
    m_horizontalFinsPosition(0),
    m_horizontalFinsAspectRatio(0),
    m_hasVerticalFins(false),
    m_verticalFinsArea(0),
    m_verticalFinsLiftCoefficientSlope(0),
    m_verticalFinsDragCoefficient(0),
    m_verticalFinsPosition(0),
    m_verticalFinsAspectRatio(0)
{

}

Submarine::~Submarine() {
    if (m_body) {
        delete m_body;
    }

    if (m_shape) {
        delete m_shape;
    }
}

double Submarine::crossSectionalArea() const {
    return M_PI * m_width * m_height;
}

double Submarine::pitch() const {
    return m_body->getOrientation().z();

    /*btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);

    btMatrix3x3 basis = transform.getBasis();

    float yaw, pitch, roll;
    // yes, those do appear to be in the wrong order, but it is correct
    basis.getEulerYPR(pitch, yaw, roll);

    return wrapAngle(pitch);*/
}

double Submarine::yaw() const {
    return m_body->getOrientation().y();

    /*btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);

    btMatrix3x3 basis = transform.getBasis();

    float yaw, pitch, roll;
    // yes, those do appear to be in the wrong order, but it is correct
    basis.getEulerYPR(pitch, yaw, roll);

    return wrapAngle(yaw);*/
}

double Submarine::roll() const {
    return m_body->getOrientation().x();

    /*btTransform transform;
    m_body->getMotionState()->getWorldTransform(transform);

    btMatrix3x3 basis = transform.getBasis();

    float yaw, pitch, roll;
    // yes, those do appear to be in the wrong order, but it is correct
    basis.getEulerYPR(pitch, yaw, roll);

    return wrapAngle(roll);*/
}

QVector2D Submarine::pitchVelocity() const {
    btVector3 velocity = m_body->getLinearVelocity();
    return QVector2D(velocity.x(), velocity.y());
}

QVector2D Submarine::yawVelocity() const {
    btVector3 velocity = m_body->getLinearVelocity();
    return QVector2D(velocity.x(), velocity.z());
}

double Submarine::pitchAngleOfAttack() const {
    btVector3 velocity = m_body->getLinearVelocity();
    float velocityAngle = wrapAngle(atan2(velocity.y(), velocity.x()));
    return wrapAngle(velocityAngle - pitch());
}

double Submarine::yawAngleOfAttack() const {
    btVector3 velocity = m_body->getLinearVelocity();
    float velocityAngle = wrapAngle(atan2(velocity.z(), velocity.x()));
    return wrapAngle(velocityAngle - yaw());
}

QVector3D Submarine::angularVelocity() const {
    btVector3 v = m_body->getAngularVelocity();
    return QVector3D(v.x(), v.y(), v.z());
}

QVector3D Submarine::position() const {
    btVector3 p = m_body->getCenterOfMassPosition();
    return QVector3D(p.x(), p.y(), p.z());
}

void Submarine::addToWorld(btDynamicsWorld *world) {
    if (m_body || m_shape) {
        qFatal("Already added to the world.");
    }

    double radius = (m_width + m_height) / 2.f;
    m_shape = new btCapsuleShapeX(radius, m_length);

    btVector3 localInertia;
    m_shape->calculateLocalInertia(m_mass, localInertia);

    auto info = btRigidBody::btRigidBodyConstructionInfo(m_mass, 0, m_shape,
                                                         localInertia);

    m_body = new btRigidBody(info);

    m_body->setSleepingThresholds(0, 0);

    auto motionState = new btDefaultMotionState();
    m_body->setMotionState(motionState);

    m_body->applyTorqueImpulse(btVector3(40, 0, 0));

    world->addRigidBody(m_body);
}

void Submarine::addToScene(Qt3D::QEntity *scene) {
    if (m_entity) {
        qFatal("Already added to the scene.");
    }

    m_entity = new Qt3D::QEntity(scene);

    auto material = new Qt3D::QPhongMaterial(scene);
    material->setAmbient(QColor(50, 50, 50));
    m_entity->addComponent(material);

    auto mesh = new Qt3D::QSphereMesh;
    mesh->setRadius(0.5);
    mesh->setRings(24);
    mesh->setSlices(48);
    m_entity->addComponent(mesh);

    Qt3D::QTransform *transform = new Qt3D::QTransform;

    m_translateTransform = new Qt3D::QTranslateTransform;
    m_translateTransform->setTranslation(QVector3D(0, 0, 0));

    Qt3D::QScaleTransform *scaleTransform = new Qt3D::QScaleTransform;
    scaleTransform->setScale3D(QVector3D(m_length, m_height, m_width));

    m_rotateTransform = new Qt3D::QRotateTransform;
    m_rotateTransform->setAxis(QVector3D(0, 1, 0));
    m_rotateTransform->setAngleDeg(0);

    transform->addTransform(scaleTransform);
    transform->addTransform(m_rotateTransform);
    transform->addTransform(m_translateTransform);

    m_entity->addComponent(transform);

    m_forceNoise = new ForceArrow(Qt::red, 1.f);
    m_forceNoise->addToScene(scene);

    m_forceWeight = new ForceArrow(Qt::green, 0.5f);
    m_forceWeight->addToScene(scene);

    m_forceBuoyancy = new ForceArrow(Qt::blue, 0.5f);
    m_forceBuoyancy->addToScene(scene);

    m_forceThrust = new ForceArrow(Qt::black, 5.f);
    m_forceThrust->addToScene(scene);

    m_forceDrag = new ForceArrow(Qt::white, 5.f);
    m_forceDrag->addToScene(scene);

    m_forcePitchLift = new ForceArrow(Qt::magenta, 1.f);
    m_forcePitchLift->addToScene(scene);

    m_forceYawLift = new ForceArrow(Qt::yellow, 1.f);
    m_forceYawLift->addToScene(scene);

    m_forceHorizontalFinsLift = new ForceArrow(Qt::darkCyan, 1.f);
    m_forceHorizontalFinsLift->addToScene(scene);

    m_forceVerticalFinsLift = new ForceArrow(Qt::darkCyan, 1.f);
    m_forceVerticalFinsLift->addToScene(scene);

    m_forceHorizontalFinsDrag = new ForceArrow(Qt::cyan, 100.f);
    m_forceHorizontalFinsDrag->addToScene(scene);

    m_forceVerticalFinsDrag = new ForceArrow(Qt::cyan, 100.f);
    m_forceVerticalFinsDrag->addToScene(scene);
}

void Submarine::update(Qt3D::QCamera *camera) {
    btMotionState *motionState = m_body->getMotionState();

    btTransform transform;
    motionState->getWorldTransform(transform);

    btVector3 t = transform.getOrigin();
    QVector3D position(t.x(), t.y(), t.z());
    m_translateTransform->setTranslation(position);

    btQuaternion q = transform.getRotation();
    btVector3 a = q.getAxis();
    m_rotateTransform->setAxis(QVector3D(a.x(), a.y(), a.z()));
    m_rotateTransform->setAngleRad(q.getAngle());

    qDebug() << position;

    camera->lookAt()->setPosition(position + QVector3D(2, 4, 10));
    camera->lookAt()->setViewCenter(position);

    applyWeight();
    applyBuoyancy();
    applyThrust();
    applyDrag();
    //applyLift();
    //applySpinningDrag();
    //applyFinsLift();
    //applyFinsDrag();
    //applyFinsDamping();
}

double Submarine::length() const
{
    return m_length;
}

void Submarine::setLength(double length)
{
    m_length = length;
    m_liftPosition = QVector3D(length / 2., 0, 0);
}

double Submarine::width() const
{
    return m_width;
}

void Submarine::setWidth(double width)
{
    m_width = width;
}

double Submarine::height() const
{
    return m_height;
}

void Submarine::setHeight(double height)
{
    m_height = height;
}

double Submarine::mass() const
{
    return m_mass;
}

void Submarine::setMass(double mass)
{
    m_mass = mass;
}

double Submarine::dragCoefficient() const
{
    return m_dragCoefficient;
}

void Submarine::setDragCoefficient(double dragCoefficient)
{
    m_dragCoefficient = dragCoefficient;
}

double Submarine::liftCoefficientSlope() const
{
    return m_liftCoefficientSlope;
}

void Submarine::setLiftCoefficientSlope(double liftCoefficientSlope)
{
    m_liftCoefficientSlope = liftCoefficientSlope;
}

double Submarine::spinningDragCoefficient() const
{
    return m_spinningDragCoefficient;
}

void Submarine::setSpinningDragCoefficient(double spinningDragCoefficient)
{
    m_spinningDragCoefficient = spinningDragCoefficient;
}

QVector3D Submarine::buoyancyPosition() const
{
    return m_buoyancyPosition;
}

void Submarine::setBuoyancyPosition(const QVector3D &buoyancyPosition)
{
    m_buoyancyPosition = buoyancyPosition;
}

QVector3D Submarine::weightPosition() const
{
    return m_weightPosition;
}

void Submarine::setWeightPosition(const QVector3D &weightPosition)
{
    m_weightPosition = weightPosition;
}

double Submarine::hasHorizontalFins() const
{
    return m_hasHorizontalFins;
}

void Submarine::setHasHorizontalFins(double hasHorizontalFins)
{
    m_hasHorizontalFins = hasHorizontalFins;
}

double Submarine::horizontalFinsArea() const
{
    return m_horizontalFinsArea;
}

void Submarine::setHorizontalFinsArea(double horizontalFinsArea)
{
    m_horizontalFinsArea = horizontalFinsArea;
}

double Submarine::horizontalFinsLiftCoefficientSlope() const
{
    return m_horizontalFinsLiftCoefficientSlope;
}

void Submarine::setHorizontalFinsLiftCoefficientSlope(double horizontalFinsLiftCoefficientSlope)
{
    m_horizontalFinsLiftCoefficientSlope = horizontalFinsLiftCoefficientSlope;
}

double Submarine::horizontalFinsDragCoefficient() const
{
    return m_horizontalFinsDragCoefficient;
}

void Submarine::setHorizontalFinsDragCoefficient(double horizontalFinsDragCoefficient)
{
    m_horizontalFinsDragCoefficient = horizontalFinsDragCoefficient;
}

double Submarine::horizontalFinsPosition() const
{
    return m_horizontalFinsPosition;
}

void Submarine::setHorizontalFinsPosition(double horizontalFinsPosition)
{
    m_horizontalFinsPosition = horizontalFinsPosition;
}

double Submarine::horizontalFinsAspectRatio() const
{
    return m_horizontalFinsAspectRatio;
}

void Submarine::setHorizontalFinsAspectRatio(double horizontalFinsAspectRatio)
{
    m_horizontalFinsAspectRatio = horizontalFinsAspectRatio;
}

double Submarine::hasVerticalFins() const
{
    return m_hasVerticalFins;
}

void Submarine::setHasVerticalFins(double hasVerticalFins)
{
    m_hasVerticalFins = hasVerticalFins;
}

double Submarine::verticalFinsArea() const
{
    return m_verticalFinsArea;
}

void Submarine::setVerticalFinsArea(double verticalFinsArea)
{
    m_verticalFinsArea = verticalFinsArea;
}

double Submarine::verticalFinsLiftCoefficientSlope() const
{
    return m_verticalFinsLiftCoefficientSlope;
}

void Submarine::setVerticalFinsLiftCoefficientSlope(double verticalFinsLiftCoefficientSlope)
{
    m_verticalFinsLiftCoefficientSlope = verticalFinsLiftCoefficientSlope;
}

double Submarine::verticalFinsDragCoefficient() const
{
    return m_verticalFinsDragCoefficient;
}

void Submarine::setVerticalFinsDragCoefficient(double verticalFinsDragCoefficient)
{
    m_verticalFinsDragCoefficient = verticalFinsDragCoefficient;
}

double Submarine::verticalFinsPosition() const
{
    return m_verticalFinsPosition;
}

void Submarine::setVerticalFinsPosition(double verticalFinsPosition)
{
    m_verticalFinsPosition = verticalFinsPosition;
}

double Submarine::verticalFinsAspectRatio() const
{
    return m_verticalFinsAspectRatio;
}

void Submarine::setVerticalFinsAspectRatio(double verticalFinsAspectRatio)
{
    m_verticalFinsAspectRatio = verticalFinsAspectRatio;
}

void Submarine::applyWeight() {
    btVector3 force(0, -9.81 * m_mass, 0);

    btVector3 position = m_body->getCenterOfMassTransform() * qtVector2btVector(m_weightPosition);

    m_body->applyForce(force, position);
    m_forceWeight->update(force, position);
}

void Submarine::applyBuoyancy() {
    btVector3 force(0, 9.81 * m_mass, 0);

    btVector3 position = m_body->getCenterOfMassTransform() * qtVector2btVector(m_buoyancyPosition);

    m_body->applyForce(force, position);
    m_forceBuoyancy->update(force, position);
}

void Submarine::applyThrust() {
    btVector3 force(100, 0, 5);
    force = force.rotate(m_body->getCenterOfMassTransform().getRotation().getAxis(), m_body->getCenterOfMassTransform().getRotation().getAngle());

    btVector3 position(-m_length / 2, 0, 0);
    position = m_body->getCenterOfMassTransform() * position;

    m_body->applyForce(force, position);
    m_forceThrust->update(force, position);
}

void Submarine::applyDrag() {
    btVector3 velocity = m_body->getLinearVelocity();
    if (velocity.length() == 0) {
        return;  // can't be normalised
    }

    float value = 0.5f * m_fluid->density() * crossSectionalArea() * m_dragCoefficient * velocity.length2();
    btVector3 force = velocity.normalized() * -value;

    btVector3 position = m_body->getCenterOfMassPosition();

    m_body->applyForce(force, position);
    m_forceDrag->update(force, position);
}

QVector2D calculateLift(float fluidDensity, float angleOfAttack, float liftCoefficientSlope, float area, QVector2D velocity) {
    float liftCoefficient = angleOfAttack * liftCoefficientSlope;

    float value = 0.5f * fluidDensity * area * liftCoefficient * velocity.lengthSquared();

    QVector2D result = velocity.normalized();

    // rotate90(1)
    float x = result.x();
    result.setX(-result.y());
    result.setY(x);

    return result * value;
}

void Submarine::applyPitchLift() {
    QVector2D velocity = pitchVelocity();
    float angleOfAttack = pitchAngleOfAttack();

    if (qAbs(angleOfAttack) < qDegreesToRadians(15.)) {
        QVector2D lift = calculateLift(m_fluid->density(), angleOfAttack, m_liftCoefficientSlope, crossSectionalArea(), velocity);

        btVector3 position = m_body->getCenterOfMassTransform() * qtVector2btVector(m_liftPosition);

        btVector3 force(lift.x(), lift.y(), 0);

        m_body->applyForce(force, position);
        m_forcePitchLift->update(force, position);
    }
}

void Submarine::applyYawLift() {
    QVector2D velocity = yawVelocity();
    float angleOfAttack = yawAngleOfAttack();

    if (qAbs(angleOfAttack) < qDegreesToRadians(15.)) {
        QVector2D lift = calculateLift(m_fluid->density(), angleOfAttack, m_liftCoefficientSlope, crossSectionalArea(), velocity);

        btVector3 position = m_body->getCenterOfMassTransform() * qtVector2btVector(m_liftPosition);

        btVector3 force(lift.x(), 0, lift.y());

        m_body->applyForce(force, position);
        m_forcePitchLift->update(force, position);
    }
}

void Submarine::applyLift() {
    applyPitchLift();
    applyYawLift();
}

float calculateSpinningDrag(float fluidDensity, float area, float spinningDragCoefficient, float angularVelocity, float length) {
    float v2 = angularVelocity * angularVelocity;
    float value = (0.5f * fluidDensity * area * spinningDragCoefficient * v2) / length;

    float drag = -value;
    if (angularVelocity < 0) {
        drag = value;
    }

    return drag;
}

void Submarine::applyPitchSpinningDrag() {
    float angularVelocity = m_body->getAngularVelocity().z();
    float drag = calculateSpinningDrag(m_fluid->density(), crossSectionalArea(), m_spinningDragCoefficient, angularVelocity, m_length);
    m_body->applyTorque(btVector3(0, 0, drag));
}

void Submarine::applyYawSpinningDrag() {
    float angularVelocity = m_body->getAngularVelocity().y();
    float drag = calculateSpinningDrag(m_fluid->density(), crossSectionalArea(), m_spinningDragCoefficient, angularVelocity, m_length);
    m_body->applyTorque(btVector3(0, 0, drag));
}

void Submarine::applySpinningDrag() {
    applyPitchSpinningDrag();
    applyYawSpinningDrag();
}

float getEllipseProportion(float proportion) {
    return qSqrt(1.f - (proportion / 0.5f) * (proportion / 0.5f));
}

void Submarine::applyHorizontalFinsLift() {
    QVector2D velocity = pitchVelocity();
    float angleOfAttack = pitchAngleOfAttack();

    if (qAbs(angleOfAttack) < qDegreesToRadians(15.)) {
        QVector2D lift = calculateLift(m_fluid->density(), angleOfAttack, m_horizontalFinsLiftCoefficientSlope, m_horizontalFinsArea, velocity);

        btVector3 position = m_body->getCenterOfMassTransform() * btVector3(m_horizontalFinsPosition, 0, 0);

        btVector3 force(lift.x(), lift.y(), 0);

        m_body->applyForce(force, position);
        m_forceHorizontalFinsLift->update(force, position);
    }
}

void Submarine::applyVerticalFinsLift() {
    QVector2D velocity = yawVelocity();
    float angleOfAttack = yawAngleOfAttack();

    if (qAbs(angleOfAttack) < qDegreesToRadians(15.)) {
        QVector2D lift = calculateLift(m_fluid->density(), angleOfAttack, m_verticalFinsLiftCoefficientSlope, m_verticalFinsArea, velocity);

        btVector3 position = m_body->getCenterOfMassTransform() * btVector3(m_verticalFinsPosition, 0, 0);

        btVector3 force(lift.x(), lift.y(), 0);

        m_body->applyForce(force, position);
        m_forceVerticalFinsLift->update(force, position);
    }
}

void Submarine::applyFinsLift() {
    if (m_hasHorizontalFins) {
        applyHorizontalFinsLift();
    }

    if (m_hasVerticalFins) {
        applyVerticalFinsLift();
    }
}

void Submarine::applyHorizontalFinsDrag() {
    QVector2D velocity = pitchVelocity();

    float v2 = velocity.lengthSquared();
    float value = 0.5f * m_fluid->density() * m_horizontalFinsArea * m_horizontalFinsDragCoefficient * v2;

    QVector2D drag = velocity.normalized() * -value;

    btVector3 position = m_body->getCenterOfMassTransform() * btVector3(m_horizontalFinsPosition, 0, 0);

    btVector3 force(drag.x(), drag.y(), 0);

    m_body->applyForce(force, position);
    m_forceHorizontalFinsDrag->update(force, position);
}

void Submarine::applyVerticalFinsDrag() {
    QVector2D velocity = yawVelocity();

    float v2 = velocity.lengthSquared();
    float value = 0.5f * m_fluid->density() * m_verticalFinsArea * m_verticalFinsDragCoefficient * v2;

    QVector2D drag = velocity.normalized() * -value;

    btVector3 position = m_body->getCenterOfMassTransform() * btVector3(m_verticalFinsPosition, 0, 0);

    btVector3 force(drag.x(), 0, drag.y());

    m_body->applyForce(force, position);
    m_forceVerticalFinsDrag->update(force, position);
}

void Submarine::applyFinsDrag() {
    if (m_hasHorizontalFins) {
        applyHorizontalFinsDrag();
    }

    if (m_hasVerticalFins) {
        applyVerticalFinsDrag();
    }
}

void Submarine::applyHorizontalFinsDamping() {
    float span = qSqrt(m_horizontalFinsAspectRatio * m_horizontalFinsArea);

    float radius = getEllipseProportion(m_horizontalFinsPosition) * m_width / 2.f;
    float v2 = m_body->getAngularVelocity().x() * m_body->getAngularVelocity().x();
    float torque = -2.f * m_fluid->density() * m_horizontalFinsArea * v2 * (radius + span) * (radius + span) * (radius + span / 2.f);

    m_body->applyTorque(btVector3(torque, 0, 0));
}

void Submarine::applyVerticalFinsDamping() {
    float span = qSqrt(m_verticalFinsAspectRatio * m_verticalFinsArea);

    float radius = getEllipseProportion(m_verticalFinsPosition) * m_width / 2.f;
    float v2 = m_body->getAngularVelocity().x() * m_body->getAngularVelocity().x();
    float torque = -2.f * m_fluid->density() * m_verticalFinsArea * v2 * (radius + span) * (radius + span) * (radius + span / 2.f);

    m_body->applyTorque(btVector3(torque, 0, 0));
}

void Submarine::applyFinsDamping() {
    if (m_hasHorizontalFins) {
        applyHorizontalFinsDamping();
    }

    if (m_hasVerticalFins) {
        applyVerticalFinsDamping();
    }
}
