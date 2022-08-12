# 

```
{
    "0": "null",
    "267": {
        "file_name": "s2n_drbg.c",
        "function_name": "s2n_drbg_instantiate",
        "function_num": 471,
        "instruction_col": 5,
        "instruction_line": 199,
        "instruction_num": 12118,
        "mutation_type": "ConstantInt",
        "opcode": "i1",
        "operand_num": 2,
        "mutated_type": "0",
        "timestamp": "2022-08-08 13:06:10.206045"
    },
    "449": {
        "file_name": "s2n_blob.c",
        "function_name": "s2n_blob_init",
        "function_num": 784,
        "instruction_col": 10,
        "instruction_line": 43,
        "instruction_num": 20104,
        "mutation_type": "ConstantInt",
        "opcode": "i32",
        "operand_num": 1,
        "mutated_type": "2",
        "timestamp": "2022-08-08 13:06:10.206524"
    }
},
```
# Manual Test

[Source Code 1](https://github.com/watssec/s2n-tls/blob/36c825bebd94dc651fcf5e9705f25fc2171e1bfb/crypto/s2n_drbg.c#L199)
[Source Code 2](https://github.com/watssec/s2n-tls/blob/36c825bebd94dc651fcf5e9705f25fc2171e1bfb/utils/s2n_blob.c#L43)


## Test Seperately


```
opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_drbg.c -function_num 471 -instruction_num 12118 -operand_num 2 -target_type 0 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

```
Running check sat
[18:45:21.935] Subgoal failed: s2n_drbg_instantiate safety assertion:
/home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:405:19: error: in _SAW_verify_prestate
Literal equality postcondition
Expected term: Cryptol.ecNumber (Cryptol.TCNum 0) (Prelude.Vec 32 Prelude.Bool)
  (Cryptol.PLiteralSeqBool (Cryptol.TCNum 32))
Actual term:   Prelude.bvNat 32 4294967295


[18:45:21.936] SolverStats {solverStatsSolvers = fromList ["W4 ->yices"], solverStatsGoalSize = 29}
[18:45:21.936] ----------Counterexample----------
[18:45:21.936] <<All settings of the symbolic variables constitute a counterexample>>
[18:45:21.936] ----------------------------------
[18:45:21.936] Stack trace:
"include" (/home/r2ji/s2n-tls/tests/mutation/verify_drbg.saw:1:1-1:8):
"crucible_llvm_verify" (/home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:405:19-405:39):
Proof failed.

```



```
 opt -load ../../mutation/passes/ConstantInt/build/Mutation/libMutation.so -file_name s2n_blob.c -function_num 784 -instruction_num 20104 -operand_num 1 -target_type 2 -ConstantInt < ./bitcode/all_llvm.bc > ./bitcode/all_llvm_mutated.bc
```

```
"include" (/home/r2ji/s2n-tls/tests/mutation/verify_drbg.saw:1:1-1:8):
"crucible_llvm_verify" (/home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:397:12-397:32):
Symbolic execution failed.
Abort due to assertion failure:
  s2n_blob.c:32:5: error: in s2n_blob_validate
  Global symbol not allocated
  Details:
    Global symbol "s2n_debug_str" has no associated allocation
Stack frame s2n_blob_validate
  No writes or allocations
Stack frame s2n_blob_init
  Allocations:
    StackAlloc 2022 0x7:[64] Mutable 1-byte-aligned internal
  Writes:
    memcopy (2020, 0x11:[64]) (2022, 0x0:[64]) 0x7:[64]
    Indexed chunk:
      2020 |->   memset (2020, 0x10:[64]) 0x0:[8] 0x1:[64]
                 *(2020, 0xc:[64]) := 0xffffffff:[32]
                 *(2020, 0x8:[64]) := 0x10:[32]
                 *(2020, 0x0:[64]) := (2016, 0x10:[64])
Stack frame s2n_drbg_bits
  Allocations:
    StackAlloc 2021 0x10:[64] Mutable 16-byte-aligned internal
    StackAlloc 2020 0x18:[64] Mutable 8-byte-aligned internal
  Writes:
    Indexed chunk:
      2020 |->   memset (2020, 0x0:[64]) 0x0:[8] 0x18:[64]

```

When combined together -> symbolic execution fail

#


```
    {
        "0": "null",
        "267": {
            "file_name": "s2n_drbg.c",
            "function_name": "s2n_drbg_instantiate",
            "function_num": 471,
            "instruction_col": 5,
            "instruction_line": 199,
            "instruction_num": 12118,
            "mutation_type": "ConstantInt",
            "opcode": "i1",
            "operand_num": 2,
            "mutated_type": "1",
            "timestamp": "2022-08-08 13:10:58.298275"
        },
        "338": {
            "file_name": "s2n_drbg.c",
            "function_name": "s2n_drbg_bits",
            "function_num": 481,
            "instruction_col": 23,
            "instruction_line": 69,
            "instruction_num": 12378,
            "mutation_type": "ICmp",
            "opcode": "icmp",
            "operand_num": 0,
            "mutated_type": "no target",
            "timestamp": "2022-08-08 13:10:58.298304"
        }
    },
```

==>

```
[19:06:13.904] Stack trace:
"include" (/home/r2ji/s2n-tls/tests/mutation/verify_drbg.saw:1:1-1:8):
"crucible_llvm_verify" (/home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:397:12-397:32):
at /home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:272:5
error when loading through pointer that appeared in the override's points-to precondition(s):
Precondition:
  Pointer: concrete pointer: allocation = 2019, offset = 0
  Pointee: let { x@1 = Cryptol.TCNum 8
                 x@2 = Cryptol.TCNum 32
                 x@3 = Cryptol.TCNum 16
                 x@4 = Cryptol.TCNum 256
                 x@5 = Prelude.Vec 16 (Prelude.Vec 8 Prelude.Bool)
                 x@6 = Cryptol.tcMul x@2 x@1
      }
   in Cryptol.ecSplit x@2 x@1 Prelude.Bool
        (Prelude.coerce (Prelude.Vec 256 Prelude.Bool)
           (Cryptol.seq x@6 Prelude.Bool)
           (Cryptol.seq_cong1 x@4 x@6 Prelude.Bool
              (Prelude.unsafeAssert Cryptol.Num x@4 x@6))
           (cryptol:/DRBG/drbg_generate_internal x@4 (Cryptol.TCNum 2)
              (fresh:drbg-%3ebytes_used#863,
               Cryptol.ecJoin x@3 x@1 Prelude.Bool fresh:key#862,
               Cryptol.ecJoin x@3 x@1 Prelude.Bool
                 fresh:drbg-%3ev#865)).1) : [32][8]

  Assertion made at: /home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:272:5
Failure reason: 
  When reading through pointer: (2019, 0x0:[64])
  in the  postcondition of an override
  Tried to read something of size: 32
  And type: [32 x i8]
  Found 1 possibly matching allocation(s):
  - HeapAlloc 2019 0x20:[64] Mutable 1-byte-aligned /home/r2ji/s2n-tls/tests/mutation/spec/DRBG/DRBG.saw:99:14

```


#

```
    {
        "0": "null",
        "311": {
            "file_name": "s2n_drbg.c",
            "function_name": "s2n_drbg_update",
            "function_num": 480,
            "instruction_col": 5,
            "instruction_line": 95,
            "instruction_num": 12274,
            "mutation_type": "Branch",
            "opcode": "br",
            "operand_num": 0,
            "mutated_type": "no target",
            "timestamp": "2022-08-09 10:17:19.407762"
        },
        "351": {
            "file_name": "s2n_drbg.c",
            "function_name": "s2n_drbg_bits",
            "function_num": 481,
            "instruction_col": 9,
            "instruction_line": 75,
            "instruction_num": 12403,
            "mutation_type": "Branch",
            "opcode": "br",
            "operand_num": 0,
            "mutated_type": "no target",
            "timestamp": "2022-08-09 10:17:19.408161"
        },
        "343": {
            "file_name": "s2n_drbg.c",
            "function_name": "s2n_drbg_bits",
            "function_num": 481,
            "instruction_col": 9,
            "instruction_line": 71,
            "instruction_num": 12394,
            "mutation_type": "Branch",
            "opcode": "br",
            "operand_num": 0,
            "mutated_type": "no target",
            "timestamp": "2022-08-09 10:17:19.408165"
        }
    }
```