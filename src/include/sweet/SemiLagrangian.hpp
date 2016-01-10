/*
 * SemiLangrangian.hpp
 *
 *  Created on: 5 Dec 2015
 *      Author: Martin Schreiber <schreiberx@gmail.com>
 */
#ifndef SRC_INCLUDE_SWEET_SEMILAGRANGIAN_HPP_
#define SRC_INCLUDE_SWEET_SEMILAGRANGIAN_HPP_

#include <sweet/DataArray.hpp>
#include <sweet/Sampler2D.hpp>

class SemiLagrangian
{
	Sampler2D sample2D;

public:
	SemiLagrangian()
	{
	}


	void setup(
		double i_domain_size[2],
		std::size_t i_res[2]
	)
	{
		sample2D.setup(i_domain_size, i_res);
	}


	/**
	 * Stable extrapolation Two-Time-Level Scheme, Mariano Hortal,
	 *     Development and testing of a new two-time-level semi-lagrangian scheme (settls) in the ECMWF forecast model.
	 * Quaterly Journal of the Royal Meterological Society
	 *
	 * r_d = r_a - dt/2 * (2 * v_n(r_d) - v_{n-1}(r_d) + v_n(r_a))
	 *
	 * v^{iter} := (dt*v_n - dt*0.5*v_{n-1})
	 * r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
	 */
	void compute_departure_points_settls(
			DataArray<2>* i_velocity_field_t_prev[2],	///< velocity field at time n-1
			DataArray<2>* i_velocity_field_t[2],	///< velocity field at time n
			DataArray<2>* i_pos_arrival[2],			///< position at time n+1
			double i_dt,							///< time step size
			DataArray<2>* o_pos_departure[2],			///< departure points at time n,
			double *i_staggering = nullptr				///< staggering (ux, uy, vx, vy)
	)
	{
		if (i_staggering == nullptr)
		{
			static double constzerostuff[4] = {0,0,0,0};
			i_staggering = constzerostuff;
		}

		DataArray<2> &vx_n_prev = *i_velocity_field_t_prev[0];
		DataArray<2> &vy_n_prev = *i_velocity_field_t_prev[1];

		DataArray<2> &vx_n = *i_velocity_field_t[0];
		DataArray<2> &vy_n = *i_velocity_field_t[1];

		DataArray<2> &rx_a = *i_pos_arrival[0];
		DataArray<2> &ry_a = *i_pos_arrival[1];

		DataArray<2> &rx_d = *o_pos_departure[0];
		DataArray<2> &ry_d = *o_pos_departure[1];

		rx_d.set(0,0,0);
		ry_d.set(0,0,0);

		double dt = i_dt;

		DataArray<2> vx_iter(vx_n_prev.resolution);
		DataArray<2> vy_iter(vx_n_prev.resolution);

		vx_iter = dt * vx_n - dt*0.5 * vx_n_prev;
		vy_iter = dt * vy_n - dt*0.5 * vy_n_prev;

		DataArray<2> rx_d_new(vx_n_prev.resolution);
		DataArray<2> ry_d_new(vx_n_prev.resolution);

		DataArray<2> rx_d_prev = rx_a;
		DataArray<2> ry_d_prev = rx_a;

		//DataArray<2>* r_d[2] = {&rx_d, &ry_d};

		// initialize departure points with arrival points
		rx_d = rx_a;
		ry_d = ry_a;

		int iters = 0;
		for (; iters < 10; iters++)
		{
			// r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
			rx_d_new = rx_a - dt*0.5 * vx_n - sample2D.bilinear_scalar(vx_iter, rx_d, ry_d, i_staggering[0], i_staggering[1]);
			ry_d_new = ry_a - dt*0.5 * vy_n - sample2D.bilinear_scalar(vy_iter, rx_d, ry_d, i_staggering[2], i_staggering[3]);

			double diff = (rx_d_new - rx_d_prev).reduce_maxAbs() + (ry_d_new - ry_d_prev).reduce_maxAbs();
			rx_d_prev = rx_d_new;
			ry_d_prev = ry_d_new;

			for (std::size_t i = 0; i < rx_d.resolution[0]*rx_d.resolution[1]; i++)
			{
				rx_d.array_data_cartesian_space[i] = sample2D.wrapPeriodic(rx_d_new.array_data_cartesian_space[i], sample2D.domain_size[0]);
				ry_d.array_data_cartesian_space[i] = sample2D.wrapPeriodic(ry_d_new.array_data_cartesian_space[i], sample2D.domain_size[1]);
			}

			if (diff < 1e-8)
				break;
//			std::cout << iters << ": " << diff << std::endl;
		}
//		std::cout << iters << std::endl;
	}

