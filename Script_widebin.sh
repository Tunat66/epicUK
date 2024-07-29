#!/bin/bash
# Script; shyam kumar; INFN Bari, Italy
# Modified: Tuna Tasali. University of Oxford
# shyam.kumar@ba.infn.it; shyam055119@gmail.com

mom_array=(0.5 1.0 2.0 5.0 10.0 15.0)
particle_array=("pi-")
filename=("tracking_output") 
etabin_array=(-3.5 -2.5 -1.0 1.0 2.5 3.5)
nevents=100

rm -rf truthseed/ realseed/ *.root
mkdir -p truthseed/pi-/mom realseed/pi-/mom truthseed/pi-/dca realseed/pi-/dca

# run the simulation
source ../epic/install/bin/thisepic.sh 
dd_web_display --export -o epic_craterlake_tracking_only.root ../epic/install/share/epic/epic_craterlake_tracking_only.xml
for ((i=0; i<${#mom_array[@]}; i++)); do
npsim --compactFile ../epic/install/share/epic/epic_craterlake_tracking_only.xml --outputFile sim${mom_array[i]}.edm4hep.root --numberOfEvents $nevents --enableGun --gun.thetaMin 3*deg --gun.thetaMax 177*deg --gun.distribution eta --gun.particle pi- --gun.momentumMin ${mom_array[i]}*GeV --gun.momentumMax ${mom_array[i]}*GeV --gun.multiplicity 1 --random.seed 100000
done
# run the reconstruction
source ../epic/install/setup.sh 
#source ../EICrecon/install/bin/eicrecon-this.sh

for ((i=0; i<${#mom_array[@]}; i++)); do
/opt/local/bin/eicrecon \
 -Pnthreads=1 \
 -Pjana:debug_plugin_loading=1 \
 -Pjana:nevents=$nevents \
 -Pacts:MaterialMap=calibrations/materials-map.cbor \
 -Ppodio:output_file="${filename}"_${mom_array[i]}.edm4eic.root \
 -Pdd4hep:xml_files=../epic/install/share/epic/epic_craterlake_tracking_only.xml   \
 -Ppodio:output_collections="MCParticles,CentralCKFTrajectories,CentralCKFTrackParameters,CentralCKFSeededTrackParameters,CentralTrackVertices" \
 sim${mom_array[i]}.edm4hep.root
done 

# run the analysis
for ((iparticle=0; iparticle<${#particle_array[@]}; iparticle++)); do
# truth seeding
for ((i=0; i<${#mom_array[@]}; i++)); do
#Form("./%s_%1.1f",mom)
root -b -l -q Tracking_Performances.C'("./'${filename}_$(printf "%.1f" ${mom_array[i]})'.edm4eic.root","'${particle_array[iparticle]}'",'${mom_array[i]}',0.15,"")'
done

# real seeding
for ((i=0; i<${#mom_array[@]}; i++)); do
root -b -l -q Tracking_Performances.C'("./'${filename}_$(printf "%.1f" ${mom_array[i]})'.edm4eic.root","'${particle_array[iparticle]}'",'${mom_array[i]}',0.15,"Seeded")'
done
done
cd truthseed/pi-/dca
hadd final_hist_dca_truthseed.root *.root
cd ../../../realseed/pi-/dca/
hadd final_hist_dca_realseed.root *.root
cd ../../../



