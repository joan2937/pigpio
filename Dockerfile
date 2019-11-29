FROM debian:buster AS builder

RUN apt-get update \
  && apt-get install -y make gcc \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /workdir
COPY . /workdir
RUN make install

FROM debian:buster-slim

COPY --from=builder /usr/local/lib/libpigpio.so.1 /usr/local/lib/libpigpio.so.1
COPY --from=builder /usr/local/bin/pigpiod /usr/local/bin/pigpiod

RUN ldconfig

CMD ["/usr/local/bin/pigpiod", "-g"]
