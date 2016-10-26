/*
 * SphereData_To_ScalarDataArray.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: martin
 */

#ifndef SRC_INCLUDE_SWEET_SPHERE_CONVERT_SPHEREDATACOMPLEX_TO_SPHEREDATA_HPP_
#define SRC_INCLUDE_SWEET_SPHERE_CONVERT_SPHEREDATACOMPLEX_TO_SPHEREDATA_HPP_

#include <sweet/sphere/SphereData.hpp>
#include <sweet/sphere/SphereDataComplex.hpp>
#include <sweet/ScalarDataArray.hpp>

class Convert_SphereDataComplex_To_SphereData
{
public:
	static
	SphereData physical_convert(
			const SphereDataComplex &i_sphereData
	)
	{
		SphereData out(i_sphereData.sphereDataConfig);

		i_sphereData.request_data_physical();

#if SWEET_THREADING
#pragma omp parallel for
#endif
		for (int i = 0; i < out.sphereDataConfig->physical_array_data_number_of_elements; i++)
			out.physical_space_data[i] = i_sphereData.physical_space_data[i].real();

#if SWEET_USE_SPHERE_SPECTRAL_SPACE
		out.physical_space_data_valid = true;
		out.spectral_space_data_valid = false;
#endif

		return out;
	}
};



#endif /* SRC_INCLUDE_SWEET_SPHERE_CONVERT_SPHEREDATA_TO_SCALARDATAARRAY_HPP_ */
