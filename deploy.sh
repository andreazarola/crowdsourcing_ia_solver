#!/bin/bash
if [ -z "$1" ]; then
  export ENV=prod
else
  export ENV=$1
fi
export BUILD=ia_solver
echo "Building and deploying ${BUILD}_${ENV} into $ENV environment"
docker-compose build --force-rm ${BUILD}_${ENV} && docker-compose push ${BUILD}_${ENV}
# if [ $? -eq 0 ]; then
#   ssh localadmin@satt.diegm.uniud.it << EOSSH
# cd steel-production
# echo "Remotely pulling and deploying $BUILD into $ENV environment"
# echo "docker-compose pull ${BUILD}_${ENV} && docker-compose stop ${BUILD}_${ENV} && docker-compose up --no-start ${BUILD}_${ENV} && docker-compose start ${BUILD}_${ENV}"
# docker-compose pull ${BUILD}_${ENV} && docker-compose stop ${BUILD}_${ENV} && docker-compose up --no-start ${BUILD}_${ENV} && docker-compose start ${BUILD}_${ENV}
# EOSSH
# fi
