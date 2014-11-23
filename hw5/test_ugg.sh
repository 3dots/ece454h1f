echo ""
/usr/bin/time -f "%e real" ./gol 2000 ./inputs/512.pbm ./outputs/512_2000_t.pbm
diff ./outputs/512_2000_t.pbm ./outputs/512_2000.pbm
echo "" 
