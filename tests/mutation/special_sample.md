# 0 Mutation Categories

## I Binop

Iterate through instructions, when we find a Binop operator, randomly mutate it into another Binop according to the following rule:

```
{"Binop":["add","sub","mul"],"fBinop":["fadd","fsub","fmul"],"udiv":["udiv","urem"],"fdiv":["fdiv","frem"],"sdiv":["sdiv","srem"],"BitwiseBinop":["shl","lshr","ashr"],"BitwiseCompare":["and","or","xor"]}
```

## II Branch

switch the operand of `br`
## III ICmp/FCmp

switch the operand of `icmp`/`fcmp`

## IV ConstantInt/FP


# Existing Problems

## We neglect getElementPtr in Initialization

A counter example 

### Report Content 

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
[Source code](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/crypto/s2n_drbg.c#L95)

### Manual Test

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_drbg.c  -function_num 480  -instruction_num 12258 -operand_num 2 -target_type 2 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:18:                                     ; preds = %14
  %19 = tail call i32 @EVP_CIPHER_CTX_key_length(%struct.evp_cipher_ctx_st* nonnull %10) #16, !dbg !43740
  %20 = add nsw i32 %19, 16, !dbg !43742
  %21 = getelementptr inbounds [48 x i8], [48 x i8]* %3, i64 0, i64 0, !dbg !43742
  call void @llvm.lifetime.start(i64 48, i8* %21) #16, !dbg !43743
  tail call void @llvm.dbg.declare(metadata [48 x i8]* %3, metadata !43686, metadata !43695), !dbg !43742
  call void @llvm.memset.p0i8.i64(i8* %21, i8 0, i64 48, i32 16, i1 false), !dbg !43745
  %22 = icmp ult i32 %20, 49, !dbg !43747
  br i1 %22, label %25, label %23, !dbg !43747
```

Mutated

```
; <label>:18:                                     ; preds = %14
  %19 = tail call i32 @EVP_CIPHER_CTX_key_length(%struct.evp_cipher_ctx_st* nonnull %10) #16, !dbg !43740
  %20 = add nsw i32 %19, 16, !dbg !43742
  %21 = getelementptr inbounds [48 x i8], [48 x i8]* %3, i64 -1, i64 0, !dbg !43742
  call void @llvm.lifetime.start(i64 48, i8* %21) #16, !dbg !43743
  tail call void @llvm.dbg.declare(metadata [48 x i8]* %3, metadata !43686, metadata !43695), !dbg !43742
  call void @llvm.memset.p0i8.i64(i8* %21, i8 0, i64 48, i32 16, i1 false), !dbg !43745
  %22 = icmp ult i32 %20, 49, !dbg !43747
  br i1 %22, label %25, label %23, !dbg !43747

```

`  %21 = getelementptr inbounds [48 x i8], [48 x i8]* %3, i64 -1, i64 0, !dbg !43742`, the [first index](https://llvm.org/docs/GetElementPtr.html#what-is-the-first-index-of-the-gep-instruction) is mutated. However, this only leads to confusion instead of useful mutation (symbolic execution fail).

## We neglect all the intrinsic function in a operand position

Search for `intrinsic` [here](https://llvm.org/docs/LangRef.html)



### A counter example

In this example, the debug information is preserved as we want to make getting back to source code easier.

#### Report Content

```
{
    "0": "null",
    "794": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_bits",
        "function_num": 481,
        "instruction_col": 21,
        "instruction_line": 64,
        "instruction_num": 12407,
        "mutation_type": "ConstantInt",
        "opcode": "i64",
        "operand_num": 2,
        "mutated_type": "1"
    }
},
```
#### Manual Test

**Step 0**

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_handshake_io.c -function_num 481 -instruction_num 12407 -operand_num 2 -target_type 1 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:50:                                     ; preds = %47
  %51 = getelementptr inbounds [16 x i8], [16 x i8]* %4, i64 0, i64 0, !dbg !43782
  call void @llvm.lifetime.start(i64 16, i8* %51) #16, !dbg !43782
  call void @llvm.dbg.declare(metadata [16 x i8]* %4, metadata !43689, metadata !43695), !dbg !43783
  call void @llvm.dbg.value(metadata %struct.s2n_blob* %3, i64 0, metadata !43685, metadata !43747), !dbg !43748
  %52 = call i32 @s2n_increment_drbg_counter(%struct.s2n_blob* nonnull %3), !dbg !43784
  %53 = call zeroext i1 @s2n_result_is_ok(i32 0) #16, !dbg !43788
  br i1 %53, label %54, label %74, !dbg !43784

```
Mutated

