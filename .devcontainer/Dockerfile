ARG DOCKER_TAG=latest
FROM espressif/idf:${DOCKER_TAG}

ARG CONTAINER_USER=esp
ARG USER_UID=1050
ARG USER_GID=$USER_UID
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN groupadd --gid $USER_GID $CONTAINER_USER \
  && adduser --uid $USER_UID --gid $USER_GID --disabled-password --gecos "" ${CONTAINER_USER} \
  && usermod -a -G root $CONTAINER_USER && usermod -a -G dialout $CONTAINER_USER \
  && apt-get update && apt-get install -y sudo \
  && echo "${CONTAINER_USER} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers \
  && rm -rf /var/lib/apt/lists/*

USER ${CONTAINER_USER}
ENV USER=${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]