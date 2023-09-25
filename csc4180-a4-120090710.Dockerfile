FROM ubuntu:18.04

RUN apt-get clean 
RUN apt-get update
RUN apt-get install make -y
RUN apt-get install gcc -y
RUN apt-get install g++ -y
