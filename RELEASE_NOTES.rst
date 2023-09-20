..
	Copyright 2021-2023 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.

.. |BOARD_NAME| replace:: AMAP4PEVB
.. |BOARD_REVISION| replace:: 1
.. |RCP| replace:: MICROEJ SDK
.. |VEE_PORT| replace:: MicroEJ VEE Port
.. |VEE_PORTS| replace:: MicroEJ VEE Ports
.. |VEE_PORT_VER| replace:: 1.0.2
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



MISRA Compliance
================

This VEE Port has a list of components that are MISRA-compliant (MISRA C:2012) with some noted exception.
Below is the list of ``folders that have been verified``:

- microej/core/
- microej/main/
- microej/ui/

Among the folders verified, below is the list of ``files that have not been verified``:

- core/src/LLBSP_generic.c
- core/src/microej_main.c
- core/src/microej_time_freertos.c
- ui/src/LLDW_PAINTER_impl.c
- ui/src/LLUI_INPUT_LOG_impl.c
- ui/src/microui_event_decoder.c
- ui/src/LLUI_DISPLAY_HEAP_impl.c
- ui/src/ui_drawing_nema.c

It has been verified with Cppcheck v2.10. Here is the list of deviations from MISRA standard:

+------------+-----------+----------------------------------------------------------------------+
| Deviation  | Category  | Justification                                                        |
+============+===========+======================================================================+
| Rule 2.3   | Advisory  | A type can be defined at API level and not used by the application.  |
|            |           |                                                                      |
| Rule 2.4   | Advisory  | A tag can be defined at API level and not used by the application.   |
|            |           |                                                                      |
| Rule 2.5   | Advisory  | A macro can be defined at API level and not used by the application. |
|            |           |                                                                      |
| Rule 5.5   | Required  | Intentional usage of macros to define native functions.              |
|            |           |                                                                      |
| Rule 8.2   | Required  | False positive error.                                                |
|            |           |                                                                      |
| Rule 8.4   | Required  | Cannot be seen during analysis.                                      |
|            |           |                                                                      |
| Rule 8.7   | Advisory  | External linkage, cannot be seen during analysis.                    |
|            |           |                                                                      |
| Rule 8.9   | Advisory  | Global buffer needed for the java world.                             |
|            |           |                                                                      |
| Rule 10.8  | Required  | Cast for include in nema matrix.                                     |
|            |           |                                                                      |
| Rule 11.3  | Required  | Used by many C framework to factorize code                           |
|            |           |                                                                      |
| Rule 11.4  | Advisory  | Used when coding BSP C source code (drivers, etc.)                   |
|            |           |                                                                      |
| Rule 11.8  | Required  | Need to cast for the function signature.                             |
|            |           |                                                                      |
| Rule 12.2  | Required  | Only sliding 15 bit on a 32 bit variable.                            |
|            |           |                                                                      |
| Rule 14.4  | Required  | False positive: The function return a bool.                          |
|            |           |                                                                      |
| Rule 17.8  | Required  | Can be useful when designing C library.                              |
|            |           |                                                                      |
| Rule 18.4  | Required  | Can be used in configurable C library.                               |
|            |           |                                                                      |
| Rule 20.9  | Required  | configUSE_TICKLESS_IDLE defined in the FreeRTOSConfig.h.             |
|            |           |                                                                      |
| Rule 20.10 | Advisory  | Used by MicroEJ architectures.                                       |
|            |           |                                                                      |
| Rule 20.14 | Required  | False positive, #if above.                                           |
|            |           |                                                                      |
| Rule 21.1  | Required  | Underscore prefix in library headers guard.                          |
|            |           |                                                                      |
| Rule 21.6  | Required  | Used for printf usage only.                                          |
+------------+-----------+----------------------------------------------------------------------+



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