```
; <label>:50:                                     ; preds = %47
  %51 = getelementptr inbounds [16 x i8], [16 x i8]* %4, i64 0, i64 0, !dbg !43782
  call void @llvm.lifetime.start(i64 16, i8* %51) #16, !dbg !43782
  call void @llvm.dbg.declare(metadata [16 x i8]* %4, metadata !43689, metadata !43695), !dbg !43783
  call void @llvm.dbg.value(metadata %struct.s2n_blob* %3, i64 1, metadata !43685, metadata !43747), !dbg !43748
  %52 = call i32 @s2n_increment_drbg_counter(%struct.s2n_blob* nonnull %3), !dbg !43784
  %53 = call zeroext i1 @s2n_result_is_ok(i32 0) #16, !dbg !43788
  br i1 %53, label %54, label %74, !dbg !43784
```

It is of no use to mutate the parameter in @llvm.dbg.value. And the method to fix this is to filter out intrinsicinst/ and intrinsic functions in the operand



### Another counter example (tail call)

#### Report Content

```
{
    "0": "null",
    "756": {
        "file_name": "s2n_connection.c",
        "function_name": "s2n_connection_get_client_auth_type",
        "function_num": 574,
        "instruction_col": 90,
        "instruction_line": 813,
        "instruction_num": 14763,
        "mutation_type": "ConstantInt",
        "opcode": "i64",
        "operand_num": 2,
        "mutated_type": "4"
    }
},
```

#### Manual Test
```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_connection.c -function_num 574 -instruction_num 14763 -operand_num 2 -target_type 4 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
  tail call void @llvm.dbg.value(metadata %struct.s2n_connection* %0, i64 0, metadata !43684, metadata !43686), !dbg !43687
  tail call void @llvm.dbg.value(metadata i32* %1, i64 0, metadata !43685, metadata !43686), !dbg !43688
  %3 = icmp eq %struct.s2n_connection* %0, null, !dbg !43689
  br i1 %3, label %4, label %6, !dbg !43689

```
Mutated

```
new B 
  tail call void @llvm.dbg.value(metadata %struct.s2n_connection* %0, i64 0, metadata !43684, metadata !43686), !dbg !43687
  tail call void @llvm.dbg.value(metadata i32* %1, i64 -1, metadata !43685, metadata !43686), !dbg !43688
  %3 = icmp eq %struct.s2n_connection* %0, null, !dbg !43689
  br i1 %3, label %4, label %6, !dbg !43689

```

### Another Another counter example

An example on llvm.mempy

```
{
    "0": "null",
    "600": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_bits",
        "function_num": 481,
        "instruction_col": 21,
        "instruction_line": 64,
        "instruction_num": 12367,
        "mutation_type": "ConstantInt",
        "opcode": "i64",
        "operand_num": 3,
        "mutated_type": "4"
    }
},

```

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_drbg.c -function_num 481 -instruction_num 12367 -operand_num 3 -target_type 4 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc

```

Original

```
; <label>:18:                                     ; preds = %14
  %19 = bitcast %struct.s2n_blob* %3 to i8*, !dbg !43740
  call void @llvm.lifetime.start(i64 24, i8* %19) #16, !dbg !43740
  call void @llvm.memset.p0i8.i64(i8* %19, i8 0, i64 24, i32 8, i1 false), !dbg !43741
  %20 = getelementptr inbounds %struct.s2n_drbg, %struct.s2n_drbg* %0, i64 0, i32 2, i64 0, !dbg !43743
  tail call void @llvm.dbg.value(metadata %struct.s2n_blob* %3, i64 0, metadata !43685, metadata !43747), !dbg !43748
  %21 = call i32 @s2n_blob_init(%struct.s2n_blob* nonnull %3, i8* %20, i32 16) #16, !dbg !43743
  %22 = icmp sgt i32 %21, -1, !dbg !43743
  br i1 %22, label %23, label %76, !dbg !43743

