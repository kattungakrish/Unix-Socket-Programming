all: servermain.cpp serverA.cpp serverB.cpp client.cpp
	g++ -o servermain servermain.cpp
	g++ -o serverA serverA.cpp
	g++ -o serverB serverB.cpp
	g++ -o client client.cpp

servermain: servermain.cpp
	g++ -o servermain servermain.cpp

serverA: serverA.cpp
	g++ -o serverA serverA.cpp

serverB: serverB.cpp
	g++ -o serverB serverB.cpp

client: client.cpp
	g++ -o client client.cpp

clean:
	$(RM) servermain serverA serverB client
