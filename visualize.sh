#now to visualize the geometry
echo "Pls enter the xml file you want to visualize as the first argument of the script"

dd_web_display -o $DETECTOR_PATH/detector_geometry.root --export $1
#root -l  $DETECTOR_PATH/root_visualize.C

