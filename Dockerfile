FROM ubuntu:20.04

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Options for common package install script - SHA updated on release
ARG INSTALL_ZSH="true"
ARG UPGRADE_PACKAGES="true"
ARG COMMON_SCRIPT_SOURCE="https://raw.githubusercontent.com/microsoft/vscode-dev-containers/master/script-library/common-debian.sh"
ARG COMMON_SCRIPT_SHA="dev-mode"

# Configure apt and install common packages
RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends curl ca-certificates 2>&1 \
    && curl -sSL  ${COMMON_SCRIPT_SOURCE} -o /tmp/common-setup.sh \
    && ([ "${COMMON_SCRIPT_SHA}" = "dev-mode" ] || (echo "${COMMON_SCRIPT_SHA} */tmp/common-setup.sh" | sha256sum -c -)) \
    && /bin/bash /tmp/common-setup.sh "${INSTALL_ZSH}" "${USERNAME}" "${USER_UID}" "${USER_GID}" "${UPGRADE_PACKAGES}" \
    && rm /tmp/common-setup.sh \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

ARG PROTOBUF_TARGET_VERSION=3.13.0
ARG PROTOBUF_SOURCES=protobuf-${PROTOBUF_TARGET_VERSION}
ARG PROTOBUF_SOURCES_ZIP=protobuf-cpp-${PROTOBUF_TARGET_VERSION}.zip

RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get -y install --no-install-recommends build-essential autoconf automake libtool curl make g++ unzip cmake valgrind cppcheck gdb clang-format \
    # BUILD PROTOBUF
    && cd tmp \
    && wget https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_TARGET_VERSION}/${PROTOBUF_SOURCES_ZIP} -O ${PROTOBUF_SOURCES_ZIP} \
    && unzip ${PROTOBUF_SOURCES_ZIP} \
    && rm ${PROTOBUF_SOURCES_ZIP} \
    && cd ${PROTOBUF_SOURCES} \
    && ./configure \
    && make -j $(nproc) \
    # && make check \ # Very long... :( It works with ubuntu:20.04
    && sudo make install -j $(nproc) \
    && sudo ldconfig \
    && cd /tmp \
    && rm -rf protopbuf-cpp \
    # CLEANING UP
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

ARG CC=gcc
ARG CXX=g++

ENV CC ${CC}
ENV CXX ${CXX}

RUN mkdir -p /home/${USERNAME}/app
WORKDIR /home/${USERNAME}/app