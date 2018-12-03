需要测试tbus配置在运行时添加配置，刷新共享内存，系统不受影响

注意新加配置后，需要用tbusmgr重写一次GCIM共享内存，程序需要调用CommBusLayer::RefressHandle(), 否则新通道两端进程无法通信