```

Mutated

```
; <label>:18:                                     ; preds = %14
  %19 = bitcast %struct.s2n_blob* %3 to i8*, !dbg !43740
  call void @llvm.lifetime.start(i64 24, i8* %19) #16, !dbg !43740
  call void @llvm.memset.p0i8.i64(i8* %19, i8 0, i64 23, i32 8, i1 false), !dbg !43741
  %20 = getelementptr inbounds %struct.s2n_drbg, %struct.s2n_drbg* %0, i64 0, i32 2, i64 0, !dbg !43743
  tail call void @llvm.dbg.value(metadata %struct.s2n_blob* %3, i64 0, metadata !43685, metadata !43747), !dbg !43748
  %21 = call i32 @s2n_blob_init(%struct.s2n_blob* nonnull %3, i8* %20, i32 16) #16, !dbg !43743
  %22 = icmp sgt i32 %21, -1, !dbg !43743
  br i1 %22, label %23, label %76, !dbg !43743

```

# Safety Macro



## Report Content

```
    {
        "0": "null",
        "743": {
            "file_name": "s2n_drbg.c",
            "function_name": "s2n_drbg_bytes_used",
            "function_num": 488,
            "instruction_col": 5,
            "instruction_line": 243,
            "instruction_num": 12630,
            "mutation_type": "ConstantInt",
            "opcode": "i32",
            "operand_num": 1,
            "mutated_type": "0"
        }
    },

```
[Source code](https://github.com/aws/s2n-tls/blob/282c0dd405bd6fd6127551b8a480335e41733c1d/crypto/s2n_drbg.c#L243)



**Step 0**

```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_drbg.c -function_num 488 -instruction_num 12630 -operand_num 1 -target_type 0 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:4:                                      ; preds = %2
  store i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.17.540, i64 0, i64 0), i8** @s2n_debug_str, align 8, !dbg !43693, !tbaa !43698
  store i32 402653193, i32* @s2n_errno, align 4, !dbg !43693, !tbaa !43702
  %5 = tail call i32 @s2n_calculate_stacktrace() #16, !dbg !43693
  br label %13, !dbg !43704
```

Mutated

```
; <label>:4:                                      ; preds = %2
  store i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.17.540, i64 0, i64 0), i8** @s2n_debug_str, align 8, !dbg !43693, !tbaa !43698
  store i32 0, i32* @s2n_errno, align 4, !dbg !43693, !tbaa !43702
  %5 = tail call i32 @s2n_calculate_stacktrace() #16, !dbg !43693
  br label %13, !dbg !43704
```
**Step 1**
Pass


## Source Code Analysis

`RESULT_ENSURE_REF` is defined in [here](https://github.com/aws/s2n-tls/blob/9b700b663fa54c5a331c31aa7a06c22b146b4dcd/utils/s2n_safety_macros.h#L120)

`s2n_errno` is defined [here](https://github.com/aws/s2n-tls/blob/dd9cd2bad2a6e903485aeba569b8a8915317ded6/error/s2n_errno.h#L302)

Explanation for `s2n_errno` in [md](https://github.com/aws/s2n-tls/blob/e9906015bbc19ba17dd26846b84210c93dcc6e13/docs/USAGE-GUIDE.md#error-handling)

`store i32 402653193, i32* @s2n_errno, align 4, !dbg !43693, !tbaa !43702`


# Symbolic execution fail


## Report Content
```
{
    "0": "null",
    "493": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_increment_drbg_counter",
        "function_num": 470,
        "instruction_col": 9,
        "instruction_line": 37,
        "instruction_num": 11981,
        "mutation_type": "Binop",
        "opcode": "and",
        "operand_num": 0,
        "mutated_type": "xor"
    }
},
```
[Source code](https://github.com/aws/s2n-tls/blob/7f1017ee9b09ab6910f1d2bf56135663ca0b12c5/crypto/s2n_drbg.c#L32)
[saw Spec](https://github.com/watssec/s2n-tls/blob/caa01e02da72abc24ca64d814835582199c8955c/tests/saw/spec/DRBG/DRBG.saw#L257)
## Source Code Analysis

A for loop:

```
int s2n_increment_drbg_counter(struct s2n_blob *counter)
{
    for (uint32_t i = counter->size; i > 0; i--) {
        counter->data[i-1] += 1;
        if (counter->data[i-1]) {
            break;
        }

       /* seq[i] wrapped, so let it carry */
    }
    return 0;
}
```

## Manual Test

**Step 0**

```
opt -load ../../mutation/passes/Binop/build/Mutation/libMutation.so -function_num 470 -instruction_num 11981 -target_type xor -Binop < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
``` 

Original
```
; Function Attrs: nounwind sspreq uwtable
define hidden i32 @s2n_increment_drbg_counter(%struct.s2n_blob* nocapture readonly) local_unnamed_addr #0 !dbg !59092 {
  tail call void @llvm.dbg.value(metadata %struct.s2n_blob* %0, i64 0, metadata !59096, metadata !43704), !dbg !59099
  %2 = getelementptr inbounds %struct.s2n_blob, %struct.s2n_blob* %0, i64 0, i32 1, !dbg !59100
  %3 = load i32, i32* %2, align 8, !dbg !59100, !tbaa !45455
  tail call void @llvm.dbg.value(metadata i32 %3, i64 0, metadata !59097, metadata !43704), !dbg !59101
  %4 = getelementptr inbounds %struct.s2n_blob, %struct.s2n_blob* %0, i64 0, i32 0, !dbg !59102
  %5 = zext i32 %3 to i64, !dbg !59105
  br label %6, !dbg !59105

