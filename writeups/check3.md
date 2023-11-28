Checkpoint 3 Writeup
====================

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the TCPSender:
[]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

Notes:

1. if receiver_window = 4, 且sender存了"abcde"5个字节，SYN还没有发送，如果要生成TCPSenderMessage, 那么 msg 应该是{SYN=true, buffer="abc"}, 就是说window=4, 其中 SYN 也要占一个坑位，同理 FIN 也一样