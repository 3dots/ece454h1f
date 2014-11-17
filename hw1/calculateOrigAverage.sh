vprArgs="iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0"

timeSum=0

cd ./orig/
for i in {1..10}
do
	/usr/bin/time -f %U -o ../temp.txt ./vpr $vprArgs
	tempValue=$(cat ../temp.txt | bc -l)
	echo "Current is= $tempValue" >> ../origResults.txt
	#The following is a fucking timeSum+=timeResult statement. Pointer arithmetic in bash...
	timeSum=$(echo "$tempValue + $timeSum" | bc -l)
done

cd ..
echo "Average is=" >> ./origResults.txt
origAverage=$(echo "$timeSum / 10.0" | bc -l)
echo $origAverage >> ./origResults.txt
echo $origAverage > ./origAverage.txt




