
# 0


## Test Range

All the functions that are specified by `llvm_verify` and the functions that are directly or indirectly called by these functions are within the dataset to be tested here.

## llvm_verify

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
    "190": {
        "file_name": "s2n_handshake_io.c",
        "function_name": "s2n_conn_set_handshake_type",
        "function_num": 433,
        "instruction_col": 36,
        "instruction_line": 861,
        "instruction_num": 10332,
        "mutation_type": "ConstantInt",
        "opcode": "i32",
        "operand_num": 2,
        "mutated_type": "1"
    }
},



```

[Source code location](https://github.com/aws/s2n-tls/blob/282c0dd405bd6fd6127551b8a480335e41733c1d/tls/s2n_handshake_io.c#L861)



 
## Manual tested?

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_handshake_io.c -function_num 433 -instruction_num 10332 -operand_num 2 -target_type 1 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc

```


**Step 0** 

```
; <label>:78:                                     ; preds = %75
  %79 = icmp slt i32 %76, 0, !dbg !43840
  br i1 %79, label %80, label %84, !dbg !43842
```


```
; <label>:78:                                     ; preds = %75
  %79 = icmp slt i32 %76, 1, !dbg !43840
  br i1 %79, label %80, label %84, !dbg !43842
```

## Source Code Analysis

```
        if (r == S2N_SUCCESS || (r < S2N_SUCCESS && S2N_ERROR_IS_BLOCKING(s2n_errno))) {
```

