ESP32 Template Documentation
============================

Welcome to the ESP32 Template project documentation. This template provides a complete development environment for ESP32 IoT projects with GitHub Codespaces, QEMU emulation, and professional requirements engineering methodology.

.. toctree::
   :maxdepth: 2
   :caption: Requirements Engineering:

   11_requirements/index
   12_design/index

.. toctree::
   :maxdepth: 2
   :caption: API Reference:

   21_api/index

.. toctree::
   :maxdepth: 2
   :caption: Traceability:

   31_traceability/index

About This Template
-------------------

This is a **template repository** designed to be:

* **Forked for new projects** - Starting point for ESP32 applications
* **Zero-setup development** - GitHub Codespaces with ESP-IDF pre-configured
* **Hardware optional** - QEMU emulation for testing without physical devices
* **Production-ready structure** - Component-based architecture following ESP-IDF best practices
* **Professional documentation** - Sphinx-Needs for requirements traceability

Key Features
------------

* **Component-based architecture** - Modular components in ``main/components/``
* **Example components provided**:
  
  * ``config_manager`` - NVS configuration storage patterns
  * ``web_server`` - HTTP server with captive portal
  * ``cert_handler`` - HTTPS certificate handling
  * ``netif_uart_tunnel`` - QEMU network bridge

* **Real-time OS** - FreeRTOS for task management
* **Memory optimized** - 4MB flash configuration

Documentation Structure
-----------------------

This documentation uses Sphinx with Sphinx-Needs extension for professional requirements engineering:

* **Requirements** - System and component requirements with traceability
* **Design** - Architecture and design specifications
* **API Reference** - Auto-generated from C code (Doxygen)
* **Traceability** - Automatic coverage and dependency tracking

For user guides, tutorials, and development documentation, see the `User Documentation <../mkdocs/index.html>`_ (MkDocs).

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`

All Requirements Overview
=========================

.. needlist::
   :show_status:
   :show_tags:
