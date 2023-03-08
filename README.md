# SMRT-Fridge-TIVA-PROTO-

## This repo contains a project created for the course ENGR 478 (Design with Microprocessors) offered at San Francisco State University. 

The course required students to create a project utilizing the TIVA TM4C123GH6PM microcontroller.  
The KEIL IDE was used for programming the ARM cortex microprocessor on the TIVA board.

### Project Description from proposal:

  The goal of our project is to create a “smart refrigerator,” that will mitigate users' from cataloging their groceries. The smart fridge design will
consist of a 1-D barcode scanner, a current list of items in the fridge, and the ability to print a
grocery list. We will scan the item's barcode, which will be sent to the barcode.monster API and
return specific information about the item. 
  
  This information will then be associated with the
switch that has recently changed from the off state to the on state (which will be acknowledged
using reverse logic within the Tiva board). There will also be an asynchronous switch, which
when pressed will “dump” the stored information (barcode and item name) to a UART thermal
printer. Whose purpose will be to print out a grocery list of the items with switches that are not
active 

### Microcontroller Specifications:

32-bit Arm Cortex-M4F based MCU with 80 -MHz, 256 -KB Flash, 32 -KB RAM, 2 CAN, RTC, USB, 64-Pin
https://www.ti.com/product/TM4C123GH6PM#params

Project Contributors: Brandon Foley, Ervin Pendersen

Website: https://smartfridge478.carrd.co/
