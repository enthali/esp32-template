ARG DOCKER_TAG=v5.4.1
FROM espressif/idf:${DOCKER_TAG}

# Install additional tools for development
RUN apt-get update && apt-get install -y \
    git \
    curl \
    wget \
    nano \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Set up bash environment
RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

# Create workspace directory
WORKDIR /workspace

# Set proper permissions for ESP-IDF tools
RUN chmod -R 755 /opt/esp

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]
CMD ["/bin/bash", "-c"]
