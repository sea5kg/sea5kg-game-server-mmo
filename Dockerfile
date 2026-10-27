# stage 0: build binary
FROM sea5kg/sea5kg-game-server-mmo:build-environment-2026-10-28

COPY ./ /root/
WORKDIR /root/
RUN ./build_simple.sh

# stage 1: release
FROM sea5kg/sea5kg-game-server-mmo:release-environment-2026-10-28
LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea5kg/sea5kg-game-server-mmo"

RUN mkdir /root/data
COPY --from=0 /root/game-server-mmo /usr/bin/game-server-mmo

CMD ["game-server-mmo","start"]
