/*
 * SWE_Sphere_TS_ln_edtrk.cpp
 *
 *  Created on: 21 Aug 2017
 *      Author: Martin Schreiber <SchreiberX@gmail.com>
 */

#include "SWE_Sphere_TS_l_rexi_n_etdrk.hpp"




void SWE_Sphere_TS_l_rexi_n_etdrk::run_timestep(
		SphereDataSpectral &io_phi,	///< prognostic variables
		SphereDataSpectral &io_u,	///< prognostic variables
		SphereDataSpectral &io_v,	///< prognostic variables

		double i_dt,
		double i_simulation_timestamp
)
{
	if (i_dt <= 0)
		FatalError("SWE_Plane_TS_l_phi0_n_edt: Only constant time step size allowed");

	const SphereDataConfig *sphereDataConfig = io_phi.sphereDataConfig;

	if (timestepping_order == 0 || timestepping_order == 1)
	{
		/*
		 * U_{1} = \psi_{0}( \Delta t L ) U_{0}
		 * 			+\Delta t \psi_{1}(\Delta tL) N(U_{0}).
		 */

		SphereDataSpectral phi0_Un_h(sphereDataConfig);
		SphereDataSpectral phi0_Un_u(sphereDataConfig);
		SphereDataSpectral phi0_Un_v(sphereDataConfig);
		ts_phi0_rexi.run_timestep(
				io_phi, io_u, io_v,
				phi0_Un_h, phi0_Un_u, phi0_Un_v,
				i_dt,
				i_simulation_timestamp
			);

		SphereDataSpectral FUn_h(sphereDataConfig);
		SphereDataSpectral FUn_u(sphereDataConfig);
		SphereDataSpectral FUn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				io_phi, io_u, io_v,
				FUn_h, FUn_u, FUn_v,
				i_simulation_timestamp
		);

		SphereDataSpectral phi1_FUn_h(sphereDataConfig);
		SphereDataSpectral phi1_FUn_u(sphereDataConfig);
		SphereDataSpectral phi1_FUn_v(sphereDataConfig);

		ts_phi1_rexi.run_timestep(
				FUn_h, FUn_u, FUn_v,
				phi1_FUn_h, phi1_FUn_u, phi1_FUn_v,
				i_dt,
				i_simulation_timestamp
			);

		io_phi = phi0_Un_h + i_dt*phi1_FUn_h;
		io_u = phi0_Un_u + i_dt*phi1_FUn_u;
		io_v = phi0_Un_v + i_dt*phi1_FUn_v;
	}
	else if (timestepping_order == 2)
	{
		/*
		 * A_{n}=\psi_{0}(\Delta tL)U_{n}+\Delta t\psi_{1}(\Delta tL)F(U_{n})
		 */

		SphereDataSpectral phi0_Un_h(sphereDataConfig);
		SphereDataSpectral phi0_Un_u(sphereDataConfig);
		SphereDataSpectral phi0_Un_v(sphereDataConfig);

		ts_phi0_rexi.run_timestep(
				io_phi, io_u, io_v,
				phi0_Un_h, phi0_Un_u, phi0_Un_v,
				i_dt,
				i_simulation_timestamp
			);

		SphereDataSpectral FUn_h(sphereDataConfig);
		SphereDataSpectral FUn_u(sphereDataConfig);
		SphereDataSpectral FUn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				io_phi, io_u, io_v,
				FUn_h, FUn_u, FUn_v,
				i_simulation_timestamp
		);

		SphereDataSpectral phi1_FUn_h(sphereDataConfig);
		SphereDataSpectral phi1_FUn_u(sphereDataConfig);
		SphereDataSpectral phi1_FUn_v(sphereDataConfig);

		ts_phi1_rexi.run_timestep(
				FUn_h, FUn_u, FUn_v,
				phi1_FUn_h, phi1_FUn_u, phi1_FUn_v,
				i_dt,
				i_simulation_timestamp
			);

		SphereDataSpectral A_h = phi0_Un_h + i_dt*phi1_FUn_h;
		SphereDataSpectral A_u = phi0_Un_u + i_dt*phi1_FUn_u;
		SphereDataSpectral A_v = phi0_Un_v + i_dt*phi1_FUn_v;

		/*
		 * U_{n+1} = A_{n}+ \Delta t \psi_{2}(\Delta tL)
		 * 				\left(F(A_{n},t_{n}+\Delta t)-F(U_{n})\right)
		 */

		SphereDataSpectral FAn_h(sphereDataConfig);
		SphereDataSpectral FAn_u(sphereDataConfig);
		SphereDataSpectral FAn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				A_h, A_u, A_v,
				FAn_h, FAn_u, FAn_v,
				i_simulation_timestamp
		);


		SphereDataSpectral phi2_X_h(sphereDataConfig);
		SphereDataSpectral phi2_X_u(sphereDataConfig);
		SphereDataSpectral phi2_X_v(sphereDataConfig);

		ts_phi2_rexi.run_timestep(
				FAn_h - FUn_h,
				FAn_u - FUn_u,
				FAn_v - FUn_v,

				phi2_X_h,
				phi2_X_u,
				phi2_X_v,

				i_dt,
				i_simulation_timestamp
			);

		io_phi = A_h + i_dt*phi2_X_h;
		io_u = A_u + i_dt*phi2_X_u;
		io_v = A_v + i_dt*phi2_X_v;
	}
	else if (timestepping_order == 4)
	{
		double dt = i_dt;
		double dt_half = dt*0.5;

		/*
		 * Precompute commonly used terms
		 */
		SphereDataSpectral phi0_Un_h(sphereDataConfig);
		SphereDataSpectral phi0_Un_u(sphereDataConfig);
		SphereDataSpectral phi0_Un_v(sphereDataConfig);

		ts_phi0_rexi.run_timestep(
				io_phi, io_u, io_v,
				phi0_Un_h, phi0_Un_u, phi0_Un_v,
				dt_half,
				i_simulation_timestamp
			);

		SphereDataSpectral FUn_h(sphereDataConfig);
		SphereDataSpectral FUn_u(sphereDataConfig);
		SphereDataSpectral FUn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				io_phi, io_u, io_v,
				FUn_h, FUn_u, FUn_v,
				i_simulation_timestamp
		);



		/*
		 * Some commonly shared buffers
		 */

		SphereDataSpectral phi1_h(sphereDataConfig);
		SphereDataSpectral phi1_u(sphereDataConfig);
		SphereDataSpectral phi1_v(sphereDataConfig);


		/*
		 * A_{n} = \psi_{0}(0.5*\Delta tL)U_{n} + \Delta t\psi_{1}(0.5*\Delta tL) F(U_{n})
		 */
		ts_phi1_rexi.run_timestep(
				FUn_h, FUn_u, FUn_v,
				phi1_h, phi1_u, phi1_v,
				dt_half,
				i_simulation_timestamp
			);

		SphereDataSpectral A_h = phi0_Un_h + dt_half*phi1_h;
		SphereDataSpectral A_u = phi0_Un_u + dt_half*phi1_u;
		SphereDataSpectral A_v = phi0_Un_v + dt_half*phi1_v;



		/*
		 * B_{n} = \psi_{0}(0.5*\Delta tL)U_{n} + 0.5*\Delta t\psi_{1}(0.5*\Delta tL) F(A_{n}, t_{n} + 0.5*\Delta t)
		 */

		SphereDataSpectral FAn_h(sphereDataConfig);
		SphereDataSpectral FAn_u(sphereDataConfig);
		SphereDataSpectral FAn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				A_h, A_u, A_v,
				FAn_h, FAn_u, FAn_v,
				i_simulation_timestamp + dt_half
		);

		ts_phi1_rexi.run_timestep(
				FAn_h, FAn_u, FAn_v,
				phi1_h, phi1_u, phi1_v,
				dt_half,
				i_simulation_timestamp
			);

		SphereDataSpectral B_h = phi0_Un_h + dt_half*phi1_h;
		SphereDataSpectral B_u = phi0_Un_u + dt_half*phi1_u;
		SphereDataSpectral B_v = phi0_Un_v + dt_half*phi1_v;



		/*
		 * C_{n} = \psi_{0}(0.5*\Delta tL)U_{n} + 0.5*\Delta t\psi_{1}(0.5* \Delta tL) ( 2 F(B_{n},t_{n} + 0.5*\Delta t)-F(U_{n},t_{n})).
		 */

		SphereDataSpectral phi0_An_h(sphereDataConfig);
		SphereDataSpectral phi0_An_u(sphereDataConfig);
		SphereDataSpectral phi0_An_v(sphereDataConfig);

		ts_phi0_rexi.run_timestep(
				A_h, A_u, A_v,
				phi0_An_h, phi0_An_u, phi0_An_v,
				dt_half,
				i_simulation_timestamp
			);


		SphereDataSpectral FBn_h(sphereDataConfig);
		SphereDataSpectral FBn_u(sphereDataConfig);
		SphereDataSpectral FBn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				B_h, B_u, B_v,
				FBn_h, FBn_u, FBn_v,
				i_simulation_timestamp + dt_half
		);

		ts_phi1_rexi.run_timestep(
				2.0*FBn_h - FUn_h,
				2.0*FBn_u - FUn_u,
				2.0*FBn_v - FUn_v,
				phi1_h,	phi1_u,	phi1_v,
				dt_half,
				i_simulation_timestamp
			);

		SphereDataSpectral C_h = phi0_An_h + dt_half*phi1_h;
		SphereDataSpectral C_u = phi0_An_u + dt_half*phi1_u;
		SphereDataSpectral C_v = phi0_An_v + dt_half*phi1_v;



		/*
		 * R0 - R3
		 */
		SphereDataSpectral FCn_h(sphereDataConfig);
		SphereDataSpectral FCn_u(sphereDataConfig);
		SphereDataSpectral FCn_v(sphereDataConfig);

		ts_l_erk_n_erk.euler_timestep_update_n(
				C_h, C_u, C_v,
				FCn_h, FCn_u, FCn_v,
				i_simulation_timestamp + dt
		);

		SphereDataSpectral R0_h = io_phi;
		SphereDataSpectral R0_u = io_u;
		SphereDataSpectral R0_v = io_v;

		SphereDataSpectral &R1_h = FUn_h;
		SphereDataSpectral &R1_u = FUn_u;
		SphereDataSpectral &R1_v = FUn_v;

		SphereDataSpectral R2_h = FAn_h + FBn_h;
		SphereDataSpectral R2_u = FAn_u + FBn_u;
		SphereDataSpectral R2_v = FAn_v + FBn_v;

		SphereDataSpectral &R3_h = FCn_h;
		SphereDataSpectral &R3_u = FCn_u;
		SphereDataSpectral &R3_v = FCn_v;


		/*
		 * U_{n+1} =
		 * 		\psi_{0}(\Delta tL)R_{0}
		 * 			+ \Delta t
		 * 			(
		 * 				  \upsilon_{1}(\Delta tL) R_{1} +
		 * 				2*\upsilon_{2}(\Delta tL) R_{2} +
		 * 				  \upsilon_{3}(\Delta tL) R_{3}
		 * 			)
		 */
		ts_ups0_rexi.run_timestep(
				R0_h, R0_u, R0_v,
				dt,		i_simulation_timestamp
			);

		ts_ups1_rexi.run_timestep(
				R1_h, R1_u, R1_v,
				dt,		i_simulation_timestamp
			);

		ts_ups2_rexi.run_timestep(
				R2_h, R2_u, R2_v,
				dt,		i_simulation_timestamp
			);

		ts_ups3_rexi.run_timestep(
				R3_h, R3_u, R3_v,
				dt,		i_simulation_timestamp
			);

		io_phi = R0_h + dt*(R1_h + 2.0*R2_h + R3_h);
		io_u = R0_u + dt*(R1_u + 2.0*R2_u + R3_u);
		io_v = R0_v + dt*(R1_v + 2.0*R2_v + R3_v);
	}
	else
	{
		FatalError("TODO: This order is not implemented, yet!");
	}
}



