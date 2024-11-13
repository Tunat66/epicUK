A warm welcome to Oxford Particle Physics ePIC research group. Here are some useful instructions
provided that you installed the eic-shell and cloned this repository.
********************************************************************************************************
SETUP:
Now that you have cloned the repository, do the following:

0- rename the repo folder to "epic" (this is because some .sh scripts contain a silly ../epic directory, you can fix this later.)
1- Get in the eic-shell. To get in the eic-shell:
    1- Make sure the eic-shell executable is installed on the parent directory (..)
    2- Run: ../eic-shell
    3- Run: source cmake_build.sh (this will set up the .xml files and scripts in the build directory, that is install/)
    4- Run: source install/setup.sh to set up your environment variables.
3- Your project is ready. Edit as you like. After an edit in the compact files or the src files, recompile the edit by sourcing recompile.sh

********************************************************************************************************
WORKFLOW:
To commit your changes, use "git commit -a" command and write your commit message in the vim or vscode editor popping up.
Git pushes don't work in the eic-shell, quit the shell before pushing a commit.

To produce a root file for visualization: source visualize.sh <name_of_compact_file_you_want_to_visualize>
    This will produce a detector_geometry.root file in $DETECTOR_PATH directory. 

To build the repository: source cmake_build.sh.
To recompile a simulation after making changes to the compact or src files: source recompile.sh.

To test a setup with vertex barrel and silicon barrel using a single event simulation: source test_silicon_vertex_oneevent.sh
To test sim and recon: use the script run_single_momentum_test.sh. Adjust the variables to your liking. 
    DISCLAIMER: Whatever you do, DON'T do heavy computing on it unless it is your personal machine.

To import a new CAD Geometry: Go to the ./CAD directory and follow the instructions in the README file there.

Raise any immediate bugs/faults as an issue in the github repo. If it is something severe: email me at mustafa.tasali@pmb.ox.ac.uk
*****************************************************************************************************************************
OXFORD ONLY (Written in August 2024):

DON'T RUN YOUR FULL SIMULATION ON THE PHYSICS SERVER. You will need to submit it to the cluster. The condor_submit command will
handle that for you. Fortunately, I created some scripts for you that will handle that automatically. However, those scripts
require some configuring by hand.

1- open run_multi_momentum.sh
2- in the file you will see some variables under the comment CONFIG_VARIABLES. Adjust them to your liking.
3- Adjust the same variables to the same values in thr run_single_momentum.sh file (I should find a better way of doing this, probably just exporting the variables.)
4- run the run_multi_momentum.sh file (./run_multi_momentum.sh, don't source it) 

After running your simulation you should see a new folder generated containing a bunch of sim and tracking_output files with the
.root extension. These files contain the simulation and reconstruction data respectively. To peform analysis run the given script
as follows with the argument:

1- Run: ./analye.sh <directory_containing_sim_and_tracking_output>

To perform some further analysis I included the root macros: Tracking_Hits_phiz.C provides an "unrolled" hit plot of the detector 
and Tracking_Hits_xy.C provides a cross sectional plot, both very useful for confirming the correct placement/geometry of sensors.
To use them, just pass the directory of a simulation file (starting with a "sim" followed by a number and ending with a .root extension).

There is another script, ReconEfficiency.C that enables the plotting of reconstruction efficiency in a particular run. To use it, just
pass the name of the experiment folder as an argument to the root macro.

Similarly, the files doCompare_results_mom.C, doCompare_results_dcaT.C, doCompare_results_dcaz.C allow you to compare results between
runs. To use them, pass the name of the run folders as follows (note the curly braces in the argument):

    .L <Name_of_the_script>.C
    <Name_of_the_script>({"<results_folder_1>", etc. etc.})

To get a sensitive area report on an L4 stave only (currently), read Sens_Area_Report.txt

Enjoy!
************************************************************************************************************************************
TASKS: Here are some immediate tasks to get you going:

0- Read the report and go through the slides to get an overview of the system. They contain some essential information.
1- Get in the BarrelTrackerOuter_standardized_geo.cpp file, fix the bits writing out to Sens_Area_Report.txt to include clear information
about both the sensitive areas L3 and L4 barrels. (currently, it only does so for the L4 Stave, not the entire barrel, only.)
2- There was an issue with track reconstrucion efficiencies until I realised it was not an issue. Of course if you exclude the
endcaps and have a source shooting in all directions, MOST OF THE TRACKS WON'T HIT ANYTHING. I realised instead that the L4 design
with no dead areas was wrong. Fix it with a CAD software of your own liking and produce some new data.
3- Experimental visualization method: I am trying to visualize the entire barrel using the Geant4 visualization system, and it worked legends,
the issue was to allow a docker/singularity shell to produce graphical output, you need an XServer which you then point towards
the IP address of your monitor, allowing visual output. I followed the following steps so far. 

    0- (Note that I followed all these in a WSL Linux virtual machine on my own personal Win11 Laptop and NOT the Linux server, 
    your mileage may vary)
    0- Just trying to run Geant_Visualize.sh gave me an error like:
        G4OpenGLStoredX::CreateViewer: error flagged by negative view id in G4OpenGLStoredXViewer creation.
    Try that, tweak the the command in the shell script. If a similar error appears:
    0- ChatGPT is your friend now. I didn't come up with the XServer idea (I didn't know they existed), ChatGPT did. (Althought it did take substantial tweaking.)
    1- Install an XServer (VcXsrv is good.) ASK PERMISSION FROM IT SUPPORT FOR THIS OR CONSIDER INSTALLING IN eic-shell as an alternative.
       Run it with the following command: vcxsrv -ArgumentList ":0 -multiwindow -clipboard -ac" #vcxsrv executable dir must be in $PATH
    2- Export the display IP address as a variable:
        export DISPLAY=$(grep -oP "(?<=nameserver ).+" /etc/resolv.conf):0
        source ~/.bashrc
    If the above commands don't work (didn't work for me), try finding the relevant IP address manually by:
        1- For powershell in windows, I found the IP address with the command: ipconfig (I don't know the bash equivalent.)
        2- Look for the IPv4 address printed.
        2- export DISPLAY=<whatever_ipconfig_gave_for_IPv4_address>
        3- echo $DISPLAY    
    3- This is the hard bit, the eic-shell executable is actually a shell script, where there is a docker or singularity command
    starting the shell. We need to tweak that command to pass this $DISPLAY environment variable. In docker it was something like:
    docker run -it --rm \
    -e DISPLAY=$DISPLAY \ #add this line, specifically
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    your-docker-image
    4- Start the eic-shell and do the following:
        1- install the following with apt-get: x11-apps libx11-dev libxext-dev libglu1-mesa-dev freeglut3-dev
        2- run the command: xclock. If a clock appears on the screen, you are good to go. Visualization should work with Geant_Visualize.sh
    
The above method seemed to work with a geometry where dead areas were not modelled (less detailed.)
I also assumed you know how Geant4 Macro files work. If you don't, consult your supervisor.

After you got all these sorted out, try to do an all detector materialScan. There seemed to be something wrong with this 
(described in the presentation.)

