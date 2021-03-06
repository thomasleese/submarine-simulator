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

public slots:
    void step();
    void reset();

public:
    Fluid *fluid() const;
    void setFluid(Fluid *fluid);

    Submarine *submarine() const;
    void setSubmarine(Submarine *submarine);

    int frame() const;
    double time() const;

    Q_PROPERTY(Fluid *fluid READ fluid WRITE setFluid)
    Q_PROPERTY(Submarine *submarine READ submarine WRITE setSubmarine)
    Q_PROPERTY(int frame READ frame)
    Q_PROPERTY(double time READ time STORED false)

private:
    // simulation
    Fluid *m_fluid;
    Submarine *m_submarine;
    ForceArrow *m_axisX;
    ForceArrow *m_axisY;
    ForceArrow *m_axisZ;

    int m_frame;

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
