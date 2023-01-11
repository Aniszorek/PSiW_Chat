for i in *.c
do
	gcc -Wall "$i" -o "${i%.c}"
done
