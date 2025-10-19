Architecture Documentation
==========================

This section provides high-level architectural overview of the ESP32 Template, focusing on generic patterns applicable to any ESP32 IoT application.

.. toctree::
   :maxdepth: 2
   :caption: Architecture:

   overview

Overview
--------

The architecture documentation describes the **overall system structure** and **design patterns** used in the template:

* **System Overview** - High-level template architecture
* **Component Organization** - How components are structured
* **Data Flow Patterns** - How data moves through the system
* **Design Principles** - Architectural decisions and rationale
* **QEMU Emulation** - Hardware-free development architecture
* **Threading Model** - FreeRTOS task organization
* **Memory Management** - Flash and heap usage strategies

Purpose
-------

This documentation helps users understand:

* **Template Structure**: How the template is organized
* **Design Patterns**: Reusable patterns for ESP32 applications
* **Customization Points**: Where and how to add your own code
* **QEMU Integration**: How emulation support works
* **Best Practices**: Recommended approaches for ESP32 development

Relationship to Other Documentation
------------------------------------

.. code-block:: text

   Architecture (10_architecture/)
        ↓ implements
   Requirements (11_requirements/)
        ↓ detailed by
   Design Specs (12_design/)
        ↓ realized in
   Implementation (main/, components/)

Navigation
----------

Start with :doc:`overview` for a comprehensive architectural overview of the ESP32 Template.
