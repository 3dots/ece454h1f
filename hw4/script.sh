
numProc=1
 
make clean
rm rg rgs

make randtrack_global_lock
./randtrack $numProc 50 > rg
sort -n rg > rgs

diff rs rgs
echo "Tested vs orginal with proc: $numProc"



