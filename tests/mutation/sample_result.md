
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

# 3

## Report Content 

```
{
    "0": "null",
    "496": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_bits",
        "function_num": 481,
        "instruction_col": 5,
        "instruction_line": 84,
        "instruction_num": 12420,
        "mutation_type": "Binop",
        "opcode": "sub",
        "operand_num": 0,
        "mutated_type": "add"
    }
},
```

[Source Code](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/crypto/s2n_drbg.c#L84)
[Saw spec](https://github.com/aws/s2n-tls/blob/247bf3151194c3326e1c9f63cb1b1c06ab30f1ea/tests/saw/spec/DRBG/DRBG.saw#L397)

## Manual Test
**Step 0**

```
opt -load ../../mutation/passes/Binop/build/Mutation/libMutation.so -function_num 481 -instruction_num 12420 -target_type add -Binop < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:58:                                     ; preds = %54
  %59 = getelementptr inbounds %struct.s2n_drbg, %struct.s2n_drbg* %0, i64 0, i32 0, !dbg !43796
  %60 = load i64, i64* %59, align 8, !dbg !43797, !tbaa !43775
  %61 = add i64 %60, 16, !dbg !43797
  store i64 %61, i64* %59, align 8, !dbg !43797, !tbaa !43775
  %62 = load i32, i32* %24, align 8, !dbg !43798, !tbaa !43750
  %63 = sub i32 %62, %26, !dbg !43798
  call void @llvm.dbg.value(metadata i32 %63, i64 0, metadata !43690, metadata !43695), !dbg !43800
  %64 = icmp eq i32 %63, 0, !dbg !43798
  br i1 %64, label %73, label %65, !dbg !43798, !prof !43801
```

Mutated

```
; <label>:58:                                     ; preds = %54
  %59 = getelementptr inbounds %struct.s2n_drbg, %struct.s2n_drbg* %0, i64 0, i32 0, !dbg !43796
  %60 = load i64, i64* %59, align 8, !dbg !43797, !tbaa !43775
  %61 = add i64 %60, 16, !dbg !43797
  store i64 %61, i64* %59, align 8, !dbg !43797, !tbaa !43775
  %62 = load i32, i32* %24, align 8, !dbg !43798, !tbaa !43750
  %63 = add i32 %62, %26, !dbg !43798
  %64 = sub i32 %62, %26, !dbg !43798
  call void @llvm.dbg.value(metadata i32 %63, i64 0, metadata !43690, metadata !43695), !dbg !43800
  %65 = icmp eq i32 %63, 0, !dbg !43798
  br i1 %65, label %74, label %66, !dbg !43798, !prof !43801
```

**Step 1**
PASS

## Source Code Analysis

In function `s2n_drbg_bits`, the behavior of `RESULT_CHECKED_MEMCPY`, which is defined [here](https://github.com/aws/s2n-tls/blob/9b700b663fa54c5a331c31aa7a06c22b146b4dcd/utils/s2n_safety_macros.h#L162), is not specified in the spec.

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
[saw spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake_io_lowlevel.saw#L419)


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

PASS

## Report Analysis

`    POSIX_ENSURE_REF(conn);`

`POSIX_ENSURE_REF` is defined [here](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/utils/s2n_safety_macros.h#L212), however this ensure behavior is not monitored in the spec function, which means if we switch the conditional branch, the prover can not detect the mutation

# 7

## Report Content

```
{
    "0": "null",
    "57": {
        "file_name": "s2n_socket.c",
        "function_name": "s2n_socket_write_uncork",
        "function_num": 25,
        "instruction_col": 5,
        "instruction_line": 170,
        "instruction_num": 515,
        "mutation_type": "ConstantInt",
        "opcode": "i32",
        "operand_num": 3,
        "mutated_type": "2"
    }
},

```

[Source Code](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/utils/s2n_socket.c#L170)

[Saw Spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake_io_lowlevel.saw#L419)

## Manual Test

**Step 0**


```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_socket.c -function_num 25 -instruction_num 515 -operand_num 3 -target_type 2 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc

```

Original

```
; <label>:14:                                     ; preds = %6
  %15 = getelementptr inbounds %struct.s2n_socket_read_io_context, %struct.s2n_socket_read_io_context* %10, i64 0, i32 0, !dbg !43749
  %16 = load i32, i32* %15, align 4, !dbg !43749, !tbaa !43750
  %17 = call i32 @setsockopt(i32 %16, i32 6, i32 3, i8* %7, i32 4) #16, !dbg !43752
  br label %18, !dbg !43753
```

Mutated

```
; <label>:14:                                     ; preds = %6
  %15 = getelementptr inbounds %struct.s2n_socket_read_io_context, %struct.s2n_socket_read_io_context* %10, i64 0, i32 0, !dbg !43749
  %16 = load i32, i32* %15, align 4, !dbg !43749, !tbaa !43750
  %17 = call i32 @setsockopt(i32 %16, i32 6, i32 -1, i8* %7, i32 4) #16, !dbg !43752
  br label %18, !dbg !43753
```

**Step 1**

PASS

## Source Code Analysis

```
    setsockopt(w_io_ctx->fd, IPPROTO_TCP, S2N_CORK, &optval, sizeof(optval));
```

Function `s2n_socket_write_uncork` calls function `setsockopt`, if we mutate the third parameter `S2N_CORK` which is defined [here](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/utils/s2n_socket.c#L27), the current proof can not catch this mutation.



# 8

## Report Content
```
{
    "0": "null",
    "92": {
        "file_name": "s2n_handshake_io.c",
        "function_name": "s2n_generate_new_client_session_id",
        "function_num": 428,
        "instruction_col": 38,
        "instruction_line": 700,
        "instruction_num": 10181,
        "mutation_type": "Binop",
        "opcode": "and",
        "operand_num": 0,
        "mutated_type": "or"
    }
},
```

[Source Code](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/tls/s2n_handshake_io.c#L699)

[Saw Spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake_io_lowlevel.saw#L433)



## Manual Test

**Step 0**

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_handshake_io.c -function_num 428 -instruction_num 10168 -operand_num 2 -target_type 2 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc

```

```
  %2 = alloca %struct.s2n_blob, align 8
  tail call void @llvm.dbg.value(metadata %struct.s2n_connection* %0, i64 0, metadata !43681, metadata !43685), !dbg !43686
  %3 = getelementptr inbounds %struct.s2n_connection, %struct.s2n_connection* %0, i64 0, i32 12, !dbg !43687
  %4 = load i32, i32* %3, align 8, !dbg !43687, !tbaa !43688
  %5 = icmp eq i32 %4, 0, !dbg !43721
  br i1 %5, label %6, label %20, !dbg !43722
```

```
  %2 = alloca %struct.s2n_blob, align 8
  tail call void @llvm.dbg.value(metadata %struct.s2n_connection* %0, i64 0, metadata !43681, metadata !43685), !dbg !43686
  %3 = getelementptr inbounds %struct.s2n_connection, %struct.s2n_connection* %0, i64 0, i32 12, !dbg !43687
  %4 = load i32, i32* %3, align 8, !dbg !43687, !tbaa !43688
  %5 = icmp eq i32 %4, -1, !dbg !43721
  br i1 %5, label %6, label %20, !dbg !43722
```


**Step 1**
PASS

## Source Code Analysis

```
    if (conn->mode == S2N_SERVER) {
```

When we mutate this condition into `conn->mode == -1`, the prover can not catch this mutation.

This is caused by the spec of `s2n_generate_new_client_session_id_spec`:

```
// Specification for s2n_generate_new_client_session_id. This is essentially
// a noop function that returns 0 from the perspective of our current proof
let s2n_generate_new_client_session_id_spec = do {
    pconn <- crucible_alloc_readonly (llvm_struct "struct.s2n_connection");
   
    crucible_execute_func [pconn];

    crucible_return (crucible_term {{ 0 : [32] }});
};
```
# 9

## Report Content

```
{
    "0": "null",
    "553": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_mix",
        "function_num": 485,
        "instruction_col": 17,
        "instruction_line": 155,
        "instruction_num": 12587,
        "mutation_type": "Binop",
        "opcode": "add",
        "operand_num": 0,
        "mutated_type": "sub"
    }
},

```
[Source Code](https://github.com/watssec/s2n-tls/blob/05b685125feb3e68da677c13f8ecbc490d4bbca6/crypto/s2n_drbg.c#L155)

[Saw Spec](https://github.com/aws/s2n-tls/blob/247bf3151194c3326e1c9f63cb1b1c06ab30f1ea/tests/saw/spec/DRBG/DRBG.saw#L403)

## Manual Test

**Step 0**

```
opt -load ../../mutation/passes/Binop/build/Mutation/libMutation.so -file_name s2n_drbg.c -function_num 485 -instruction_num 12587 -target_type sub -Binop < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:30:                                     ; preds = %27
  %31 = getelementptr inbounds %struct.s2n_drbg, %struct.s2n_drbg* %0, i64 0, i32 3, !dbg !43758
  %32 = load i64, i64* %31, align 8, !dbg !43759, !tbaa !43760
  %33 = add i64 %32, 1, !dbg !43759
  store i64 %33, i64* %31, align 8, !dbg !43759, !tbaa !43760
  br label %34, !dbg !43761
```

Mutated

```
; <label>:30:                                     ; preds = %27
  %31 = getelementptr inbounds %struct.s2n_drbg, %struct.s2n_drbg* %0, i64 0, i32 3, !dbg !43758
  %32 = load i64, i64* %31, align 8, !dbg !43759, !tbaa !43760
  %33 = sub i64 %32, 1, !dbg !43759
  %34 = add i64 %32, 1, !dbg !43759
  store i64 %33, i64* %31, align 8, !dbg !43759, !tbaa !43760
  br label %35, !dbg !43761
```

**Step 1**

PASS

# Source Code Analysis

```
drbg->mixes += 1;
```

At the end of function `s2n_drbg_mix`, the value of `drbg->mixes` is increased by 1, however this behavior is not monitored by the spec function.

# 10

## Report Content 

```
{
    "0": "null",
    "29": {
        "file_name": "s2n_socket.c",
        "function_name": "s2n_socket_was_corked",
        "function_num": 23,
        "instruction_col": 16,
        "instruction_line": 134,
        "instruction_num": 438,
        "mutation_type": "Binop",
        "opcode": "and",
        "operand_num": 0,
        "mutated_type": "xor"
    }
},
```
[Source Code](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/utils/s2n_socket.c#L134)
[Saw Spec](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/tests/saw/spec/handshake/handshake_io_lowlevel.saw#L471)

## Manual Test

**Step 0**
```
 opt -load ../../mutation/passes/Binop/build/Mutation/libMutation.so -file_name s2n_handshake_io.c -function_num 23 -instruction_num 438 -target_type xor -Binop < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:5:                                      ; preds = %1
  %6 = bitcast %struct.s2n_connection* %0 to i32*, !dbg !43704
  %7 = load i32, i32* %6, align 8, !dbg !43704
  %8 = and i32 %7, 512, !dbg !43704
  %9 = icmp eq i32 %8, 0, !dbg !43706
  br i1 %9, label %10, label %24, !dbg !43707
```

Mutated

```
; <label>:5:                                      ; preds = %1
  %6 = bitcast %struct.s2n_connection* %0 to i32*, !dbg !43704
  %7 = load i32, i32* %6, align 8, !dbg !43704
  %8 = xor i32 %7, 512, !dbg !43704
  %9 = and i32 %7, 512, !dbg !43704
  %10 = icmp eq i32 %8, 0, !dbg !43706
  br i1 %10, label %11, label %25, !dbg !43707
```

**Step 1**

PASS

## Source Code Analysis


This happens because the spec function for `s2n_socket_was_corked` function is omitted, which means the spec for this conditional branch is missing .

```
    if (!conn->managed_send_io || !conn->send) {
        return 0;
    }
```

# Reference

[Saw Manual](https://saw.galois.com/manual.html)
