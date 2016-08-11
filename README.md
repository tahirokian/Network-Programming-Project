# Network-Programming-Project
Internet Protocols Course Project - Dynamic Adaptive Video Streaming  (Over TCP)

This software project is a video server and client(s) system for Dynamic Adaptive Streaming over TCP. It is designed to improve the client’s viewing experience by avoiding interruptions in play due to fluctuating network conditions. It adapts to the network condition and manages the trade-off between the quality of the video and lag by changing the video quality.

More specifically, this system server accepts video requests from the client(s) initially and then segments for individual video segments of the video and send only the requested parts of the video files. The client requests the videos in segments and adapt according to the network changes: switching to request for lower resolution files in case of low throughput values or vice versa. The software will facilitate communication and transfer of the files between the client(s) and server to the end of the video. 

![alt tag](/images/Architecture.png)
