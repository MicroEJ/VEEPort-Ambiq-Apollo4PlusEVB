.. 
	Copyright 2021-2023 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.

.. |BOARD_NAME| replace:: AMAP4PEVB
.. |VEE_PORT_VER| replace:: 1.0.1
.. |RCP| replace:: MICROEJ SDK
.. |VEE_PORT| replace:: MicroEJ VEE Port
.. |VEE_PORTS| replace:: MicroEJ VEE Ports
.. |SIM| replace:: MicroEJ Simulator
.. |ARCH| replace:: MicroEJ Architecture
.. |CIDE| replace:: MICROEJ SDK
.. |RTOS| replace:: FreeRTOS RTOS
.. |MANUFACTURER| replace:: Ambiq

.. _README: ./../../../README.rst
.. _RELEASE NOTES: ./../../../RELEASE_NOTES.rst
.. _CHANGELOG: ./../../../CHANGELOG.rst

================
|BOARD_NAME| BSP
================

This project contains the BSP sources of the |VEE_PORT| for the
|BOARD_NAME|.

This document does not describe how to setup the |VEE_PORT|.  Please
refer to the `README`_ for that.

Board Configuration
-------------------

|BOARD_NAME| provides several connectors, each connector is used by the MicroEJ Core Engine itself or by a foundation library.

Mandatory Connectors
~~~~~~~~~~~~~~~~~~~~

|BOARD_NAME| provides a multi function USB port used as:

- Power supply connector
- Probe connector
- Virtual COM port

Ensure the Power Supply jumper J3 is fit to the second option: EXT link (default setting); and set the power switch to the on settings.
Then just plug a mini USB Type-C cable from a computer to the JLINK-USB port of the board, to be able to program an application on it and see the traces.

For a detailed Power Supply setup check the user manual and Quick start on |MANUFACTURER| website under `Necessary documents <https://ambiq.com/apollo4-plus/>`__.

CPU clock
---------

The CPU clock can be put at either at :
- 192 MHz use the IAR flag HIGH_PERFORMANCE_MODE (default IAR configuration).
- 96 MHz without the flag above flag.

Build & Run Scripts
---------------------

In the folder ``project/microej/iar/scripts/`` 
for the IAR toolchain are scripts that can be used to build and flash the BSP.

- The ``build.bat`` scripts are used to compile and link the BSP with a
  MicroEJ Application to produce a MicroEJ Firmware
  (``application.out``) that can be flashed on a device.

- The ``run.bat`` scripts are used to flash a MicroEJ Firmware
  (``application.out``) on a device.

These scripts work out of the box, assuming the toolchain is
installed in the default path.

The following environment variables are customizable:

**IAR toolchain**

- ``IAREW_INSTALLATION_DIR``: The path to IAR installation directory (already set to the default IAR Workbench default installation directory).
- ``IAREW_PROJECT_CONFIGURATION``: The project configuration (``Debug`` or ``Release``).
- ``IAREW_PROJECT_DIR``: The directory that contains the ``application.eww`` IAR project file (set to ``%~dp0``: the directory that contains the executed ``.bat``).
- ``IAREW_PROJECT_NAME``: The Eclipse CDT project name (``application`` by default).

The environment variables can be defined globally by the user or in
the ``set_local_env.bat`` scripts.  When the ``.bat`` scripts
are executed, the ``set_local_env.bat`` script is executed if it exists.
Configure these files to customize the environment locally.

Debugging with the |BOARD_NAME|
-------------------------------

IAR Debugging 
~~~~~~~~~~~~~

A debug session can be started using IAR IDE. Once the IAR IDE is open with the workspace ``-bsp\projects\microej\iar\application.eww``,

- Right click on the project and select ``Options...``.
- Then go to the ``C/C++ Compiler`` tab and set the ``Optimizations > Level`` to ``None``.
- After that, rebuild the firmware and click on ``Project > Download and Debug``.
