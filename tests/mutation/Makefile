#####################
# Copyright 2016 Galois, Inc. All Rights Reserved
#
#  Authors:
#	Joey Dodds : jdodds@galois.com
#       Nathan Collins : conathan@galois.com
#	
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
#  http://aws.amazon.com/apache2.0
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
########################

#The scripts are all of the saw files in this directory
SCRIPTS = $(wildcard *.saw)
#A log file will be created for each test in the temp dir
LOGS=$(patsubst %.saw,tmp/%.log,$(SCRIPTS))
SHELL:=/bin/bash
YICES_VERSION=$(shell yices-smt2 --version)
export LIBCRYPTO_ROOT:=$(shell pwd)/../../libcrypto-root

.PHONY : all
all:
	@echo "Running formal verification with ${YICES_VERSION}"
	@${MAKE} clean-logs
	@${MAKE} clean_the_dir
	@${MAKE} $(LOGS)
	


############################################
## Clean targets
############################################
.PHONY : clean-logs
clean-logs : 
	$(RM) -- $(wildcard tmp/*.log)



.PHONY : clean-failure-logs
clean-failure-logs :
	$(RM) -- $(wildcard failure_tests/*.log)

.PHONY: clean
clean: decruft

.PHONY: decruft
decruft : clean-logs
	${MAKE} -C bitcode decruft
	${RM} -r s2n
	${RM} -r tmp

clean_the_dir :
	${RM} -r s2n
	${RM} -r tmp
	${RM} -r failure_tests/*.log
	${RM} -r sike_r1/*.log
	${RM} -r bike_r1/*.log
	${RM} -r bike_r2/*.log
	${RM} -r bitcode/*.bc




#The bitcode files don't get deleted, in case we want to do tests on them
.SECONDARY : $(wildcard bitcode/*.bc)



#if we ask this makefile to create bitcode it will always completely rebuild it
.PHONY : bitcode
bitcode :
	${MAKE} clean_the_dir


.PHONY : sike_r1_patched_bitcode
sike_r1_patched_bitcode :
	${MAKE} clean_the_dir


.PHONY : bike_r1_patched_bitcode
bike_r1_patched_bitcode :
	${MAKE} clean_the_dir
	

.PHONY : bike_r2_patched_bitcode
bike_r2_patched_bitcode :
	${MAKE} clean_the_dir


########################################################
## Rules to copy the s2n directory for patching and bulding
########################################################

CRYPTO_C = $(wildcard ../../crypto/*.c) $(wildcard ../../crypto/*.h) ../../crypto/Makefile
CRYPTO_COPY = $(addprefix s2n/crypto/, $(notdir $(CRYPTO_C)))

PQ_CRYPTO_C = $(wildcard ../../pq-crypto/*.c) $(wildcard ../../pq-crypto/*.h) ../../pq-crypto/s2n_pq_asm.mk ../../pq-crypto/Makefile
PQ_CRYPTO_COPY = $(addprefix s2n/pq-crypto/, $(notdir $(PQ_CRYPTO_C)))

SIKE_R1_C = $(wildcard ../../pq-crypto/sike_r1/*.c) $(wildcard ../../pq-crypto/sike_r1/*.h) ../../pq-crypto/sike_r1/Makefile
SIKE_R1_COPY = $(addprefix s2n/pq-crypto/sike_r1/, $(notdir $(SIKE_R1_C)))

BIKE_R1_C = $(wildcard ../../pq-crypto/bike_r1/*.c) $(wildcard ../../pq-crypto/bike_r1/*.h) ../../pq-crypto/bike_r1/Makefile
BIKE_R1_COPY = $(addprefix s2n/pq-crypto/bike_r1/, $(notdir $(BIKE_R1_C)))

BIKE_R2_C = $(wildcard ../../pq-crypto/bike_r2/*.c) $(wildcard ../../pq-crypto/bike_r2/*.h) ../../pq-crypto/bike_r2/Makefile
BIKE_R2_COPY = $(addprefix s2n/pq-crypto/bike_r2/, $(notdir $(BIKE_R2_C)))

# PQ SAW proofs only exist for SIKE (round 1) and BIKE round 1 and round 2. But the SAW Makefile still requires
# that we copy and build all PQ code (e.g. for kyber, round 3 SIKE/BIKE)

BIKE_R3_C = $(wildcard ../../pq-crypto/bike_r3/*.c) $(wildcard ../../pq-crypto/bike_r3/*.h) ../../pq-crypto/bike_r3/Makefile
BIKE_R3_COPY = $(addprefix s2n/pq-crypto/bike_r3/, $(notdir $(BIKE_R3_C)))

KYBER_R2_C = $(wildcard ../../pq-crypto/kyber_r2/*.c) $(wildcard ../../pq-crypto/kyber_r2/*.h) ../../pq-crypto/kyber_r2/Makefile
KYBER_R2_COPY = $(addprefix s2n/pq-crypto/kyber_r2/, $(notdir $(KYBER_R2_C)))

KYBER_90S_R2_C = $(wildcard ../../pq-crypto/kyber_90s_r2/*.c) $(wildcard ../../pq-crypto/kyber_90s_r2/*.h) ../../pq-crypto/kyber_90s_r2/Makefile
KYBER_90S_R2_COPY = $(addprefix s2n/pq-crypto/kyber_90s_r2/, $(notdir $(KYBER_90S_R2_C)))

KYBER_R3_C = $(wildcard ../../pq-crypto/kyber_r3/*.c) $(wildcard ../../pq-crypto/kyber_r3/*.h) ../../pq-crypto/kyber_r3/Makefile
KYBER_R3_COPY = $(addprefix s2n/pq-crypto/kyber_r3/, $(notdir $(KYBER_R3_C)))

SIKE_R3_C = $(wildcard ../../pq-crypto/sike_r3/*.c) $(wildcard ../../pq-crypto/sike_r3/*.h) ../../pq-crypto/sike_r3/Makefile
SIKE_R3_COPY = $(addprefix s2n/pq-crypto/sike_r3/, $(notdir $(SIKE_R3_C)))

UTILS_C = $(wildcard ../../utils/*.c) $(wildcard ../../utils/*.h) ../../utils/Makefile
UTILS_COPY =$(addprefix s2n/utils/, $(notdir $(UTILS_C)))

TLS_C = $(wildcard ../../tls/*.c ../../tls/*/*.c ../../tls/*.h ../../tls/*/*.h ../../tls/*/Makefile) ../../tls/Makefile
TLS_COPY = $(subst ../../tls/, s2n/tls/, $(TLS_C))
TLS_DIRS = $(sort $(dir $(TLS_COPY)))

STUFFER_C = $(wildcard ../../stuffer/*.c) $(wildcard ../../stuffer/*.h) ../../stuffer/Makefile
STUFFER_COPY =$(addprefix s2n/stuffer/, $(notdir $(STUFFER_C)))

API_COPY =$(addprefix s2n/api/, $(notdir $(wildcard ../../api/*.h)))

ERROR_C = $(wildcard ../../error/*.c) $(wildcard ../../error/*.h) ../../error/Makefile
ERROR_COPY = $(addprefix s2n/error/, $(notdir $(ERROR_C)))

LIB_COPY = s2n/lib/Makefile

s2n/error :
	mkdir -p $@

s2n/api :
	mkdir -p $@

s2n/crypto :
	mkdir -p $@

s2n/pq-crypto :
	mkdir -p $@

s2n/pq-crypto/sike_r1 :
	mkdir -p $@

s2n/pq-crypto/sike_r3 :
	mkdir -p $@

s2n/pq-crypto/bike_r1 :
	mkdir -p $@

s2n/pq-crypto/bike_r2 :
	mkdir -p $@

s2n/pq-crypto/bike_r3 :
	mkdir -p $@

s2n/pq-crypto/kyber_r2 :
	mkdir -p $@

s2n/pq-crypto/kyber_90s_r2 :
	mkdir -p $@

s2n/pq-crypto/kyber_r3 :
	mkdir -p $@

s2n/utils :
	mkdir -p $@

s2n/tls :
	mkdir -p $(TLS_DIRS)
 
s2n/stuffer :
	mkdir -p $@

s2n/lib :
	mkdir -p $@

export BITCODE_DIR := $(CURDIR)/bitcode/

tmp:
	mkdir -p tmp


s2n/lib/libs2n.so : s2n/crypto s2n/pq-crypto s2n/pq-crypto/sike_r1 s2n/pq-crypto/sike_r3 s2n/pq-crypto/bike_r1 s2n/pq-crypto/bike_r2 s2n/pq-crypto/bike_r3 s2n/pq-crypto/kyber_r2 s2n/pq-crypto/kyber_90s_r2 s2n/pq-crypto/kyber_r3 s2n/utils s2n/tls s2n/api s2n/error s2n/stuffer s2n/lib s2n/Makefile s2n/s2n.mk $(CRYPTO_COPY) $(PQ_CRYPTO_COPY) $(SIKE_R1_COPY) $(SIKE_R3_COPY) $(BIKE_R1_COPY) $(BIKE_R2_COPY) $(BIKE_R3_COPY) $(KYBER_R2_COPY) $(KYBER_90S_R2_COPY) $(KYBER_R3_COPY) $(UTILS_COPY) $(TLS_COPY) $(API_COPY) $(ERROR_COPY) $(STUFFER_COPY) $(LIB_COPY)
	${MAKE} -C s2n libs NO_STACK_PROTECTOR=1 NO_INLINE=1

s2n/%.h : ../../%.h
	cp $< $@

s2n/%.c : ../../%.c
	cp $< $@

s2n/%.S : ../../%.S
	cp $< $@

s2n/%Makefile : ../../%Makefile
	cp $< $@

s2n/Makefile : ../../Makefile
	cp $< $@

s2n/s2n.mk : ../../s2n.mk
	cp $< $@

s2n/pq-crypto/s2n_pq_asm.mk : ../../pq-crypto/s2n_pq_asm.mk
	cp $< $@
