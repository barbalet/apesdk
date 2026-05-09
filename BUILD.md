## Building the ApeSDK

ApeSDK has been built with a variety of development environments including CodeWarrior, Visual Studio and XCode. As a majority of the development environments used to build the Simulation have not included traditional make files, the Simulation source is not packaged with these files.

You can also get sub-projects that require the ApeSDK and contain these project files (`simulatedape` for the Simulated Ape Mac and Windows GUIs, the Simulated Urban Mac JSON output togetther with the Planet and War Simulation).

## Command Line

There are a number of command line build scripts. These should build for Mac and Linux:

* `build.sh` - builds the `simape` command line version that allows for a variety of executions and analysis

In the test directory:

* `test_gui_wo_exec.sh` - builds `test_gui` which runs the guis in a leaderless mode to check buffers/overruns etc. Note: this script doesn’t execute `test_gui`, that is done separately.

* `test_toolkit_wo_exec.sh` - builds a series of executable binaries including `test_math`, `test_io`, `test_object`, `test_object_string` etc

* `test_object_file.sh` - run after `test_toolkit_wo_exec.sh` to feed through and test the object files contained within the test directory.


* `test_script_wo_exec.sh` - builds `test_apescript` without executing it

* `test_wo_exec.sh` - builds `test_sim` and `test_sim_time` without executing them.

## Contact

To contact Tom Barbalet directly email;

   `barbalet at gmail dot com`
