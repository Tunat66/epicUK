IFS=$'\n'
for file in $(find . -name '*.gdml');
do
  mv -- "$file" "${file// /_}"
done
for file in $(find . -name '*.stl');
do
  mv -- "$file" "${file// /_}"
done
unset IFS 