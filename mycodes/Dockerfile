FROM ubuntu:16.04
RUN mkdir /hw4
RUN apt-get update && apt-get install -y make && apt-get install -y g++
RUN apt-get update && apt-get install -y sudo
RUN apt-get update && apt-get install -y postgresql
RUN apt-get update && apt-get install -y postgresql-contrib
RUN apt-get update && apt-get install -y libpqxx-4.0 libpq-dev libpqxx-dev

WORKDIR /hw4
ADD . /hw4