; <label>:6:                                      ; preds = %9, %1
  %7 = phi i64 [ %20, %9 ], [ %5, %1 ]
  %8 = icmp eq i64 %7, 0, !dbg !59106
  br i1 %8, label %21, label %9, !dbg !59108

; <label>:9:                                      ; preds = %6
  %10 = add i64 %7, 4294967295, !dbg !43703
  %11 = and i64 %10, 4294967295, !dbg !43704
  %12 = load i8*, i8** %4, align 8, !dbg !43696, !tbaa !43705
  %13 = getelementptr inbounds i8, i8* %12, i64 %11, !dbg !43704
  %14 = load i8, i8* %13, align 1, !dbg !43706, !tbaa !43707
  %15 = add i8 %14, 1, !dbg !43706
  store i8 %15, i8* %13, align 1, !dbg !43706, !tbaa !43707
  %16 = load i8*, i8** %4, align 8, !dbg !43708, !tbaa !43705
  %17 = getelementptr inbounds i8, i8* %16, i64 %11, !dbg !43710
  %18 = load i8, i8* %17, align 1, !dbg !43710, !tbaa !43707
  %19 = icmp eq i8 %18, 0, !dbg !43710
  %20 = add nsw i64 %7, -1, !dbg !43711
  br i1 %19, label %6, label %21, !dbg !43711

```
Mutated

```
<pre>; &lt;label&gt;:9:                                      ; preds = %6
  %10 = add i64 %7, 4294967295, !dbg !43703
  %11 = xor i64 %10, 4294967295, !dbg !43704
  %12 = and i64 %10, 4294967295, !dbg !43704
  %13 = load i8*, i8** %4, align 8, !dbg !43696, !tbaa !43705
  %14 = getelementptr inbounds i8, i8* %13, i64 %11, !dbg !43704
  %15 = load i8, i8* %14, align 1, !dbg !43706, !tbaa !43707
  %16 = add i8 %15, 1, !dbg !43706
  store i8 %16, i8* %14, align 1, !dbg !43706, !tbaa !43707
  %17 = load i8*, i8** %4, align 8, !dbg !43708, !tbaa !43705
  %18 = getelementptr inbounds i8, i8* %17, i64 %11, !dbg !43710
  %19 = load i8, i8* %18, align 1, !dbg !43710, !tbaa !43707
  %20 = icmp eq i8 %19, 0, !dbg !43710
  %21 = add nsw i64 %7, -1, !dbg !43711
  br i1 %20, label %6, label %22, !dbg !43711
</pre>
```


**Step 1**


in `verify_drbg.saw` -> symbolic execution failed


# Others



## Report Content

```

{
    "0": "null",
    "689": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_generate",
        "function_num": 484,
        "instruction_col": 5,
        "instruction_line": 207,
        "instruction_num": 12512,
        "mutation_type": "ICmp",
        "opcode": "icmp",
        "operand_num": 0,
        "mutated_type": "no target"
    }
},


```

[Source code location](https://github.com/watssec/s2n-tls/blob/fa123d7f6b1ffcecd8b8a89ac2befdb6635dee78/crypto/s2n_drbg.c#L207)



 
## Manual Test

```
 opt -load ../../mutation/passes/ICmp/build/Mutation/libMutation.so -file_name s2n_drbg.c -function_num 484 -instruction_num 12512 -ICmp < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

