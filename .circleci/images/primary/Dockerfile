FROM ubuntu:20.04

RUN apt-get update && apt-get install --no-install-recommends -y \
  make \
  gcc \
  gcc-arm-none-eabi \
  git \
  libc6-dev \
  libnewlib-arm-none-eabi \
  openssh-client \
  ruby \
 && rm -rf /var/lib/apt/lists/*