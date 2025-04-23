#to create .o files:
mkdir -p obj
gcc -std=gnu11 -Wall -Werror -c codes/lex.c -o obj/lex.o 
gcc -std=gnu11 -Wall -Werror -c codes/parse.c -o obj/parse.o
gcc -std=gnu11 -Wall -Werror -c codes/run.c -o obj/run.o
gcc -std=gnu11 -Wall -Werror -c codes/main.c -o obj/main.o
gcc -o interpret obj/lex.o obj/parse.o obj/run.o obj/main.o

#to run the codes:
./interpret examples/filename.txt

./interpret examples/countdigits.txt 
./interpret examples/factorial.txt  
./interpret examples/fibonacci.txt
./interpret examples/grades.txt
./interpret examples/largestof3.txt
./interpret examples/power.txt
./interpret examples/printeven.txt
./interpret examples/reverse.txt
./interpret examples/smallestof3.txt
./interpret examples/sum.txt
./interpret examples/swap.txt
./interpret examples/tables.txt
./interpret examples/array1.txt
./interpret examples/array2.txt
./interpret examples/array3.txt