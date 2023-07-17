# CIMIN
CIMIN: Crashing Input MINimizer

### Contributors
* 21900458 Gunmin Yoo
* 22100113 Seongbin Kim
___
# Usage
1. Build the program
```
$ sh build.sh
```
2. Execute the created binary file
```
./bin/cimin <options>
	-i	: a file path of the crashing input
	-m 	: a string whose appearance in standard error\
			determines whether the expected crash occurs or not
	-o 	: a new file path to store the reduced crashing input
	./a.out : a file path to the executable binary of the target program
```
* Example:
```
./bin/cimin -i ./target_programs/jsmn/testcases/crash.json -m "heap-buffer-overflow" -o ./output/reduced ./target_programs/jsmn/jsondump
```
3. Check the results in the output path
```
vi <output_filepath>
```

