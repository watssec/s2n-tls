
#Clean targets

clean_the_dir :
	${RM} -r s2n
	${RM} -r tmp
	$(RM) -r sike_r1/*.log
	$(RM) -r bike_r1/*.log
	$(RM) -r bike_r2/*.log
	${RM} -r bitcode/*.bc

#lead the files to be mutated into 


.PHONY : all
all:

# Rules to copy the s2n directory for patching and building

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

bitcode/all_llvm.bc : s2n/crypto s2n/pq-crypto s2n/pq-crypto/sike_r1 s2n/utils s2n/tls s2n/api s2n/error s2n/stuffer s2n/Makefile s2n/s2n.mk $(CRYPTO_COPY) $(PQ_CRYPTO_COPY) $(SIKE_R1_COPY) $(UTILS_COPY) $(TLS_COPY) $(API_COPY) $(ERROR_COPY) $(STUFFER_COPY)
	${MAKE} -C s2n bc
	${MAKE} -C bitcode all_llvm.bc

.PHONY : bitcode/sike_r1_llvm.bc
bitcode/sike_r1_llvm.bc : s2n/crypto s2n/pq-crypto s2n/pq-crypto/sike_r1 s2n/utils s2n/tls s2n/api s2n/error s2n/stuffer s2n/Makefile s2n/s2n.mk $(CRYPTO_COPY) $(PQ_CRYPTO_COPY) $(SIKE_R1_COPY) $(UTILS_COPY) $(TLS_COPY) $(API_COPY) $(ERROR_COPY) $(STUFFER_COPY)
	patch s2n/pq-crypto/sike_r1/ec_isogeny_r1.c sike_r1/patch/ec_isogeny_r1.c.patch
	${MAKE} -C s2n sike_r1_bc
	${MAKE} -C bitcode all_llvm.bc

.PHONY : bitcode/bike_r1_llvm.bc
bitcode/bike_r1_llvm.bc : s2n/crypto s2n/pq-crypto s2n/pq-crypto/bike_r1 s2n/utils s2n/tls s2n/api s2n/error s2n/stuffer s2n/Makefile s2n/s2n.mk $(CRYPTO_COPY) $(PQ_CRYPTO_COPY) $(BIKE_R1_COPY) $(UTILS_COPY) $(TLS_COPY) $(API_COPY) $(ERROR_COPY) $(STUFFER_COPY)
	patch s2n/pq-crypto/bike_r1/cleanup.h bike_r1/patch/cleanup.h.patch
	patch s2n/pq-crypto/bike_r1/sampling_portable.c bike_r1/patch/sampling_portable.c.patch
	patch s2n/pq-crypto/bike_r1/sampling.c bike_r1/patch/sampling.c.patch
	patch s2n/pq-crypto/bike_r1/bike_r1_kem.c bike_r1/patch/bike_r1_kem.c.patch
	${MAKE} -C s2n bike_r1_bc
	${MAKE} -C bitcode bike_r1_llvm.bc
	${RM} -r s2n

.PHONY : bitcode/bike_r2_llvm.bc
bitcode/bike_r2_llvm.bc : s2n/crypto s2n/pq-crypto s2n/pq-crypto/bike_r2 s2n/utils s2n/tls s2n/api s2n/error s2n/stuffer s2n/Makefile s2n/s2n.mk $(CRYPTO_COPY) $(PQ_CRYPTO_COPY) $(BIKE_R2_COPY) $(UTILS_COPY) $(TLS_COPY) $(API_COPY) $(ERROR_COPY) $(STUFFER_COPY)
	patch s2n/pq-crypto/bike_r2/cleanup.h bike_r2/patch/cleanup.h.patch
	patch s2n/pq-crypto/bike_r2/aes_ctr_prf.h bike_r2/patch/aes_ctr_prf.h.patch
	patch s2n/pq-crypto/bike_r2/aes_ctr_prf.c bike_r2/patch/aes_ctr_prf.c.patch
	patch s2n/pq-crypto/bike_r2/sampling_portable.c bike_r2/patch/sampling_portable.c.patch
	patch s2n/pq-crypto/bike_r2/sampling.c bike_r2/patch/sampling.c.patch
	patch s2n/pq-crypto/bike_r2/bike_r2_kem.c bike_r2/patch/bike_r2_kem.c.patch
	${MAKE} -C s2n bike_r2_bc
	${MAKE} -C bitcode bike_r2_llvm.bc
	${RM} -r s2n

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


