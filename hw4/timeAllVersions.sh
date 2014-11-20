rm ./to ./garbage ./timing_results
make clean
make randtrack_list_lock
sampleSkip=50 

echo ""
for numProc in 1 2 4 
do	
	echo "randtrack_list_lock with thread #: $numProc" >> timing_results

	for runCount in 1 2 3 4 5
	do
	
	/usr/bin/time -f %e -o ./to ./randtrack_list_lock $numProc $sampleSkip > ./garbage
	cat ./to >> timing_results
	echo "randtrack_list_lock with thread #: $numProc Finished. Run count: $runCount"
	done
done

