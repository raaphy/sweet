/*
 * SWE_Sphere_TS_l_irk_na_erk.hpp
 *
 *  Created on: 30 May 2017
 *      Author: Martin Schreiber <SchreiberX@gmail.com>
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_L_IRK_NA_ERK_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_L_IRK_NA_ERK_HPP_

#include <sweet/sphere/SphereData_Spectral.hpp>
#include <sweet/sphere/SphereOperators_SphereData.hpp>
#include <sweet/sphere/SphereTimestepping_ExplicitRK.hpp>
#include <limits>
#include <sweet/SimulationVariables.hpp>

#include "SWE_Sphere_TS_interface.hpp"
#include "SWE_Sphere_TS_l_irk.hpp"
#include "SWE_Sphere_TS_ln_erk_split_vd.hpp"



class SWE_Sphere_TS_l_irk_na_erk_vd	: public SWE_Sphere_TS_interface

{
public:
	bool implements_timestepping_method(const std::string &i_timestepping_method)
	{
		if (
			i_timestepping_method == "l_irk_na_erk_vd" || i_timestepping_method == "l_irk_na_erk_vd_ver0" ||
			i_timestepping_method == "l_cn_na_erk_vd" || i_timestepping_method == "l_cn_na_erk_vd_ver0"		||
			i_timestepping_method == "l_irk_na_erk_vd_ver1"
		)
			return true;

		return false;
	}

public:
	std::string string_id()
	{
		std::string s = "l_irk_n_erk_ver";

		if (version_id == 0)
			s += "0";
		else if (version_id == 1)
			s += "1";
		else
			SWEETError("Version ID");

		return s;
	}

	SimulationVariables &simVars;
	SphereOperators_SphereData &op;

	double timestep_size;

	/*
	 * Linear time steppers
	 */
	SWE_Sphere_TS_l_irk timestepping_l_irk;

	/*
	 * Non-linear time steppers
	 */
	SWE_Sphere_TS_ln_erk_split_vd timestepping_ln_erk_split_vd;

	SphereTimestepping_ExplicitRK timestepping_rk_nonlinear;

	int version_id;

	int timestepping_order;
	int timestepping_order2;


public:
	SWE_Sphere_TS_l_irk_na_erk_vd(
			SimulationVariables &i_simVars,
			SphereOperators_SphereData &i_op
		);

	void setup(
			int i_order,	///< order of RK time stepping method for linear parts
			int i_order2,	///< order of RK time stepping method for non-linear parts
			int i_version_id
	);

	void setup_auto();

	void run_timestep_pert(
			SphereData_Spectral &io_phi_pert,	///< prognostic variables
			SphereData_Spectral &io_vort,	///< prognostic variables
			SphereData_Spectral &io_div,	///< prognostic variables

			double i_fixed_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);



	virtual ~SWE_Sphere_TS_l_irk_na_erk_vd();
};

#endif /* SRC_PROGRAMS_SWE_PLANE_REXI_SWE_PLANE_TS_LN_ERK_HPP_ */
