#!/bin/sh
int=1
while(( $int<=4000 ))
do
	./ChatClient 127.0.0.1 6000 &
	let "int++"
done
