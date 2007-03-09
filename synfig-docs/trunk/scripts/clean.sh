LANGSAVAL="en ca"

for i in $LANGSAVAL
	do
	make SGMLDIR=$i clean
	rm -Rf result/
done
