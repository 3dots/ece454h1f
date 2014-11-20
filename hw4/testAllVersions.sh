rm r rs
make clean
make randtrack
sampleSkip=50

./randtrack_reduction 1 $sampleSkip > r
sort -n r > rs

echo "Original randtrack results generated."

make clean
make randtrack_global_lock

for numProc in 1 2 4 
do
	rm rt rts	
	./randtrack_global_lock $numProc $sampleSkip > rt
	sort -n rt > rts

	diff rs rts
	echo "randtrack_global_lock success vs orginal with thread #: $numProc"
done

make clean
make randtrack_tm

for numProc in 1 2 4 
do
	rm rt rts	
	./randtrack_tm $numProc $sampleSkip > rt
	sort -n rt > rts

	diff rs rts
	echo "randtrack_tm success vs orginal with thread #: $numProc"
done

make clean
make randtrack_list_lock

for numProc in 1 2 4 
do
	rm rt rts	
	./randtrack_list_lock $numProc $sampleSkip > rt
	sort -n rt > rts

	diff rs rts
	echo "randtrack_list_lock success vs orginal with thread #: $numProc"
done

make clean
make randtrack_element_lock

for numProc in 1 2 4 
do
	rm rt rts	
	./randtrack_element_lock $numProc $sampleSkip > rt
	sort -n rt > rts

	diff rs rts
	echo "randtrack_element_lock success vs orginal with thread #: $numProc"
done

make clean
make randtrack_reduction

for numProc in 1 2 4 
do
	rm rt rts	
	./randtrack_reduction $numProc $sampleSkip > rt
	sort -n rt > rts

	diff rs rts
	echo "randtrack_reduction success vs orginal with thread #: $numProc"
done

