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

RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get -y install --no-install-recommends build-essential cmake valgrind cppcheck gdb clang-format \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

ARG CC=gcc
ARG CXX=g++

ENV CC ${CC}
ENV CXX ${CXX}

RUN mkdir -p /home/${USERNAME}/app
WORKDIR /home/${USERNAME}/app