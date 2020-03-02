# /bin/bash
# This script builds pirate-llvm in a docker image.
#
# The compiler will be stored in /dist
#
# This should be run from root of git repo.  It will leave artifacts in the
# "dist" directory."
#
# If build fails the container will be left running so that it can be
# inspected.
set -e

mkdir -p cache

docker build -t pirateteam/build images/build

container=$(docker create \
       -i \
       --cpus=2 \
       --memory=32G \
       --mount type=bind,src=`pwd`/images/llvm-ubuntu,dst=/root/dist \
       --mount type=bind,src=`pwd`/cache,dst=/root/cache \
       --mount type=bind,src=`pwd`,dst=/root/pirate-llvm,ro \
        -w /root \
       pirateteam/build)
echo "Created container: $container"
docker start $container
docker exec $container /root/pirate-llvm/scripts/docker-internal.sh Release
docker stop $container
docker container rm $container
