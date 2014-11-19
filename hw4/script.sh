numProc=1

for numProc in 1 2 4
make clean
rm rg rgs

make randtrack_global_lock
./randtrack numProc 50 > rg
sort -n rg > rgs

diff r rg
echo "Tested vs orginal with proc: " $numProc




