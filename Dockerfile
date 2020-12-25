# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.154.0/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="buster"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends default-jdk default-jre

RUN cd /usr/local/lib \
    && curl -O https://www.antlr.org/download/antlr-4.9-complete.jar \
    && echo "export CLASSPATH=\".:/usr/local/lib/antlr-4.9-complete.jar:$CLASSPATH\" \n\
    alias antlr4='java -Xmx500M -cp \"/usr/local/lib/antlr-4.9-complete.jar:$CLASSPATH\" org.antlr.v4.Tool' \n\
    alias grun='java -Xmx500M -cp \"/usr/local/lib/antlr-4.9-complete.jar:$CLASSPATH\" org.antlr.v4.gui.TestRig' \n\
    " >> /home/vscode/.bashrc