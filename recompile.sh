echo "Note: this script requires you to have the cmake files setting the install prefix, otherwise it wont work"
echo "in case of a corrputed build/ directory, run the command: cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install"
cmake --build build -- install
#make -C build
#now to visualize the geometry currently disabled
#dd_web_display --export $DETECTOR_PATH/epic_vertex_only.xml
#root -l  $DETECTOR_PATH/root_visualize.C

