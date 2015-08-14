#!/bin/bash

set -e
workdir=`pwd`

container_id=`docker run -d -v $workdir:/home/jupyter/analysis -p 8888 betatim/roofit-tutorial ipython notebook --ip=0.0.0.0 --no-browser`

if hash boot2docker 2>/dev/null; then
    connect_string=`docker port $container_id 8888 | sed -e 's/0.0.0.0/'$(boot2docker ip)'/'`
else
    connect_string=`docker port $container_id 8888`
fi

echo "Once you are done run the following two commands to clean up:"
echo
echo "    docker stop "$container_id
echo "    docker rm "$container_id
echo
echo "To get started point your browser at: "$connect_string
