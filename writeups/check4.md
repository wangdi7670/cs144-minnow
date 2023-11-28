Checkpoint 4 Writeup
====================

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the NetworkInterface:
[]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

Note:
1. 记录5内的 ARP_request

关于记录5s内的ARP_request，我在recv_frame()里也记录了其他主机发送的ARP_request, 但是把这行代码去掉，测试用例也能过；

我的思路是基于：一个局域网上，任意一个host给其他 host 发送 MAC_frame, 该局域网上的其他 host 也都能收到该 MAC_frame。

关于以上思路是根据 checkpoint4.pdf 对 NetworkInterface::recv frame()函数的解释猜测出来的，我自己目前不敢保证这种猜测一定是对的，但根据文档的意思是这样的