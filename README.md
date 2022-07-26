# Item assignment solver for crowdsourcing tasks

This repository contains the solver used for the advanced laboratory done for the course of Web Informatio Retrieval.

The Docker compose file inside the repository allows to deploy the solver and a reverse proxy inside a Docker container.

## How to deploy the solver
To deploy the solver run the following commands on the terminal:
```
    docker-compose build
    docker-compose up
```
When the solver is deployed, it reply to requests coming from the port 80.
This can be changed by changing the port of the reverse proxy service inside the docker-compose file.