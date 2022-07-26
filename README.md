# Item assignment solver for crowdsourcing tasks

This repository contains the solver that has been used for the advanced laboratory within the course of Web Information Retrieval.

The solver allows to solve input instances about an item assignment optimization problem during the creation of crowdsourcing tasks. 
The Docker compose file inside the repository allows to deploy both the solver and also a reverse proxy, based on nginx, that forwards incoming requests to the solver.

## How to deploy the solver
To deploy the solver run the following commands on the terminal:
```
    docker-compose build
    docker-compose up
```
When the solver is deployed, it replies to requests coming from the port 80.
This can be changed by changing the port of the reverse proxy service inside the docker-compose file.