/*
 * SWEMergeVortex.hpp
 *
 *  Created on: 01 Nov 2017
 *      Author: Pedro Peixoto <pedrosp@ime.usp.br>
 */
#ifndef SWE_PLANE_MERGEVORTEX_HPP_
#define SWE_PLANE_MERGEVORTEX_HPP_


#include <stdlib.h>
#include <sweet/sweetmath.hpp>
#include <sweet/SimulationVariables.hpp>
#include <sweet/plane/PlaneData.hpp>


/**
 * Implement merging vortex initial conditions
 * See Energy- and enstrophy-conserving schemes for the shallow-water
 * equations, based on mimetic finite elements
 * Andrew T. T. McRae and Colin J. Cotter
 *
 */
class SWEMergeVortex
{
	SimulationVariables &simVars;

	PlaneOperators &op;

	double f = simVars.sim.f0;
	double g = simVars.sim.gravitation;
	double sx = simVars.sim.domain_size[0];
	double sy = simVars.sim.domain_size[1];

	double stream(
			double x,
			double y
			)
	{

		double radius = simVars.setup.radius_scale*sqrt((double)sx*(double)sx+(double)sy*(double)sy);
		double factor = 500.0;

		// Gaussian Vortice 1
		double dx = x-0.45*sx;
		double dy = y-0.5*sy;

		dx /= radius;
		dy /= radius;

		double exp1 = std::exp(-factor*(dx*dx + dy*dy));

		// Gaussian Vortice 2
		dx = x-0.55*sx;
		dy = y-0.5*sy;

		dx /= radius;
		dy /= radius;

		double exp2 = std::exp(-factor*(dx*dx + dy*dy));

		return exp1+exp2;

	}

	void setup_stream(
			PlaneData &o_psi
	)
	{

		for (int j = 0; j < simVars.disc.res_physical[1]; j++)
		{
			for (int i = 0; i < simVars.disc.res_physical[0]; i++)
			{

				// h - lives in the center of the cell
				double x = (((double)i+0.5)/(double)simVars.disc.res_physical[0])*simVars.sim.domain_size[0];
				double y = (((double)j+0.5)/(double)simVars.disc.res_physical[1])*simVars.sim.domain_size[1];

				o_psi.p_physical_set(j, i, stream(x, y));
			}
		}

	}


public:
	SWEMergeVortex(
		SimulationVariables &io_simVars,
		PlaneOperators &io_op
	)	:
		simVars(io_simVars),
		op(io_op)
	{
	}

	void setup(
			PlaneData &o_h,
			PlaneData &o_u,
			PlaneData &o_v
	)
	{

		PlaneData psi(o_h.planeDataConfig);

		/*
		 * Prepare laplace operator
		 */
		PlaneData laplace = op.diff2_c_x + op.diff2_c_y;


		/*
		 * Setup stream function
		 */
		setup_stream(psi);
		//psi.file_physical_saveData_ascii("ouput_stream");

		//Calculate Velocities
		o_u = op.diff_c_y(psi);
		o_v = -op.diff_c_x(psi);

		//Calculate vorticity
		PlaneData vort = op.vort(o_u, o_v);

		//Solve Poisson equation for height to get balance initial condition
		PlaneData lap_h = (f/g)*vort;
		o_h = lap_h.spectral_div_element_wise(laplace);

	}


};


#endif