mutate the constant value [S2N_SUCCESS](https://github.com/aws/s2n-tls/blob/282c0dd405bd6fd6127551b8a480335e41733c1d/api/s2n.h#L50) from 0 ->1

Although the first condition  ` r == S2N_SUCCESS ` does not hold anymore, the second condition `r < S2N_SUCCESS && S2N_ERROR_IS_BLOCKING(s2n_errno)` still holds.

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

`s2n_conn_set_handshake_type` should be part of the compositional proof, but it's not and 
there is no spec for `s2n_handshake_type_set_tls12_flag`

```
let s2n_conn_set_handshake_type_ovs = [s2n_allowed_to_cache_connection, auth_type_proof, s2n_generate_new_client_session_id, s2n_decrypt_session_ticket];
```
[saw spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake.saw#L90)

```
let s2n_conn_set_handshake_type_ovs = [s2n_allowed_to_cache_connection, auth_type_proof, s2n_generate_new_client_session_id, s2n_decrypt_session_ticket];
print "Proving correctness of s2n_conn_set_handshake_type (NULL chosen_psk)";
s2n_conn_set_handshake_type_chosen_psk_null_proof     <- crucible_llvm_verify llvm "s2n_conn_set_handshake_type" s2n_conn_set_handshake_type_ovs false (s2n_conn_set_handshake_type_spec true)  (w4_unint_yices []);
print "Proving correctness of s2n_conn_set_handshake_type (non-NULL chosen_psk)";
s2n_conn_set_handshake_type_chosen_psk_non_null_proof <- crucible_llvm_verify llvm "s2n_conn_set_handshake_type" s2n_conn_set_handshake_type_ovs false (s2n_conn_set_handshake_type_spec false) (w4_unint_yices []);

```

## Manual test
```
opt -load ../../mutation/passes/Binop/build/Mutation/libMutation.so -file_name s2n_handshake_type.c -function_num 296 -instruction_num 7387 -target_type xor -Binop < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

**Step 0**

original
```
; <label>:11:                                     ; preds = %6
  %12 = getelementptr inbounds %struct.s2n_connection, %struct.s2n_connection* %0, i64 0, i32 47, i32 5, !dbg !53485
  %13 = load i32, i32* %12, align 8, !dbg !53486, !tbaa !53435
  %14 = or i32 %13, %1, !dbg !53486
  store i32 %14, i32* %12, align 8, !dbg !53486, !tbaa !53435
  br label %15, !dbg !53487

; <label>:15:                                     ; preds = %11, %9, %4
  %16 = phi i32 [ 0, %11 ], [ -1, %9 ], [ -1, %4 ]
  ret i32 %16, !dbg !53488
}
```

mutated

```
; <label>:11:                                     ; preds = %6
  %12 = getelementptr inbounds %struct.s2n_connection, %struct.s2n_connection* %0, i64 0, i32 47, i32 5, !dbg !53485
  %13 = load i32, i32* %12, align 8, !dbg !53486, !tbaa !53435
  %14 = mul i32 %13, %1, !dbg !53486
  %15 = xor i32 %13, %1, !dbg !53486
  %16 = or i32 %13, %1, !dbg !53486
  store i32 %15, i32* %12, align 8, !dbg !53486, !tbaa !53435
  br label %17, !dbg !53487


```

**Step 1**

pass


# 4

## Report Content
```
    {
        "0": "null",
        "412": {
            "file_name": "s2n_handshake_io.c",
            "function_name": "s2n_validate_ems_status",
            "function_num": 436,
            "instruction_col": 26,
            "instruction_line": 807,
            "instruction_num": 10525,
            "mutation_type": "ConstantInt",
            "opcode": "i32",
            "operand_num": 2,
            "mutated_type": "1"
        }
    },

```
[Source code](https://github.com/watssec/s2n-tls/blob/caa01e02da72abc24ca64d814835582199c8955c/tls/s2n_handshake_io.c#L807)

[saw spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake.saw#L90)


## Manual Test

**Step 0**

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_drbg.c -function_num 436 -instruction_num 10525 -operand_num 2 -target_type 1 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```
Original

```
; <label>:28:                                     ; preds = %9
  %29 = zext i1 %20 to i32, !dbg !43729
  %30 = shl nuw nsw i32 %29, 16, !dbg !43730
  %31 = and i32 %22, -65537, !dbg !43730
  %32 = or i32 %30, %31, !dbg !43730
  store i32 %32, i32* %21, align 8, !dbg !43730
  br label %33, !dbg !43731

```
Mutated

```
; <label>:28:                                     ; preds = %9
  %29 = zext i1 %20 to i32, !dbg !43729
  %30 = shl nuw nsw i32 %29, 16, !dbg !43730
  %31 = and i32 %22, 1, !dbg !43730
  %32 = or i32 %30, %31, !dbg !43730
  store i32 %32, i32* %21, align 8, !dbg !43730
  br label %33, !dbg !43731

```

## Source Code Analysis

The value of `conn->ems_negotiated` is set to ems_extension_recv at the end of function `s2n_validate_ems_status`.

```
    conn->ems_negotiated = ems_extension_recv;
```

However, this behavior is not monitored in the spec function. 


(function `s2n_validate_ems_status` is not a component of the spec linkage of `s2n_conn_set_handshake_type`)
**Step 1**
## Report Analysis

`s2n_validate_status` is called by `s2n_conn_set_handshake_type`, spec for `s2n_validate_status` is missing

**Step 1**

Pass

# 5
## Report Content

```
{
    "0": "null",
    "227": {
        "file_name": "s2n_handshake_io.c",
        "function_name": "s2n_conn_set_handshake_type",
        "function_num": 433,
        "instruction_col": 9,
        "instruction_line": 884,
        "instruction_num": 10386,
        "mutation_type": "Branch",
        "opcode": "br",
        "operand_num": 0,
        "mutated_type": "no target"
    }
},
```

[Source code](https://github.com/aws/s2n-tls/blob/034aa5ff145eaee63288f7daf7b6649eb7bcece6/tls/s2n_handshake_io.c#L884)

[Saw spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake_io_lowlevel.saw#L305)



## Manual Test

**Step 0**

```
opt -load ../../mutation/passes/Branch/build/Mutation/libMutation.so -file_name s2n_handhsake_io.c -function_num 433 -instruction_num 10386 -Branch < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original complete block
```

; <label>:123:                                    ; preds = %119
  %124 = getelementptr inbounds %struct.s2n_connection, %struct.s2n_connection* %0, i64 0, i32 30, i32 8, !dbg !57608
  %125 = load %struct.s2n_cert_chain_and_key*, %struct.s2n_cert_chain_and_key** %124, align 8, !dbg !57608, !tbaa !56407
  %126 = icmp eq %struct.s2n_cert_chain_and_key* %125, null, !dbg !57608
  br i1 %126, label %131, label %127, !dbg !57608

; <label>:127:                                    ; preds = %123
  %128 = getelementptr inbounds %struct.s2n_cert_chain_and_key, %struct.s2n_cert_chain_and_key* %125, i64 0, i32 2, i32 1, !dbg !57610
  %129 = load i32, i32* %128, align 8, !dbg !57610, !tbaa !57612
  %130 = icmp eq i32 %129, 0, !dbg !57610
  br i1 %130, label %131, label %140, !dbg !57613

; <label>:131:                                    ; preds = %119, %123, %127
  %132 = load i32, i32* %23, align 8, !dbg !57614, !tbaa !49204
  br label %133, !dbg !57614

; <label>:133:                                    ; preds = %131, %116
  %134 = phi i32 [ %132, %131 ], [ %117, %116 ], !dbg !57614
  %135 = icmp eq i32 %134, 1, !dbg !57614
  br i1 %135, label %136, label %143, !dbg !57614

```
Original

```
old op  br i1 %126, label %131, label %127, !dbg !43899
```

Mutated

```
new op  br i1 %126, label %127, label %131, !dbg !43899
```

## Source code Analysis

```
if (s2n_server_can_send_ocsp(conn) || s2n_server_sent_ocsp(conn)) {
    POSIX_GUARD_RESULT(s2n_handshake_type_set_tls12_flag(conn, OCSP_STATUS));
}
```


`s2n_server_can_send_ocsp(conn)` is defined [here](https://github.com/aws/s2n-tls/blob/bf2eebc1b671ad04d95e690b44d71e3abd38c9d4/tls/s2n_tls.h#L92).

`<label> 123` checks `(conn)->handshake_params.our_chain_and_key` is null or not. This mutation show that if we change the condition ot `!(conn)->handhsake_params.our_chain_and_key`, the prover still passes. Because this is not specified in the spec function.


**Step 1**

Pass

# 6 

## Report Content
```
{
    "0": "null",
    "79": {
        "file_name": "s2n_socket.c",
        "function_name": "s2n_socket_write_uncork",
        "function_num": 25,
        "instruction_col": 5,
        "instruction_line": 163,
        "instruction_num": 494,
        "mutation_type": "Branch",
        "opcode": "br",
        "operand_num": 0,
        "mutated_type": "no target"
    }
},
```

[Source code](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/utils/s2n_socket.c#L163)
[saw spec]()
## Report Analysis

## Manual Test

```
opt -load ../../s2n-tls/mutation/passes/Inspec/build/Mutation/libMutation.so -function_num 25 -instruction_num 494 -Branch ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc

```
**Step 0**
Original

```
%2 = alloca i32, align 4
tail call void @llvm.dbg.value(metadata %struct.s2n_connection* %0, i64 0, metadata !43683, metadata !43686), !dbg !43687
%3 = icmp eq %struct.s2n_connection* %0, null, !dbg !43688
br i1 %3, label %4, label %6, !dbg !43688

```
Mutated

```
%2 = alloca i32, align 4
tail call void @llvm.dbg.value(metadata %struct.s2n_connection* %0, i64 0, metadata !43683, metadata !43686), !dbg !43687
%3 = icmp eq %struct.s2n_connection* %0, null, !dbg !43688
br i1 %3, label %6, label %4, !dbg !43688
```

**Step 1**



[Saw Manual](https://saw.galois.com/manual.html)