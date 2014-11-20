sampleSkip=50

rm ./to ./garbage ./timing_results


make clean
make randtrack
echo "Starting randtrack" >> timing_results
echo "" >> timing_results
echo ""
	
echo "randtrack" >> timing_results

for runCount in 1 2 3 4 5
do

	/usr/bin/time -f %e -o ./to ./randtrack 1 $sampleSkip > ./garbage
	cat ./to >> timing_results
	echo "randtrack Finished. Run count: $runCount"
done

echo "" >> timing_results
echo ""

echo ""
echo "" >> timing_results

make clean
make randtrack_global_lock
echo "Starting randtrack_global_lock" >> timing_results
echo "" >> timing_results
echo ""
for numProc in 1 2 4 
do	
	echo "randtrack_global_lock with thread #: $numProc" >> timing_results

	for runCount in 1 2 3 4 5
	do
	
		/usr/bin/time -f %e -o ./to ./randtrack_global_lock $numProc $sampleSkip > ./garbage
		cat ./to >> timing_results
		echo "randtrack_global_lock with thread #: $numProc Finished. Run count: $runCount"
	done
	
	echo "" >> timing_results
	echo ""
done
echo ""
echo "" >> timing_results

make clean
make randtrack_tm
echo "Starting randtrack_tm" >> timing_results
echo "" >> timing_results
echo ""
for numProc in 1 2 4 
do	
	echo "randtrack_tm with thread #: $numProc" >> timing_results

	for runCount in 1 2 3 4 5
	do
	
		/usr/bin/time -f %e -o ./to ./randtrack_tm $numProc $sampleSkip > ./garbage
		cat ./to >> timing_results
		echo "randtrack_tm with thread #: $numProc Finished. Run count: $runCount"
	done
	
	echo "" >> timing_results
	echo ""
done
echo ""
echo "" >> timing_results

make clean
make randtrack_list_lock
echo "Starting randtrack_list_lock" >> timing_results
echo "" >> timing_results
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
	
	echo "" >> timing_results
	echo ""
done
echo ""
echo "" >> timing_results

make clean
make randtrack_element_lock
echo "Starting randtrack_element_lock" >> timing_results
echo "" >> timing_results
echo ""
for numProc in 1 2 4 
do	
	echo "randtrack_element_lock with thread #: $numProc" >> timing_results

	for runCount in 1 2 3 4 5
	do
	
		/usr/bin/time -f %e -o ./to ./randtrack_list_lock $numProc $sampleSkip > ./garbage
		cat ./to >> timing_results
		echo "randtrack_element_lock with thread #: $numProc Finished. Run count: $runCount"
	done
	
	echo "" >> timing_results
	echo ""
done
echo ""
echo "" >> timing_results

make clean
make randtrack_reduction
echo "Starting randtrack_reduction" >> timing_results
echo "" >> timing_results
echo ""
for numProc in 1 2 4 
do	
	echo "randtrack_reduction with thread #: $numProc" >> timing_results

	for runCount in 1 2 3 4 5
	do
	
		/usr/bin/time -f %e -o ./to ./randtrack_list_lock $numProc $sampleSkip > ./garbage
		cat ./to >> timing_results
		echo "randtrack_reduction with thread #: $numProc Finished. Run count: $runCount"
	done
	
	echo "" >> timing_results
	echo ""
done
echo ""
echo "" >> timing_results
