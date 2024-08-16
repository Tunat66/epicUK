IFS=$'\n'
for file in $(find . -name '*.gdml');
do
  mv -- "$file" "${file// /_}"
done
unset IFS 