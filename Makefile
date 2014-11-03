THRIFT_VER =thrift-0.9.1
USR_DIR    =${HOME}/usr
THRIFT_DIR =${USR_DIR}/${THRIFT_VER}
INCS_DIRS  =-I${USR_DIR}/include -I${THRIFT_DIR}/include/thrift
LIBS_DIRS  =-L${USR_DIR}/lib -L${USR_DIR}/${THRIFT_VER}/lib
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   =-Wall -O2 --std=c++0x
LIBS       =-lthrift

GEN_SRC    = build/gen-cpp/ReplicatedStorage_types.cpp  \
						 build/gen-cpp/ReplicatedStorage_constants.cpp \
						 build/gen-cpp/Coordinator.cpp
GEN_INC    = -Ibuild/gen-cpp

default: server client

server: src/cpp/CppServer.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o build/bin/CppServer ${GEN_INC} ${INCS_DIRS} src/cpp/CppServer.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

client: src/cpp/CppClient.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o build/bin/CppClient ${GEN_INC} ${INCS_DIRS} src/cpp/CppClient.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

clean:
	$(RM) -r build/bin/CppClient build/bin/CppServer
