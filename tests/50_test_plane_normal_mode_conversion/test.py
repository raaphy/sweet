#! /usr/bin/env python3

import sys
import os
os.chdir(os.path.dirname(sys.argv[0]))

print("*"*80)
for key, value in os.environ.items():
    print(key+":"+value)
print("*"*80)

from mule_local.JobMule import *
from mule.exec_program import *
from mule.InfoError import *

exec_program('mule.benchmark.cleanup_all', catch_output=False)

jg = JobGeneration()
jg.compile.unit_test="test_plane_sw_normal_modes"
jg.compile.plane_spectral_space="enable"
jg.compile.plane_spectral_dealiasing="enable"
jg.compile.mode="release"
for nx in [8, 36]:
	for ny in [ 8, 36]:
		jg.runtime.space_res_physical = (nx, ny)
		jg.gen_jobscript_directory()

exitcode = exec_program('mule.benchmark.jobs_run_directly', catch_output=False)
if exitcode != 0:
	sys.exit(exitcode)

exec_program('mule.benchmark.cleanup_all', catch_output=False)
