#ifndef FIN_H
#define FIN_H

#include <Qt3DCore/QEntity>

namespace Qt3D {
class QRotateTransform;
class QTranslateTransform;
}

class btVector3;

class Fluid;
class ForceArrow;
class Submarine;

namespace Physics {
class DragForce;
class FinDampingTorque;
class LiftForce;
}

class Fin : public Qt3D::QEntity
{
    Q_OBJECT

public:
    enum Orientation {
        North,
        East,
        South,
        West
    };

    enum Plane {
        Unknown,
        Horizontal,
        Vertical
    };

    explicit Fin(Qt3D::QEntity *scene, Qt3D::QNode *parent = 0);

    void calculatePosition(Orientation orientation, float position);

    void applyForces(const Fluid *fluid) const;
    void applyLift(const Fluid *fluid) const;
    void applyDrag(const Fluid *fluid) const;
    void applyDamping(const Fluid *fluid) const;

    Submarine *submarine() const;
    void setSubmarine(Submarine *submarine);

    Plane plane() const;

    double area() const;
    void setArea(double area);

    Physics::DragForce *drag() const;
    Physics::LiftForce *lift() const;
    Physics::FinDampingTorque *damping() const;

    Q_PROPERTY(Submarine *submarine READ submarine WRITE setSubmarine)
    Q_PROPERTY(Plane plane READ plane)
    Q_PROPERTY(double area READ area WRITE setArea)

private:
    Qt3D::QRotateTransform *m_rotateTransform;
    Qt3D::QTranslateTransform *m_translateTransform;

    Submarine *m_submarine;
    Plane m_plane;
    double m_area;
    double m_aspectRatio;

    btVector3 *m_forcePosition;

    Physics::DragForce *m_drag;
    Physics::LiftForce *m_lift;
    Physics::FinDampingTorque *m_damping;
};

#endif // FIN_H
