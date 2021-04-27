#pragma once

class SimulationAction {
  public:
    int _weight = 0;
    bool _success = true;
    SimulationAction(int weight = 0)
        : _weight(weight) {}
    virtual ~SimulationAction() = default;
    virtual int time() const = 0;
    virtual void execute() = 0;
};
