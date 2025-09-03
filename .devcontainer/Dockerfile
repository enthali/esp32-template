ARG DOCKER_TAG=v5.4.1
FROM espressif/idf:${DOCKER_TAG}

# Install additional tools for development and clean up in same layer to reduce image size
RUN apt-get update && apt-get install -y \
    git \
    curl \
    wget \
    nano \
    vim \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /tmp/* \
    && rm -rf /var/tmp/*

# Set up bash environment
RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

# Create workspace directory
WORKDIR /workspace

# Remove unnecessary ESP-IDF components to save space (Codespaces optimization)
RUN if [ -d "/opt/esp/tools/qemu-xtensa" ]; then rm -rf /opt/esp/tools/qemu-xtensa; fi \
    && if [ -d "/opt/esp/tools/esp-rom-elfs" ]; then find /opt/esp/tools/esp-rom-elfs -name "*.elf" -not -name "*esp32_*" -delete; fi \
    && if [ -d "/opt/esp/tools/esp32ulp-elf" ]; then rm -rf /opt/esp/tools/esp32ulp-elf; fi \
    && if [ -d "/opt/esp/tools/riscv32-esp-elf" ]; then rm -rf /opt/esp/tools/riscv32-esp-elf; fi \
    && if [ -d "/opt/esp/tools/xtensa-esp32s2-elf" ]; then rm -rf /opt/esp/tools/xtensa-esp32s2-elf; fi \
    && if [ -d "/opt/esp/tools/xtensa-esp32s3-elf" ]; then rm -rf /opt/esp/tools/xtensa-esp32s3-elf; fi \
    && find /opt/esp -name "*.a" -size +10M -delete 2>/dev/null || true \
    && find /opt/esp -name "*.so" -size +10M -delete 2>/dev/null || true

# Set minimal permissions only where needed (avoid chmod -R on huge directory)
RUN chmod +x /opt/esp/idf/export.sh \
    && chmod +x /opt/esp/entrypoint.sh \
    && chmod -R 755 /opt/esp/python_env 2>/dev/null || true

# Prevent auto-added features from running problematic commands
RUN touch /opt/esp/.permissions_set

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]
CMD ["/bin/bash", "-c"]