	/**
	 * Stable extrapolation Two-Time-Level Scheme, Mariano Hortal,
	 *     Development and testing of a new two-time-level semi-lagrangian scheme (settls) in the ECMWF forecast model.
	 * Quaterly Journal of the Royal Meterological Society
	 *
	 * r_d = r_a - dt/2 * (2 * v_n(r_d) - v_{n-1}(r_d) + v_n(r_a))
	 *
	 * v^{iter} := (dt*v_n - dt*0.5*v_{n-1})
	 * r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
	 */
	void semi_lag_departure_points_settls(
			DataArray<2> &i_u_prev, // Velocities at time t-1
			DataArray<2> &i_v_prev,
			DataArray<2> &i_u, // Velocities at time t
			DataArray<2> &i_v,
			DataArray<2> &i_posx_a, // Position of arrival points x / y
			DataArray<2> &i_posy_a,
			double i_dt,			///< time step size
			DataArray<2> &o_posx_d, // Position of departure points x / y
			DataArray<2> &o_posy_d,
			double *i_staggering = nullptr	///< staggering, if any (ux, uy, vx, vy)
	)
	{
		if (i_staggering == nullptr)
		{
			static double constzerostuff[4] = {0,0,0,0};
			i_staggering = constzerostuff;
		}

		//Init departure points
		o_posx_d.set(0,0,0);
		o_posy_d.set(0,0,0);

		//local dt
		double dt = i_dt;

		//Velocity for iterations
		DataArray<2> u_iter(i_u_prev.resolution);
		DataArray<2> v_iter(i_u_prev.resolution);

		//Time Extrapolation
		u_iter = dt * i_u - dt*0.5 * i_u_prev;
		v_iter = dt * i_v - dt*0.5 * i_v_prev;

		//Departure point tmp
		DataArray<2> rx_d_new(i_u_prev.resolution);
		DataArray<2> ry_d_new(i_u_prev.resolution);

		//Previous departure point
		DataArray<2> rx_d_prev = i_posx_a;
		DataArray<2> ry_d_prev = i_posy_a;

		// initialize departure points with arrival points
		o_posx_d = i_posx_a;
		o_posy_d = i_posy_a;

		int iters = 0;
		for (; iters < 10; iters++)
		{
			// r_d = r_a - dt/2 * v_n(r_d) - v^{iter}(r_d)
			rx_d_new = i_posx_a - dt*0.5 * i_u - sample2D.bilinear_scalar(u_iter, o_posx_d, o_posy_d, i_staggering[0], i_staggering[1]);
			ry_d_new = i_posy_a - dt*0.5 * i_v - sample2D.bilinear_scalar(v_iter, o_posx_d, o_posy_d, i_staggering[2], i_staggering[3]);

			double diff = (rx_d_new - rx_d_prev).reduce_maxAbs() + (ry_d_new - ry_d_prev).reduce_maxAbs();
			rx_d_prev = rx_d_new;
			ry_d_prev = ry_d_new;

			for (std::size_t i = 0; i < o_posx_d.resolution[0]*o_posx_d.resolution[1]; i++)
			{
				o_posx_d.array_data_cartesian_space[i] = sample2D.wrapPeriodic(rx_d_new.array_data_cartesian_space[i], sample2D.domain_size[0]);
				o_posy_d.array_data_cartesian_space[i] = sample2D.wrapPeriodic(ry_d_new.array_data_cartesian_space[i], sample2D.domain_size[1]);
			}

			if (diff < 1e-8)
				break;
				std::cout << iters << ": " << diff << std::endl;
		}
	//		std::cout << iters << std::endl;
	}
};

#endif /* SRC_INCLUDE_SWEET_SEMILAGRANGIAN_HPP_ */