Original

```
; <label>:21:                                     ; preds = %14
  %22 = bitcast %struct.s2n_blob* %4 to i8*, !dbg !43740
  call void @llvm.lifetime.start(i64 24, i8* %22) #16, !dbg !43740
  call void @llvm.memset.p0i8.i64(i8* %22, i8 0, i64 24, i32 8, i1 false), !dbg !43742
  tail call void @llvm.dbg.value(metadata %struct.s2n_blob* %4, i64 0, metadata !43687, metadata !43744), !dbg !43724
  %23 = call i32 @s2n_blob_init(%struct.s2n_blob* nonnull %4, i8* %17, i32 %16) #16, !dbg !43745
  %24 = icmp sgt i32 %23, -1, !dbg !43745
  br i1 %24, label %25, label %42, !dbg !43745
```

Mutated
```
; <label>:21:                                     ; preds = %14
  %22 = bitcast %struct.s2n_blob* %4 to i8*, !dbg !43740
  call void @llvm.lifetime.start(i64 24, i8* %22) #16, !dbg !43740
  call void @llvm.memset.p0i8.i64(i8* %22, i8 0, i64 24, i32 8, i1 false), !dbg !43742
  tail call void @llvm.dbg.value(metadata %struct.s2n_blob* %4, i64 0, metadata !43687, metadata !43744), !dbg !43724
  %23 = call i32 @s2n_blob_init(%struct.s2n_blob* nonnull %4, i8* %17, i32 %16) #16, !dbg !43745
  %24 = icmp sgt i32 -1, %23, !dbg !43745
  %25 = icmp sgt i32 %23, -1, !dbg !43745
  br i1 %24, label %26, label %43, !dbg !43745
```

`RESULT_STACK_BLOB` is defined in [here](https://github.com/aws/s2n-tls/blob/5739bcdefcdd6d4966cf3c1ed1ef6295b3eff6fb/utils/s2n_blob.h#L57)

`    RESULT_GUARD_POSIX(s2n_blob_init(&name, name ## _buf, name ## _requested_size))`

`RESULT_GUARD_POSIX` is defined [here](https://github.com/aws/s2n-tls/blob/9b700b663fa54c5a331c31aa7a06c22b146b4dcd/utils/s2n_safety_macros.h#L191)

Even if `RESULT_GUARD_POSIX`returns error, this behavior is not specified in the spec function.
```
[03:21:41.562] Stack trace:
"include" (/home/r2ji/s2n-tls/tests/mutation/verify_drbg.saw:1:1-1:8):
"crucible_llvm_verify" (/home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:407:16-407:36):
at /home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:332:5
error when loading through pointer that appeared in the override's points-to precondition(s):
Precondition:
  Pointer: concrete pointer: allocation = 2019, offset = 0
  Pointee: let { x@1 = Cryptol.TCNum 8
                 x@2 = Cryptol.TCNum 32
                 x@3 = Cryptol.TCNum 256
                 x@4 = Prelude.Vec 16 (Prelude.Vec 8 Prelude.Bool)
                 x@5 = Prelude.Vec 256 Prelude.Bool
                 x@6 = Cryptol.tcMul x@2 x@1
      }
   in Cryptol.ecSplit x@2 x@1 Prelude.Bool
        (Prelude.coerce x@5 (Cryptol.seq x@6 Prelude.Bool)
           (Cryptol.seq_cong1 x@3 x@6 Prelude.Bool
              (Prelude.unsafeAssert Cryptol.Num x@3 x@6))
           (cryptol:res#4987 fresh:fake_entropy#850 fresh:key#895
              fresh:drbg-%3ebytes_used#896
              fresh:drbg-%3ev#898).1) : [32][8]

  Assertion made at: /home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:332:5
Failure reason: 
  When reading through pointer: (2019, 0x0:[64])
  in the  postcondition of an override
  Tried to read something of size: 32
  And type: [32 x i8]
  Found 1 possibly matching allocation(s):
  - HeapAlloc 2019 0x20:[64] Mutable 1-byte-aligned /home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:99:14

```



# Another false positive case

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

[Spec location]()


 
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

Mutate the condition from 

mutate the constant value [S2N_SUCCESS](https://github.com/aws/s2n-tls/blob/282c0dd405bd6fd6127551b8a480335e41733c1d/api/s2n.h#L50) from 0 ->1



**Step 1** check saw verification

pass