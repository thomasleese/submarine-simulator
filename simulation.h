#ifndef SIMULATION_H
#define SIMULATION_H

#include <Qt3DRenderer/QWindow>

namespace Qt3D {
    class QInputAspect;
    class QEntity;
}

class btDiscreteDynamicsWorld;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;

class Fluid;
class Submarine;
class ForceArrow;

class Simulation : public Qt3D::QWindow
{
    Q_OBJECT

public:
    explicit Simulation();
    ~Simulation();

    Submarine *submarine() const;
    void setSubmarine(Submarine *submarine);

    Q_PROPERTY(Submarine *submarine READ submarine WRITE setSubmarine)

public slots:
    void step();

private:
    // simulation
    Fluid *m_fluid;
    Submarine *m_submarine;
    ForceArrow *m_axisX;
    ForceArrow *m_axisY;
    ForceArrow *m_axisZ;

    // physics
    btDiscreteDynamicsWorld *m_world;
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btBroadphaseInterface* m_pairCache;
    btSequentialImpulseConstraintSolver* m_solver;

    // graphics
    Qt3D::QInputAspect *m_input;
    Qt3D::QEntity *m_rootEntity;
};

#endif // SIMULATION_H
