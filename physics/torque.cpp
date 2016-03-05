#include <QtDebug>

#include <bullet/btBulletDynamicsCommon.h>

#include "physics/body.h"

#include "physics/torque.h"

using namespace Physics;

Torque::Torque(QString name, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_body(0)
{

}

void Torque::apply()
{
    if (!m_body) {
        qCritical() << "Body not set on torque:" << m_name;
        return;
    }

    calculate();

    btVector3 torque = btVector3(m_value.x(), m_value.y(), m_value.z());
    m_body->body()->applyTorque(torque);
}

QString Torque::name() const
{
    return m_name;
}

void Torque::setName(const QString &name)
{
    m_name = name;
}

Physics::Body *Torque::body() const
{
    return m_body;
}

void Torque::setBody(Physics::Body *body)
{
    m_body = body;
}

QVector3D Torque::value() const
{
    return m_value;
}

PropellorTorque::PropellorTorque(QObject *parent) :
    Torque("Propellor", parent)
{

}

void PropellorTorque::calculate()
{
    // intentionally do nothing
}

void PropellorTorque::setValue(const QVector3D &value)
{
    m_value = value;
}

SpinningDragTorque::SpinningDragTorque(QObject *parent) :
    Torque("Spinning Drag", parent)
{

}
