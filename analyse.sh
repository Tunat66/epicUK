rm -rf Final_Results/ Debug_Plots/ 
mkdir -p Final_Results/pi-/mom Final_Results/pi-/dca  Debug_Plots/truth/pi-/mom  Debug_Plots/truth/pi-/dca  Debug_Plots/real/pi-/mom Debug_Plots/real/pi-/dca
# loop over particles
for ((iparticle=0; iparticle<${#particle_array[@]}; iparticle++)); do
for ((i=0; i<${#etabin_array[@]}-1; i++)); do
xmax_hist=0.3 
if [ $i == 2 || $i == 1 ]; then 
xmax_hist=0.01 
fi
root -b -l -q doCompare_truth_real_widebins_mom.C'("'${particle_array[iparticle]}'",'${etabin_array[i]}','${etabin_array[i+1]}','$xmax_hist')'
done
done