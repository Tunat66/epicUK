#!/bin/bash
# Script; shyam kumar; INFN Bari, Italy
# Modified: Tuna Tasali. University of Oxford
# shyam.kumar@ba.infn.it; shyam055119@gmail.com
# first argument is the name of the folder in which the results will go

#some initial setup
#../eic-shell
source install/setup.sh

#mom_array=(0.5 0.75 1.0 1.25 1.75 2.0 2.50 3.0 4.0 5.0 7.0 8.5 10.0 12.5 15.0)
mom_array=(0.5)
particle_array=("pi-")
filename=("tracking_output") 
etabin_array=(-3.5 -2.5 -1.0 1.0 2.5 3.5)
nevents=100
results_dir="Test_results"
compact_file_name="epic_craterlake_tracking_only.xml"

#rm -rf truthseed/ realseed/ *.root
if [ -d "$results_dir" ]; then
    echo "Directory '$results_dir' already exists. Aborting program to avoid overwriting data."
    exit 1
else
    mkdir $results_dir
    echo "Directory '$results_dir' is created."
fi 
mkdir -p $results_dir/truthseed/pi-/mom $results_dir/realseed/pi-/mom $results_dir/truthseed/pi-/dca $results_dir/realseed/pi-/dca

#create a new array which has the momenta formatted to two decimal places
decimal_places=2
formatted_mom_array=()
for value in "${mom_array[@]}"; do
    formatted_mom=$(printf "%.${decimal_places}f" "$value")
    formatted_mom_array+=("$formatted_mom")
done

# run the simulation
source ../epic/install/bin/thisepic.sh 
#dd_web_display --export -o epic_craterlake_tracking_only.root ../epic/install/share/epic/epic_craterlake_tracking_only.xml
for ((i=0; i<${#mom_array[@]}; i++)); do
    npsim --compactFile ../epic/install/share/epic/$compact_file_name --outputFile $results_dir/sim${formatted_mom_array[i]}.edm4hep.root --numberOfEvents $nevents --enableGun --gun.thetaMin 3*deg --gun.thetaMax 177*deg --gun.distribution eta --gun.particle pi- --gun.momentumMin ${mom_array[i]}*GeV --gun.momentumMax ${mom_array[i]}*GeV --gun.multiplicity 1 --random.seed 100000
done
# run the reconstruction
source ../epic/install/setup.sh 
#source ../EICrecon/install/bin/eicrecon-this.sh

for ((i=0; i<${#formatted_mom_array[@]}; i++)); do
#this bit I added to make sure the default eic-shell eicrecon is running
/opt/local/bin/eicrecon -Pnthreads=1 -Pjana:debug_plugin_loading=1 -Pjana:nevents=$nevents \
 -Pacts:MaterialMap=calibrations/materials-map.cbor \
 -Ppodio:output_file=$results_dir/"${filename}"_${mom_array[i]}.edm4eic.root \
 -Pdd4hep:xml_files=../epic/install/share/epic/$compact_file_name   \
 -Ppodio:output_collections="MCParticles,CentralCKFTrajectories,CentralCKFTrackParameters,CentralCKFSeededTrackParameters,CentralTrackVertices" \
$results_dir/sim${formatted_mom_array[i]}.edm4hep.root
done 
#note that it is absolutely crucial that the line $results_dir has no indentation!!!!!

#copy the analysis scripts to the new directory, this is to make it clear which scripts are used for the analysis
cp Tracking_Performances.C $results_dir
cp doCompare_truth_real_widebins_dcaT.C $results_dir
cp doCompare_truth_real_widebins_dcaz.C $results_dir
cp doCompare_truth_real_widebins_mom.C $results_dir
cd $results_dir

## run the analysis
#for ((iparticle=0; iparticle<${#particle_array[@]}; iparticle++)); do
## truth seeding
#for ((i=0; i<${#mom_array[@]}; i++)); do
##Form("./%s_%1.1f",mom)
#root -b -l -q Tracking_Performances.C'("./'${filename}_$(printf "%.2f" ${mom_array[i]})'.edm4eic.root","'${particle_array[iparticle]}'",'${mom_array[i]}',0.15,"")'
#done
#
## real seeding
#for ((i=0; i<${#mom_array[@]}; i++)); do
#root -b -l -q Tracking_Performances.C'("./'${filename}_$(printf "%.2f" ${mom_array[i]})'.edm4eic.root","'${particle_array[iparticle]}'",'${mom_array[i]}',0.15,"Seeded")'
#done
#done
#cd truthseed/pi-/dca
#hadd final_hist_dca_truthseed.root *.root
#cd ../../../realseed/pi-/dca/
#hadd final_hist_dca_realseed.root *.root
#cd ../../../
#
#rm -rf Final_Results/ Debug_Plots/ 
#mkdir -p Final_Results/pi-/mom Final_Results/pi-/dca  Debug_Plots/truth/pi-/mom  Debug_Plots/truth/pi-/dca  Debug_Plots/real/pi-/mom Debug_Plots/real/pi-/dca
## loop over particles
#for ((iparticle=0; iparticle<${#particle_array[@]}; iparticle++)); do
#for ((i=0; i<${#etabin_array[@]}-1; i++)); do
#xmax_hist=0.3 
#if [ $i == 2 || $i == 1 ]; then 
#xmax_hist=0.01 
#fi
#root -b -l -q doCompare_truth_real_widebins_mom.C'("'${particle_array[iparticle]}'",'${etabin_array[i]}','${etabin_array[i+1]}','$xmax_hist', '${#formatted_mom_array[@]}', '$formatted_mom_array')'
#
#
#
#root -b -l -q doCompare_truth_real_widebins_dcaz.C'("'${particle_array[iparticle]}'",'${etabin_array[i]}','${etabin_array[i+1]}')'
#root -b -l -q doCompare_truth_real_widebins_dcaT.C'("'${particle_array[iparticle]}'",'${etabin_array[i]}','${etabin_array[i+1]}','$xmax_hist')'
#done
#done


