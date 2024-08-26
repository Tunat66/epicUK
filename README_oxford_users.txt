A warm welcome to Oxford Particle Physics ePIC research group. Here are some useful instructions
provided that you installed the eic-shell and cloned this repository.
To set up the eic-shell:
    1- Make sure the eic-shell executable is installed on the parent directory (..)
    2- Run: ../eic-shell
    3- Run: source install/setup.sh

To build the repository: source cmake_build.sh
To recompile a simulation after making changes to the compact or src files: source recompile.sh
To test a setup with vertex barrel and silicon barrel using a single event simulation: source test_silicon_vertex_oneevent.sh

*****************************************************************************************************************************

OXFORD ONLY (August 2024):

DON'T RUN YOUR FULL SIMULATION ON THE PHYSICS SERVER. You will need to submit it to the cluster. The condor_submit command will
handle that for you. Fortunately, I created some scripts for you that will handle that automatically.

WORK IN PROGRESS