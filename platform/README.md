5.2.1 PIO read commands
The PIO read command group includes the Identify Drive, Read Buffer, Read Long, Read Multiple and Read Sectors commands. The Identify Drive and Read Buffer commands each transfer a single block of 512 bytes. The Read Long command transfers a single block of 512 bytes plus 4 or more ECC bytes. The Read Multiple command transfers one or more blocks of data where the size of each block is a multiple of 512 bytes. The Read Sectors command transfers one or more blocks of 512 bytes each.
To execute a PIO read command, the host and drive follow these steps:
1. The host writes any required parameters to the Features, Sector Count, Sector Number, Cylinder and Drive/Head registers.
2. The host writes the command code to the Command register.
3. The drive sets BSY and prepares for data transfer.
4. When a block of data is available, the drive sets DRQ and clears BSY prior to asserting INTRQ.
5. After detecting INTRQ, the host reads the Status register. In response to the Status register being read, the drive negates INTRQ.
6. The host reads one block of data from the Data register.
7. The drive clears DRQ. If transfer of another block is required, the drive
also sets BSY and the above sequence is repeated from step 4).
The following table shows a PIO read command that transfers two blocks without an error (in this and subsequent tables, read downward to follow the sequence of steps executed).
