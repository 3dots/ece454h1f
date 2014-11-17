vprArgs="iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0"


origAverage=$(cat ./origAverage.txt | bc -l)
echo "Orig Average is= $tempValue" >> ../results.txt


timeSum=0

echo "Running New" >> ./results.txt
cd ./new/
for i in {1..5}
do
	/usr/bin/time -f %U -o ../temp.txt ./vpr $vprArgs
	tempValue=$(cat ../temp.txt | bc -l)
	echo "Current is= $tempValue" >> ../results.txt
	#The following is a fucking timeSum+=timeResult statement. Pointer arithmetic in bash...
	timeSum=$(echo "$tempValue + $timeSum" | bc -l)
done

cd ..
echo "Average is=" >> ./results.txt
newAverage=$(echo "$timeSum / 5.0" | bc -l)
echo $newAverage >> ./results.txt

echo "Ratio of Original over New =" >> ./results.txt
echo "$origAverage / $newAverage" | bc -l >>./results.txt








