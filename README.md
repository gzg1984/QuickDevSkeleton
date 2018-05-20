# QuickDevSkeleton

## Platform And PATA device
    在platform下， 我尝试了创建一个名为fake_pata_platform的platform设备，并创造了一个fake_pata_platform的platform驱动。
    
    在这个目录下的代码，展示了如何创建和使用platform体系。其流程和结构体使用，是非常简单易读的。并且也确实可以当作骨架代码来进行扩展。
    
    此外，platform体系和其他软件层的结合关系，也通过platform_dev -> platform_drv -> ata_dev 这样的流程，展示了出来。
    
    更近一步地说，这个目录下的代码，可以用于模拟出一个虚拟的PATA硬盘设备……当然，现在对PATA的协议并没有处理，
    
    但是操作系统是不知道的。
    
    当这个platform设备被作为ATA设备注册到系统中之后，操作系统的libata子系统，将会按真实设备一样去操作这个设备，
    
    然后，platform dev关联的内核线程就会监控到如下的，libata向这个虚假ata设备的操作动作：
```
    IO [2] [0x31] ==> [0x55]
    IO [3] [0x32] ==> [0xAA]
    IO [6] [0x30] ==> [0xA0]
    IO [6] [0xA0] ==> [0xB0]
    Controller Reg [ATA_REG_DATA] [0xA] ==> [0x8]
```
