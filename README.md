<div align="center">
    <h1>libgarak</h1>
</div>

[![Unittests](https://github.com/mattcoding4days/garak/actions/workflows/cmake.yml/badge.svg)](https://github.com/mattcoding4days/garak/actions/workflows/cmake.yml)

## :information_source: About 

> A lightweight async tcp framework written in C++20 using boost asio

## :building_construction: Development

### Compiling
> Standard cmake rules apply, the following commands should be made from the root of the project

1. `cmake -B build` Generate the make files, fetch build dependencies
2. `cmake --build build -j$(nproc)` Compile the code

### Using the `devkit` tool
Please see [this documentation](https://github.com/mattcoding4days/cmake-starter#using-the-devkit-tool) for indepth devkit usage
     
## :computer: Docker

1. Build.Dockerfile builds and runs all tests in the project, simulates a ci/cd pipeline
2. Remote-Dev.Dockerfile is for remote development through Clion or vscode

### :keyboard: Commands

#### Running the Build.Dockerfile

> This image is for running tests, simulates what would happen in a ci/cd pipeline

```bash
# Build the container (can be used to rebuild image after code changes)
docker image build -t garak:v0.1 -f dev_containers/Build.Dockerfile .

# Rebuild with no cache
docker image build --no-cache -t garak:v0.1 -f dev_containers/Build.Dockerfile.

# Run the container interactively
docker container run -it garak:v0.1

# Run non interactively
docker container run garak:v0.1
```

#### Running Remote-Dev.Dockerfile

> Follow this [Clion Docker tutorial](https://blog.jetbrains.com/clion/2020/01/using-docker-with-clion/) to set up your environment to connect to this container.
> Follow this [Clion Full remote mode ](https://www.jetbrains.com/help/clion/remote-projects-support.html) to set up Clion for remote development.

```bash
# Build the image
docker build -t garak/remote-cpp-env:0.1 -f dev_containers/Remote-Dev.Dockerfile .

# Once built you can run the container
docker run -d --cap-add sys_ptrace -p127.0.0.1:2222:22 --name garak_remote_env garak/remote-cpp-env:0.1

# Ports are temporarily mapped, and maybe reused, its usually worth clearing
# any cached ssh keys
ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:2222"
```
> When setting up the ssh credentials in your Editor you will need the following information,
> to connect to your newly created docker container.

1. SSH port: 2222
2. Username: user
3. Password: password

## :package: 3rd party packages

1. [gtest](https://github.com/google/googletest)
  - Googles Unit test framework which also includes a mock library
  - Why? It compiles way faster than catch2, has a built-in mocking framework, and blends nicely
    with the Google benchmark library
