#! /bin/bash


echo "***********************************************"
echo "Running tests for complex array spectral operations"
echo "***********************************************"

# set close affinity of threads
export OMP_PROC_BIND=close


# 10^4 km
MAX_SCALE=$((10000*1000))
MIN_SCALE=0.01


echo "MAX/MIN SCALE: $MAX_SCALE / $MIN_SCALE"

cd ..
if true; then
	X=$MAX_SCALE
	Y=$MIN_SCALE
	echo
	echo "***********************************************"
	echo "TEST SPECTRAL OPS (release) $X x $Y"
	echo "***********************************************"
	make clean
	scons --unit-test=test_complexarray_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=disable
	EXEC="./build/test_complexarray_ops_spectral_libfft_omp_gnu_release --use-fd-for-complex-array=1 -X $X -Y $Y -S 0"
	echo "$EXEC"
	$EXEC || exit
	EXEC="./build/test_complexarray_ops_spectral_libfft_omp_gnu_release --use-fd-for-complex-array=1  -X $X -Y $Y -S 1"
	echo "$EXEC"
	$EXEC || exit
fi

X=$MAX_SCALE
echo
echo "***********************************************"
echo "TEST SPECTRAL (complex array) OPS (release) $X"
echo "***********************************************"
make clean
scons --unit-test=test_complexarray_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=disable
EXEC="./build/test_complexarray_ops_spectral_libfft_omp_gnu_release --use-fd-for-complex-array=1 -n 128 -m 128 -X $X -Y $X -S 1"
echo "$EXEC"

X=$MIN_SCALE
echo
echo "***********************************************"
echo "TEST SPECTRAL (complexarray) OPS (release) $X"
echo "***********************************************"
make clean
scons --unit-test=test_complexarray_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=disable
./build/test_complexarray_ops_spectral_libfft_omp_gnu_release --use-fd-for-complex-array=1 -n 128 -m 128 -X $X -Y $X -S 1 || exit


echo "***********************************************"
echo "***********************************************"
echo "***************** FIN *************************"
echo "***********************************************"
echo "***********************************************"