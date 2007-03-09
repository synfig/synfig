LANGSAVAL="en ca"
TARGETS="pdf multiple-html ps"
if [ -n "$1" ]; then
	LANGSAVAL=$1
fi
if [ -n "$2" ]; then
	TARGETS=$2
fi
for i in $LANGSAVAL
	do
	echo =========== LANG: $i ===============
	for j in $TARGETS
		do
		echo =------ TARGET: $j ----------
		if [ ! -d "result/$i" ]; then
			mkdir -p "result/$i"
			mkdir -p "result/$i/out-$j"
		fi
		make SGMLDIR=$i $j
		mv result/$i/tmp-$j/*.$j result/$i/out-$j 2>/dev/null
	done
done
