VPP_ROOT=../vpp
VPP_TYPE=vpp_debug
VPP_INSTALL=install-${VPP_TYPE}-native
INST_ROOT=${VPP_ROOT}/build-root/${VPP_INSTALL}

CLS_INCLUDES = \
	-I ${INST_ROOT}/vlib/include \
	-I ${INST_ROOT}/vnet/include \
	-I ${INST_ROOT}/vppinfra/include

CLS_LIBS = \
	${INST_ROOT}/vppinfra/lib64/libvppinfra.a \
	${INST_ROOT}/vnet/lib64/libvnet.a \
	${INST_ROOT}/vlib/lib64/libvlib.a \
	${INST_ROOT}/vlib-api/lib64/libvlibmemory.a \
	${INST_ROOT}/vppinfra/lib64/libvppinfra.a \
	${INST_ROOT}/vlib-api/lib64/libvlibapi.a \
	${INST_ROOT}/svm/lib64/libsvm.a \
	${INST_ROOT}/dpdk/lib/librte_mempool.a \
	${INST_ROOT}/dpdk/build/lib/librte_eal/linuxapp/eal/librte_eal.a \
	-lpthread -lrt

all: pool classifier
pool: pool.c
	gcc -o pool pool.c -I ~/vpp/build-root/install-vpp_debug-native/vppinfra/include ~/vpp/build-root/install-vpp_debug-native/vppinfra/lib64/libvppinfra.a

classifier: classifier.c
	gcc -g -o classifier classifier.c ${CLS_INCLUDES} ${CLS_LIBS}

clean:
	rm -f pool
	rm -f classifier
