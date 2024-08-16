# Script; shyam kumar; INFN Bari, Italy
# Modified: Tuna Tasali. University of Oxford
# shyam.kumar@ba.infn.it; shyam055119@gmail.com
echo $mom_value_global
#store the global variable in a local one to avoid conflicts
mom_value=$mom_value_global
particle_array=("pi-")
filename=("tracking_output") 
etabin_array=(-3.5 -2.5 -1.0 1.0 2.5 3.5)
nevents=10000
#ADJUST THESE VALUES!!!!!!
results_dir="ParallelResults_n10000"
compact_file_name="epic_craterlake_tracking_only.xml"

source ../epic/install/bin/thisepic.sh
source ../epic/install/setup.sh 
ddsim --compactFile ../epic/install/share/epic/$compact_file_name --outputFile $results_dir/sim$mom_value.edm4hep.root --numberOfEvents $nevents --enableGun --gun.thetaMin 3*deg --gun.thetaMax 177*deg --gun.distribution eta --gun.particle pi- --gun.momentumMin $mom_value*GeV --gun.momentumMax $mom_value*GeV --gun.multiplicity 1 --random.seed 100000
#this bit I added to make sure the default eic-shell eicrecon is running
/opt/local/bin/eicrecon -Pnthreads=1 -Pjana:debug_plugin_loading=1 -Pjana:nevents=$nevents \
    -Pacts:MaterialMap=calibrations/materials-map.cbor \
    -Ppodio:output_file=$results_dir/"${filename}"_$mom_value.edm4eic.root \
    -Pdd4hep:xml_files=../epic/install/share/epic/$compact_file_name   \
    -Ppodio:output_collections="MCParticles,CentralCKFTrajectories,CentralCKFTrackParameters,CentralCKFSeededTrackParameters,CentralTrackVertices" \
    $results_dir/sim$mom_value.edm4hep.root
#note that it is absolutely crucial that the line $results_dir has no indentation wrt. the line above!!!!
