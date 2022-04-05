FROM brainboxdotcc/dpp:latest

WORKDIR /usr/src/crektlays

COPY . .

WORKDIR /usr/src/crektlays/build

RUN cmake ..
RUN make -j$(nproc)

ENTRYPOINT [ "/usr/src/crektlays/build/crektlays" ]

