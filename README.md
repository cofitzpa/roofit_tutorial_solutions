# roofit tutorial solutions

Solutions to the [roofit_tutorial ](//github.com/cofitzpa/roofit_tutorial_solutions)


# Docker container

This tutorial is meant to run inside a docker container. You
can get the container with `docker pull betatim/roofit-tutorial`.
Start it with `docker run -ti --rm betatim/roofit-tutorial bash`.
This will drop you into a terminal inside the container so you
can look around and explore.

To build the container from scratch type: `docker built -t roofit-tutorial .`
in this directory. It will build a container called `roofit-tutorial`.
