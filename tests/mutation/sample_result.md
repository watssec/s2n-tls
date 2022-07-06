# 0

An explanation for `llvm_verify` command

```
update_ov <- crucible_llvm_verify m "s2n_drbg_update" [bits_ov, encryptInit_ov, aes_128_ecb_ov, cipher_key_length_ov] false (update_spec seedsize) (w4_unint_yices ["block_encrypt"]);
```

m -> module name 

s2n_drbg_update -> function name

[bits_ov, encryptInit_ov ....] -> a list of already-verified specifications to use for compositional verification.

false -> Wether to do path satisfiability or not

update_spec seedsize -> specification of the function to be verified

block_encrypt- > from drbg.cry, the proof script to use for verification


# 1

## Report Content

```
{
    "0": "null",
    "684": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_update",
        "function_num": 480,
        "instruction_col": 5,
        "instruction_line": 95,
        "instruction_num": 12258,
        "mutation_type": "ConstantInt",
        "opcode": "i64",
        "operand_num": 2,
        "mutated_type": "2"
    }
},
```

[Source code location](https://github.com/aws/s2n-tls/blob/aa8f7fbb5e8afb649230f70917dd6bbe31702019/crypto/s2n_drbg.c#L95)


```
    RESULT_STACK_BLOB(temp_blob, s2n_drbg_seed_size(drgb), S2N_DRBG_MAX_SEED_SIZE);
```

`S2N_DRBG_MAX_SEED_SIZE` defined in `s2n_drbg.h`

`s2n_drbg_update` calls [s2n_drbg_seed_size, s2n_drbg_bits, s2n_drbg_key_size,]

[spec location](https://github.com/aws/s2n-tls/blob/247bf3151194c3326e1c9f63cb1b1c06ab30f1ea/tests/saw/spec/DRBG/DRBG.saw#L399)

```
update_ov <- crucible_llvm_verify m "s2n_drbg_update" [bits_ov, encryptInit_ov, aes_128_ecb_ov, cipher_key_length_ov] false (update_spec seedsize) (w4_unint_yices ["block_encrypt"]);
```

- specification to be verified 

bits_ov -> s2n_drbg_bits

- assumed specifications 

encryptInit_ov -> EVP_EncryptInit_ex

cipher_key_length_ov -> cipher_key_length_spec

- assumptions about external functions


aes_128_ecb_ov -> EVP_aes_128_ecb



## Manual tested?

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_drbg.c  -function_num 480  -instruction_num 12258 -operand_num 2 -target_type 2 -ConstantIn < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

**Step 0** check .ll file to see whether it has really been mutated

```
   %21 = getelementptr inbounds [48 x i8], [48 x i8]* %3, i64 0, i64 0, !dbg !59475 
```
```
   %21 = getelementptr inbounds [48 x i8], [48 x i8]* %3, i64 -1, i64 0, !dbg !59475  
```
**Step 1** check saw verification

pass

# 2

## Report Content

```
{
    "0": "null",
    "140": {
        "file_name": "s2n_handshake_type.c",
        "function_name": "s2n_handshake_type_set_tls12_flag",
        "function_num": 296,
        "instruction_col": 36,
        "instruction_line": 36,
        "instruction_num": 7387,
        "mutation_type": "Binop",
        "opcode": "or",
        "operand_num": 0,
        "mutated_type": "xor"
    }
},
```

[Source code](https://github.com/aws/s2n-tls/blob/abed2a324e59da13f73967ed7ea936b5dd2a1aae/tls/s2n_handshake_type.c#L62)

`s2n_handshake_type_set_tls12_flag` is called by `s2n_conn_set_handshake_type`

```
There is no spec for `s2n_handshake_type_set_tls12_flag`
```
[saw spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake.saw#L90)

```
let s2n_conn_set_handshake_type_ovs = [s2n_allowed_to_cache_connection, auth_type_proof, s2n_generate_new_client_session_id, s2n_decrypt_session_ticket];
print "Proving correctness of s2n_conn_set_handshake_type (NULL chosen_psk)";
s2n_conn_set_handshake_type_chosen_psk_null_proof     <- crucible_llvm_verify llvm "s2n_conn_set_handshake_type" s2n_conn_set_handshake_type_ovs false (s2n_conn_set_handshake_type_spec true)  (w4_unint_yices []);
print "Proving correctness of s2n_conn_set_handshake_type (non-NULL chosen_psk)";
s2n_conn_set_handshake_type_chosen_psk_non_null_proof <- crucible_llvm_verify llvm "s2n_conn_set_handshake_type" s2n_conn_set_handshake_type_ovs false (s2n_conn_set_handshake_type_spec false) (w4_unint_yices []);

```
# Reference

[Saw Manual](https://saw.galois.com/manual.html)