ARG DOCKER_TAG=v5.4.1
FROM espressif/idf:${DOCKER_TAG}

# Set up bash environment for ESP-IDF  
RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

# Install minimal development tools
RUN apt-get update && apt-get install -y \
    git \
    curl \
    nano \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]
CMD ["/bin/bash", "-c"]
