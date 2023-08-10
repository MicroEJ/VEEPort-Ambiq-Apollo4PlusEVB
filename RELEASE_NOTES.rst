..
	Copyright 2021-2023 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.

.. |BOARD_NAME| replace:: AMAP4PEVB
.. |BOARD_REVISION| replace:: 1
.. |RCP| replace:: MICROEJ SDK
.. |VEE_PORT| replace:: MicroEJ VEE Port
.. |VEE_PORTS| replace:: MicroEJ VEE Ports
.. |VEE_PORT_VER| replace:: 1.0.1
.. |SIM| replace:: MicroEJ Simulator
.. |ARCH| replace:: MicroEJ Architecture
.. |CIDE| replace:: MICROEJ SDK
.. |RTOS| replace:: FreeRTOS RTOS
.. |MANUFACTURER| replace:: Ambiq

.. _README MicroEJ BSP: ./AMBIQ-AMAP4PEVB-FreeRTOS-bsp/README.rst
.. _RELEASE NOTES: ./RELEASE_NOTES.rst
.. _CHANGELOG: ./CHANGELOG.rst

.. _release-notes:

========================================================
|VEE_PORT| Release Notes for |MANUFACTURER| |BOARD_NAME|
========================================================

Description
===========

This is the release notes of the |VEE_PORT| for |BOARD_NAME|.

Versions
========

VEE Port
--------

|VEE_PORT_VER|

Dependencies
------------

This |VEE_PORT| contains the following dependencies:

.. list-table::
   :header-rows: 1

   * - Dependency Name
     - Version
   * - Architecture (flopi4I35)
     - 7.18.0
   * - Pack-UI (flopi4I35-ui-pack)
     - 13.4.1

Please refer to the |VEE_PORT| `module description file <./AMBIQ-AMAP4PEVB-FreeRTOS-configuration/module.ivy>`_
for more details.

Board Support Package
---------------------

- BSP provider: |MANUFACTURER| (version: ``release_sdk_4_3_0-0ca7d78a2b``)

Third Party Software
--------------------

All third party sources can be found in the `third_party folder <./AMBIQ-AMAP4PEVB-FreeRTOS-bsp/sdk/third_party>`__ in the |MANUFACTURER| SDK.
Here, bellow, is a non-exhaustive list:

.. list-table::
   :widths: 3 3 3

   * - RTOS
     - FreeRTOS
     - 10.1.1
   * - GPU lib
     - NEMA GFX
     - 1.4.1



Features
========

Display
-------

The |BOARD_NAME| board can use the Apollo4 Plus EVB Display Kit. 

The Apollo4 Plus EVB Display Kit features a 545 * 545 AMOLED display.  The pixel format
is 8 bits-per-pixel. The display device is connected to the MCU via a I2C link.
It uses a NEMA GPU of Think Silicon.

For more information, please refer to the Quick Start Guide of the display shield available `here <https://ambiq.com/wp-content/uploads/2022/09/Apollo4-Plus-Display-Kit-Quick-Start-Guide.pdf>`__.

MicroUI requires a RAM buffer to store the dynamic images data.  A
dynamic image is an image decoded at runtime (PNG image) or an image created
by the Application using the ``Image.create(width, height)`` API.
This buffer is located in external RAM.


Known issues/limitations
========================

GPU freeze
----------

The GPU can have some issues with resources located in the MRAM (ERR105) or the SRAM (ERR049).
Even though the MRAM issue is not mentioned in the errata list of the Apollo4 Plus board, we did encounter the problem ERR049
mentioned in the errata list of the Apollo4.
For more information, pleaser refer to the errata list of the
`Apollo4 Plus <https://ambiq.com/wp-content/uploads/2022/04/Apollo4-Plus-Silicon-Errata-List.pdf>`__
or `Apollo4 <https://ambiq.com/wp-content/uploads/2022/04/Apollo4-Silicon-Errata-List.pdf>`__ board.


VEE Port Memory Layout
======================

Memory Sections
---------------

Each memory section is described in the IAR linker file available
`here <./AMBIQ-AMAP4PEVB-FreeRTOS-bsp/projects/microej/iar/linker_script.icf>`__

Memory Layout
-------------

.. list-table::
   :header-rows: 1

   * - Section Content
     - Section Source
     - Section Destination
     - Memory Type
   * - MicroEJ Application static
     - ``.bss.soar``
     - ``MCU_TCM``
     - TCM
   * - MicroEJ Application threads stack blocks
     - ``.bss.vm.stacks.java``
     - ``MCU_TCM``
     - TCM
   * - MicroEJ Core Engine internal heap
     - ``ICETEA_HEAP``
     - ``MCU_TCM``
     - TCM
   * - MicroEJ Application heap
     - ``_java_heap``
     - ``MCU_TCM``
     - TCM
   * - MicroEJ Application Immortal Heap
     - ``_java_immortals``
     - ``MCU_TCM``
     - TCM
   * - MicroEJ Application and Library code
     - ``.text.soar``
     - ``MCU_MRAM``
     - Non-volatile memory

Please also refer to the MicroEJ docs website page available `here
<https://docs.microej.com/en/latest/PlatformDeveloperGuide/coreEngine.html#link>`__
for more details.
