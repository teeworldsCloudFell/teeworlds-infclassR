# build enviroment
FROM alpine:3.13 AS build_env
RUN apk update && apk upgrade
RUN apk add --no-cache openssl gcc g++ make cmake python3 zlib-dev icu-dev libmaxminddb-dev

RUN cmake --version

WORKDIR /work
COPY . sources
WORKDIR /work/build
RUN cmake /work/sources -DCMAKE_INSTALL_PREFIX=/work/install -DSERVER_EXECUTABLE=server
RUN cmake --build . -t install

# runtime enviroment
FROM alpine:3.13 AS runtime_env
WORKDIR /infclass_srv/
RUN apk update && apk upgrade
RUN apk add --no-cache libstdc++ icu libmaxminddb
COPY --from=build_env /work/install .

EXPOSE 8303/udp
ENTRYPOINT ["./server"]
