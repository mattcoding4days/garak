# using ubuntu base image
FROM ubuntu:22.04

# Use default answers for all questions.
ENV DEBIAN_FRONTEND noninteractive

# Upgrade the system
RUN apt-get update && apt-get upgrade -y --allow-downgrades

# install basics
RUN apt-get -y install lsb-release wget software-properties-common

# Install generlized tooling
RUN apt-get -y install cmake build-essential python3-pip checkinstall vim git

# Install LLVM (clang toolchain) and formatters
RUN apt-get -y install clang clang-format clang-tidy cmake-format

# Install with pip
RUN pip install  distro

# Copy the files to the image
COPY ../cmake /libgarak/cmake
COPY ../garak /libgarak/garak
COPY ../CMakeLists.txt /libgarak/CMakeLists.txt

# copy clang config files
COPY ../.clang-format /starter
COPY ../.clang-tidy /starter
COPY ../.cmake-format.yaml /starter

# Copy the python devkit tooling
COPY ../devkit /libgarak/devkit
COPY ../tooling /libgarak/tooling

WORKDIR /libgarak

# format the project
RUN ./devkit clang --format

# Build Entire project with tests and examples on
RUN ./devkit compile --clang

# Statically analyze the project
RUN ./devkit clang --lint

# Run tests. If you did not build a project,
# make sure to comment out the tests for that project
RUN cd build/garak && ctest -VV
RUN ./build/garak/examples/.bin
