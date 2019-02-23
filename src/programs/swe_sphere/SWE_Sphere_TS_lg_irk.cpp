/*
 * SWE_Sphere_TS_lg_irk.hpp
 *
 *  Created on: 30 Aug 2016
 *      Author: Martin Schreiber <SchreiberX@gmail.com>
 */


#include "SWE_Sphere_TS_lg_irk.hpp"
#include <complex>
#include <sweet/sphere/SphereDataConfig.hpp>
#include <sweet/sphere/SphereOperators.hpp>
#include <sweet/sphere/app_swe/SWESphBandedMatrixPhysicalReal.hpp>


SWE_Sphere_TS_lg_irk::SWE_Sphere_TS_lg_irk(
		SimulationVariables &i_simVars,
		SphereOperators &i_op
)	:
	simVars(i_simVars),
	op(i_op),
	sphereDataConfig(op.sphereDataConfig)
{
}


/**
 * Setup the SWE REXI solver with SPH
 */
void SWE_Sphere_TS_lg_irk::setup(
		int i_timestep_order,
		double i_timestep_size,
		int i_extended_modes
)
{
#if SWEET_DEBUG
	if (i_extended_modes != 0)
		FatalError("Not supported");
#endif

	if (i_timestep_order != 1)
		FatalError("Only 1st order IRK supported so far!");

	timestep_size = i_timestep_size;

	alpha = -1.0/timestep_size;
	beta = -1.0/timestep_size;

	r = simVars.sim.sphere_radius;
	inv_r = 1.0/r;

	gh = simVars.sim.gravitation*simVars.sim.h0;
}



/**
 * Solve a REXI time step for the given initial conditions
 */

void SWE_Sphere_TS_lg_irk::run_timestep(
		SphereDataSpectral &io_phi,		///< prognostic variables
		SphereDataSpectral &io_vort,	///< prognostic variables
		SphereDataSpectral &io_div,		///< prognostic variables

		double i_fixed_dt,			///< if this value is not equal to 0, use this time step size instead of computing one
		double i_simulation_timestamp
)
{
	if (i_fixed_dt <= 0)
		FatalError("Only constant time step size allowed");

	if (std::abs(timestep_size - i_fixed_dt)/std::max(timestep_size, i_fixed_dt) > 1e-10)
	{
		std::cout << "Warning: Reducing time step size from " << i_fixed_dt << " to " << timestep_size << std::endl;

		timestep_size = i_fixed_dt;

		alpha = -1.0/timestep_size;
		beta = -1.0/timestep_size;
	}

	SphereDataSpectral phi0 = io_phi;
	SphereDataSpectral vort0 = io_vort;
	SphereDataSpectral div0 = io_div;

	SphereDataSpectral phi(sphereDataConfig);
	SphereDataSpectral vort(sphereDataConfig);
	SphereDataSpectral div(sphereDataConfig);

	{
		SphereDataSpectral rhs = gh*div0 + alpha*phi0;
		phi = rhs.spectral_solve_helmholtz(alpha*alpha, -gh, r);
		io_phi = phi*beta;

		rhs = alpha*div0 + op.laplace(phi0);
		div = rhs.spectral_solve_helmholtz(alpha*alpha, -gh, r);
		io_div = div*beta;

		io_vort = vort0;
	}
}


SWE_Sphere_TS_lg_irk::~SWE_Sphere_TS_lg_irk()
{
}
