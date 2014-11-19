
for numProc in 1 2 4
do
	make clean
	rm rg rgs

	make randtrack_global_lock
	./randtrack_global_lock $numProc 50 > rg
	sort -n rg > rgs

	diff rs rgs
	echo "Tested vs orginal with proc: $numProc"
done



