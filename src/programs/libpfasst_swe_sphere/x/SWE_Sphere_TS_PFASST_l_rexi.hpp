/*
 * SWE_Sphere_REXI.hpp
 *
 *  Created on: 25 Oct 2016
 *      Author: Martin Schreiber <SchreiberX@gmail.com>
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_PFASST_L_REXI_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_PFASST_L_REXI_HPP_


#ifndef SWEET_MPI
#define SWEET_MPI 1
#endif


#include <complex>
#include <rexi/REXI_Terry.hpp>
#include <sweet/SimulationVariables.hpp>
#include <string.h>
#include <sweet/sphere/SphereData_Config.hpp>
#include <sweet/sphere/SphereData_Spectral.hpp>
#include <sweet/sphere/SphereData_SpectralComplex.hpp>
#include <sweet/sphere/SphereOperators_SphereData.hpp>
#include <sweet/sphere/SphereOperators_SphereDataComplex.hpp>
//#include "../swe_sphere_timeintegrators/helpers/SWERexiTerm_SPHRobert.hpp"

#include "SWE_Sphere_TS_PFASST_interface.hpp"


#ifndef SWEET_REXI_TIMINGS
	#define SWEET_REXI_TIMINGS 1
#endif

#ifndef SWEET_REXI_TIMINGS_ADDITIONAL_BARRIERS
	#define SWEET_REXI_TIMINGS_ADDITIONAL_BARRIERS 1
#endif



#if SWEET_REXI_TIMINGS
#include <sweet/Stopwatch.hpp>
#endif

#if SWEET_MPI
	#include <mpi.h>
#endif



class SWE_Sphere_TS_PFASST_l_rexi	: public SWE_Sphere_TS_PFASST_interface
{
	typedef std::complex<double> complex;


	/// Simulation variables
	SimulationVariables &simVars;

public:
	// WARNING: Do NOT use a reference to this to get more flexibility by overriding certain things in here
	SimulationVariables::SimulationCoefficients simCoeffs;

private:
	/// Sphere operators
	SphereOperators_SphereData &op;


public:
	std::vector<std::complex<double>> rexi_alphas;
	std::vector<std::complex<double>> rexi_betas;
	std::complex<double> rexi_gamma;


	const SphereData_Config *sphereDataConfig;
	const SphereData_Config *sphereDataConfigSolver;

	/// This class is only setp and used in case of added modes
	SphereData_Config sphereDataConfigInstance;


#if SWEET_MPI
public:
//	bool final_timestep;

#endif

private:

	EXP_SimulationVariables *rexiSimVars;

	/*
	 * Time step size of REXI
	 */
	double timestep_size;

	/*
	 * Function name to be used by REXI
	 */
	std::string function_name;

	/*
	 * Don't use any Coriolis effect (reduction to very simple Helmholtz problem)
	 */
	bool no_coriolis;

	/*
	 * Assume f-sphere (reduction to Helmholtz problem)
	 */
	bool use_f_sphere;

	/*
	 * True, if REXI method is "direct"
	 */
	bool rexi_use_direct_solution;

	bool use_rexi_sphere_solver_preallocation;

	std::size_t block_size;

	class PerThreadVars
	{
	public:
//		std::vector<SWERexiTerm_SPHRobert> rexiSPHRobert_vector;

		std::vector< std::complex<double> > alpha;
		std::vector< std::complex<double> > beta_re;

		SphereData_Spectral accum_phi;
		SphereData_Spectral accum_vort;
		SphereData_Spectral accum_div;
	};

	// per-thread allocated variables to avoid NUMA domain effects
	std::vector<PerThreadVars*> perThreadVars;

	// number of threads to be used
	int num_local_rexi_par_threads;

	// number of threads to be used
	int num_global_threads;


#if SWEET_MPI
	// number of mpi ranks to be used
	int mpi_rank;

	// MPI ranks
	int num_mpi_ranks;
#endif


private:
	void reset();


public:
	SWE_Sphere_TS_PFASST_l_rexi(
			SimulationVariables &i_simVars,
			SphereOperators_SphereData &i_op
		);

private:
	void p_update_coefficients(bool i_update_rexi);

	void p_get_workload_start_end(
			std::size_t &o_start,
			std::size_t &o_end,
			int i_local_thread_id
	);


	/**
	 * setup the REXI
	 */
public:
	void setup(
			EXP_SimulationVariables &i_rexi,
			const std::string &i_function_name,
			double i_timestep_size,
			bool i_use_f_sphere,
			bool i_no_coriolis
	);

	void run_timestep_nonpert(
			SphereData_Spectral &io_h,	///< prognostic variables
			SphereData_Spectral &io_u,	///< prognostic variables
			SphereData_Spectral &io_v,	///< prognostic variables

			double i_fixed_dt,
			double i_simulation_timestamp
	);


	void run_timestep_pert(
			const SphereData_Spectral &i_h,	///< prognostic variables
			const SphereData_Spectral &i_u,	///< prognostic variables
			const SphereData_Spectral &i_v,	///< prognostic variables

			SphereData_Spectral &o_h,	///< prognostic variables
			SphereData_Spectral &o_u,	///< prognostic variables
			SphereData_Spectral &o_v,	///< prognostic variables

			double i_fixed_dt,
			double i_simulation_timestamp
	);


	/**
	 * Solve the REXI of \f$ U(t) = exp(L*t) \f$
	 *
	 * See
	 * 		doc/rexi/understanding_rexi.pdf
	 * for further information
	 */
public:
	bool run_timestep_rexi_vectorinvariant_progphivortdiv(
		SphereData_Spectral &io_phi0,
		SphereData_Spectral &io_u0,
		SphereData_Spectral &io_v0,

		double i_timestep_size,	///< timestep size

		const SimulationVariables &i_parameters
	);


	virtual ~SWE_Sphere_TS_PFASST_l_rexi();
};


#endif /* SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_PFASST_L_REXI_HPP_ */
