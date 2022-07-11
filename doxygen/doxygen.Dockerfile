FROM registry.gitlab.com/kicad/kicad-ci/source_containers/master/fedora:36 as build-doxygen-env
USER root

WORKDIR /src
COPY . ./

RUN ls
RUN mkdir build && cd build

WORKDIR /src/build

RUN cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DKICAD_USE_OCC=ON \
      -DKICAD_SPICE=ON
RUN make doxygen-docs


FROM scratch as output-image

COPY --from=build-doxygen-env /src/doxygen/out/html /doxygen-docs_html
