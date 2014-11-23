echo ""
/usr/bin/time -f "%e real" ./gol 10000 ./inputs/1k.pbm ./outputs/1k_t.pbm
diff ./outputs/1k_t.pbm ./outputs/1k.pbm
echo ""  
