
make clean

make randtrack_global_lock
./randtrack 1 50 > rg
sort -n rg > rgs

diff r rg
echo "Tested vs orginal"




