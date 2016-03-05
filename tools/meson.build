# TODO: Change test executable to CLI UI when it has been added to public
#       repository and make arguments configurable.
test_exe = lib_unit_tests
test_args = ['--gtest_repeat=100', '--gtest_filter=AnimatedTransformTest.*']

run_target('perf', 'perf.sh', test_exe, test_args)
run_target('perfstat', 'perfstat.sh', test_exe, test_args)
