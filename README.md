# vv web game (in progress)

[![Total alerts](https://img.shields.io/lgtm/alerts/g/sea-kg/vv0r1d-wmso.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/sea-kg/vv0r1d-wmso/alerts/) [![Language grade: JavaScript](https://img.shields.io/lgtm/grade/javascript/g/sea-kg/vv0r1d-wmso.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/sea-kg/vv0r1d-wmso/context:javascript)


![Alt text](/contrib/screenshot.jpg?raw=true "Screenshot 01")


## front end

canvas + javascript


## Backend

c++ cmake

```
$ ./build_simple.sh
```

## Included

- https://github.com/ithewei/libhv/
- https://www.sqlite.org/

## References

Sprites for players:

* https://untamed.wild-refuge.net/rmxpresources.php?characters
* https://opengameart.org/content/2d-effects-0


Create a `~/my-mmo-game/docker-compose.yml` file with the following content:
```yml
version: '3'

services:
  ctf01d_jury:
    image: sea5kg/sea5kg-game-server-mmo:v0.0.1
    volumes:
      - "./gs_mmo_data:/data"
    environment:
      GS_MMO_WORKDIR: "/data"
      GS_MMO_USER: 1000  # automatically will be changed rights to folder and files.
      GS_MMO_PORT: 11223
    expose:
      - "11223"
    ports:
      - "11223:11223"
    # restart: always
    networks:
      - sea5kg_game_server_mmo_net

networks:
  sea5kg_game_server_mmo_net:
    driver: bridge
```


## Build docker and run

Build
```
$ docker build --tag sea5kg/sea5kg-game-server-mmo:v0.0.1 .
```

Run
```
$ docker run --rm -it -p -v `pwd`/data:/opt/src/data 1234:1234 sea5kg/game-server-mmo:v0.0.0 bash
```

### Helpers

```
docker build --rm --tag sea5kg/sea5kg-game-server-mmo:build-environment-2026-10-28 -f Dockerfile.build-environment .
docker build --rm --tag sea5kg/sea5kg-game-server-mmo:release-environment-2026-10-28 -f Dockerfile.release-environment .
```