/*
 * Setup
 */
void SWE_Sphere_TS_l_rexi_n_etdrk::setup(
		REXI_SimulationVariables &i_rexiSimVars,
		int i_timestepping_order,
		int i_timestepping_order2,
		double i_timestep_size
)
{
	timestepping_order = i_timestepping_order;
	timestepping_order2 = i_timestepping_order2;

	ts_l_erk_n_erk.setup(timestepping_order, timestepping_order2);

	if (timestepping_order != timestepping_order2)
		FatalError("Mismatch of orders, should be equal");

	if (timestepping_order == 0 || timestepping_order == 1)
	{
		ts_phi0_rexi.setup(i_rexiSimVars, "phi0", i_timestep_size, false, false);	/* set use_f_sphere to true */
		ts_phi1_rexi.setup(i_rexiSimVars, "phi1", i_timestep_size, false, false);
	}
	else if (timestepping_order == 2)
	{
		ts_phi0_rexi.setup(i_rexiSimVars, "phi0", i_timestep_size, false, false);	/* set use_f_sphere to true */
		ts_phi1_rexi.setup(i_rexiSimVars, "phi1", i_timestep_size, false, false);
		ts_phi2_rexi.setup(i_rexiSimVars, "phi2", i_timestep_size, false, false);
	}
	else if  (timestepping_order == 4)
	{
		ts_phi0_rexi.setup(i_rexiSimVars, "phi0", i_timestep_size*0.5, false, false);	/* set use_f_sphere to true */
		ts_phi1_rexi.setup(i_rexiSimVars, "phi1", i_timestep_size*0.5, false, false);
		ts_phi2_rexi.setup(i_rexiSimVars, "phi2", i_timestep_size*0.5, false, false);

		// phi0, but with a full time step size
		ts_ups0_rexi.setup(i_rexiSimVars, "phi0", i_timestep_size, false, false);
		ts_ups1_rexi.setup(i_rexiSimVars, "ups1", i_timestep_size, false, false);
		ts_ups2_rexi.setup(i_rexiSimVars, "ups2", i_timestep_size, false, false);
		ts_ups3_rexi.setup(i_rexiSimVars, "ups3", i_timestep_size, false, false);
	}
	else
	{
		FatalError("TODO: This order is not implemented, yet!");
	}
}


SWE_Sphere_TS_l_rexi_n_etdrk::SWE_Sphere_TS_l_rexi_n_etdrk(
		SimulationVariables &i_simVars,
		SphereOperators &i_op
)	:
		simVars(i_simVars),
		op(i_op),

		ts_l_erk_n_erk(simVars, op),

		ts_phi0_rexi(simVars, op),
		ts_phi1_rexi(simVars, op),
		ts_phi2_rexi(simVars, op),

		ts_ups0_rexi(simVars, op),
		ts_ups1_rexi(simVars, op),
		ts_ups2_rexi(simVars, op),
		ts_ups3_rexi(simVars, op)
{
}



SWE_Sphere_TS_l_rexi_n_etdrk::~SWE_Sphere_TS_l_rexi_n_etdrk()
{
}

