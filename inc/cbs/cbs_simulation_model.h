#pragma once

#include <cbs/cbs_job.h>
#include <cbs/cbs_schedule.h>
#include <simulation/simulation_model.h>

class CbsSimulationModel : public SimulationModel<CbsSchedule, CbsJob> {

};
