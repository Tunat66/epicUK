echo "Note: the requires you to have the cmake files setting the install prefix, otherwise it wont work"
cmake --build build -- install
#now to visualize the geometry
dd_web_display --export $DETECTOR_PATH/epic_vertex_only.xml
root -l  $DETECTOR_PATH/root_visualize.C

