A warm welcome to Oxford Particle Physics ePIC research group. Here are some useful instructions
provided that you installed the eic-shell and cloned this repository.
To set up the eic-shell:
    1- Make sure the eic-shell executable is installed on the parent directory (..)
    2- Run: ../eic-shell
    3- Run: source install/setup.sh

To build the repository: source cmake_build.sh
To recompile a simulation after making changes to the compact or src files: source recompile.sh

WORK IN PROGRESS
To test a setup with vertex barrel and silicon barrel using a single event simulation: source test_silicon_vertex_oneevent.sh
To create a file install/share/detector_geometry.root containing models of the detectors

*****************************************************************************************************************************

OXFORD ONLY (August 2024):

DON'T RUN YOUR FULL SIMULATION ON THE PHYSICS SERVER. You will need to submit it to the cluster. The condor_submit command will
handle that for you. Fortunately, I created some scripts for you that will handle that automatically. However, those scripts
require some configuring by hand.

WORK IN PROGRESS
1- open run_multi_momentum.sh
2- in the file you will see some variables under the comment CONFIG_VARIABLES. Adjust them to your liking.
3- do the same for the 

After running your simulation you should see a new folder generated containing a bunch of sim and tracking_output files with the
.root extension. These files contain the simulation and reconstruction data respectively. To peform analysis run the given script
as follows with the argument:

1- Run: ./analye.sh <directory_containing_sim_and_tracking_output>

To perform some further analysis I included the root macros: Tracking_Hits_phiz.C provides an "unrolled" hit plot of the detector 
and Tracking_Hits_xy.C provides a cross sectional plot, both very useful for confirming the correct placement/geometry of sensors.
To use them, just pass the name of a simulation file (starting with a "sim" followed by a number and ending with a .root extension).
