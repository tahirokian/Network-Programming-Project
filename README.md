# Network-Programming-Project
Internet Protocols Course Project - Dynamic Adaptive Video Streaming  (Over TCP)

This software project is a video server and client(s) system for Dynamic Adaptive Streaming over TCP. It is designed to improve the client’s viewing experience by avoiding interruptions in play due to fluctuating network conditions. It adapts to the network condition and manages the trade-off between the quality of the video and lag by changing the video quality. No client video rendering is performed in this project.

More specifically, this system server accepts video requests from the client(s) initially and then segments for individual video segments of the video and send only the requested parts of the video files. The client requests the videos in segments and adapt according to the network changes: switching to request for lower resolution files in case of low throughput values or vice versa. The software will facilitate communication and transfer of the files between the client(s) and server to the end of the video. The flow is depicted in the figure below:

![alt tag](/images/Architecture.png)

# Server use case: Receive video request

![alt tag](/images/serverUse.png)

1. Server receives request for a video
2. Sever parses the request and checks its video repository
3. If video is found, the server responds with the MPD of the requested file.
4. Sever receives requests for segments.
5. Server sends the corresponding segments. 

# Client use case: Receive video to be played

![alt tag](/images/clientUse.png)

1. User enters a video name
2. Client sends the name to the server.
3. Client receives MPD, and parses its
4. Client checks network conditions
5. Client sends request for a video segment based on network conditions.
6. Client plays video (simulates play).  
