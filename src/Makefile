.PHONY: server client

all:
	gcc -Wall client/client.c -pthread -L/usr/local/lib -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lva-x11 -lva -lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lX11 -lasound -lSDL -lx264 -lpthread -lvpx -lvorbisenc -lvorbis -ltheoraenc -ltheoradec -logg -lopus -lmp3lame -lfreetype -lass -lz -lswresample -lswscale -lavutil -lrt -lm -o client/client
	gcc -Wall server/server.c -o server/server

server:
	gcc -Wall server/server.c -o server/server

client:
	gcc -Wall client/client.c -pthread -L/usr/local/lib -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -lva-x11 -lva -lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lX11 -lasound -lSDL -lx264 -lpthread -lvpx -lvorbisenc -lvorbis -ltheoraenc -ltheoradec -logg -lopus -lmp3lame -lfreetype -lass -lz -lswresample -lswscale -lavutil -lrt -lm -o client/client

clean:
	rm -f client/client
	rm -f server/server

cleanServ:
	rm -f server/server

cleanClient:
	rm -f client/client