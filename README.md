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

## Build docker and run

Build
```
$ docker build --tag sea5kg/game-server-mmo:v0.0.0 .
```

Run
```
$ docker run --rm -it -p -v `pwd`/data:/opt/src/data 1234:1234 sea5kg/game-server-mmo:v0.0.0 bash

```