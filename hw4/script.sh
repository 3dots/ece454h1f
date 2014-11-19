make clean
make randtrack_element_lock

for numProc in 1 2 4
do
	rm rg rgs	
	./randtrack_element_lock $numProc 50 > rg
	sort -n rg > rgs

	diff rs rgs
	echo "Tested vs orginal with proc: $numProc"
done



