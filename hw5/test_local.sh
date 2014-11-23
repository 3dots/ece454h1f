echo ""
/usr/bin/time -f "%e real" ./gol 500 ./inputs/512.pbm ./outputs/512_500_t.pbm
diff ./outputs/512_500_t.pbm ./outputs/512_500.pbm
echo ""
