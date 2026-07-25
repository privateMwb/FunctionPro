# FunctionProBenchmark Results

## Bool Check

| Test | Iteration | FunctionPro | std::function | Δ |
|---|---|---|---|---|
| Fn::operator Bool() | 10K | 10.54 us | 10.69 us | +1.5% |
| Fn::operator Bool() | 100K | 105.46 us | 105.46 us | +0.0% |
| Fn::operator Bool() | 1M | 1.09 ms | 1.05 ms | -3.6% |

## Bool Check

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::operator Bool() | 10K | 5.23 us |
| MoveOnlyFn::operator Bool() | 100K | 157.92 us |
| MoveOnlyFn::operator Bool() | 1M | 1.58 ms |

## Bool Check

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef::operator Bool() | 10K | 10.69 us |
| FnRef::operator Bool() | 100K | 52.85 us |
| FnRef::operator Bool() | 1M | 567.69 us |

## Invoke

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::operator() | 10K | 26.85 us | 34.15 us | +27.2% |
| Fn::operator() | 100K | 246.23 us | 338.46 us | +37.5% |
| Fn::operator() | 1M | 2.63 ms | 3.41 ms | +29.5% |
| Fn::operator() (empty) | 10K | 103.67 ms | 128.59 ms | +24.0% |
| Fn::operator() (empty) | 100K | 2.08 s | 3.19 s | +53.8% |
| Fn::operator() (empty) | 1M | 18.46 s | 14.92 s | -19.2% |

## Invoke

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::operator() | 10K | 32.38 us |
| MoveOnlyFn::operator() | 100K | 370.15 us |
| MoveOnlyFn::operator() | 1M | 4.56 ms |
| MoveOnlyFn::operator() (empty) | 10K | 121.58 ms |
| MoveOnlyFn::operator() (empty) | 100K | 1.23 s |
| MoveOnlyFn::operator() (empty) | 1M | 26.68 s |

## Invoke

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef::operator() | 10K | 6.23 us |
| FnRef::operator() | 100K | 61.62 us |
| FnRef::operator() | 1M | 1.23 ms |
| FnRef::operator() (empty) | 10K | 222.64 ms |
| FnRef::operator() (empty) | 100K | 2.26 s |
| FnRef::operator() (empty) | 1M | 22.99 s |
| FnRef::operator() (function Pointer) | 10K | 12.38 us |
| FnRef::operator() (function Pointer) | 100K | 123.15 us |
| FnRef::operator() (function Pointer) | 1M | 615.62 us |

## Assign

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Copy-assign | 10K | 136.31 us | 450.46 us | +230.5% |
| Fn Copy-assign | 100K | 1.29 ms | 4.62 ms | +256.9% |
| Fn Copy-assign | 1M | 13.28 ms | 45.41 ms | +241.9% |
| Fn Move-assign | 10K | 87.08 us | 319.62 us | +267.0% |
| Fn Move-assign | 100K | 800.62 us | 2.77 ms | +246.0% |
| Fn Move-assign | 1M | 8.44 ms | 29.26 ms | +246.6% |

## Assign

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Move-assign | 10K | 80.62 us |
| MoveOnlyFn Move-assign | 100K | 800.85 us |
| MoveOnlyFn Move-assign | 1M | 8.75 ms |

## Assign

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Assign | 10K | 6.23 us |
| FnRef Assign | 100K | 178.23 us |
| FnRef Assign | 1M | 1.23 ms |

## Bind

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Bind (small) | 10K | 74.77 us | 80.54 us | +7.7% |
| Fn Bind (small) | 100K | 738.69 us | 800.31 us | +8.3% |
| Fn Bind (small) | 1M | 7.56 ms | 8.61 ms | +14.0% |
| Fn Bind (large) | 10K | 2.76 ms | 2.91 ms | +5.6% |
| Fn Bind (large) | 100K | 27.88 ms | 29.77 ms | +6.8% |
| Fn Bind (large) | 1M | 276.36 ms | 290.98 ms | +5.3% |

## Bind

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Bind (small) | 10K | 68.31 us |
| MoveOnlyFn Bind (small) | 100K | 711.08 us |
| MoveOnlyFn Bind (small) | 1M | 7.42 ms |
| MoveOnlyFn Bind (large) | 10K | 2.81 ms |
| MoveOnlyFn Bind (large) | 100K | 27.64 ms |
| MoveOnlyFn Bind (large) | 1M | 278.20 ms |

## Bind

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Bind | 10K | 12.38 us |
| FnRef Bind | 100K | 123.15 us |
| FnRef Bind | 1M | 1.23 ms |

## Reset

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::reset() | 10K | 2.71 ms | 37.23 us | -98.6% |
| Fn::reset() | 100K | 27.85 ms | 369.54 us | -98.7% |
| Fn::reset() | 1M | 276.07 ms | 4.41 ms | -98.4% |

## Reset

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::reset() | 10K | 2.91 ms |
| MoveOnlyFn::reset() | 100K | 27.04 ms |
| MoveOnlyFn::reset() | 1M | 268.95 ms |

## Construct Destroy

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| Fn Construct/destroy (empty) | 10K | 30.85 us | 37.23 us | +20.7% |
| Fn Construct/destroy (empty) | 100K | 307.77 us | 369.31 us | +20.0% |
| Fn Construct/destroy (empty) | 1M | 3.12 ms | 3.73 ms | +19.4% |

## Construct Destroy

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Construct/destroy (empty) | 10K | 31.00 us |
| MoveOnlyFn Construct/destroy (empty) | 100K | 307.92 us |
| MoveOnlyFn Construct/destroy (empty) | 1M | 3.14 ms |

## Construct Destroy

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Construct/destroy (empty) | 10K | 12.38 us |
| FnRef Construct/destroy (empty) | 100K | 123.15 us |
| FnRef Construct/destroy (empty) | 1M | 1.23 ms |

## Copy Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Copy Construct (small) | 10K | 118.08 us | 130.08 us | +10.2% |
| Fn Copy Construct (small) | 100K | 1.24 ms | 1.29 ms | +4.6% |
| Fn Copy Construct (small) | 1M | 11.94 ms | 13.04 ms | +9.2% |
| Fn Copy Construct (large) | 10K | 2.99 ms | 3.03 ms | +1.5% |
| Fn Copy Construct (large) | 100K | 29.59 ms | 30.41 ms | +2.8% |
| Fn Copy Construct (large) | 1M | 299.39 ms | 308.47 ms | +3.0% |

## Copy Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Copy Construct | 10K | 12.62 us |
| FnRef Copy Construct | 100K | 123.15 us |
| FnRef Copy Construct | 1M | 1.23 ms |

## Move Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Move Construct (small) | 10K | 74.54 us | 160.62 us | +115.5% |
| Fn Move Construct (small) | 100K | 809.00 us | 2.03 ms | +150.6% |
| Fn Move Construct (small) | 1M | 7.28 ms | 16.19 ms | +122.5% |
| Fn Move Construct (large) | 10K | 3.18 ms | 2.84 ms | -10.7% |
| Fn Move Construct (large) | 100K | 28.08 ms | 28.66 ms | +2.1% |
| Fn Move Construct (large) | 1M | 284.62 ms | 290.93 ms | +2.2% |

## Move Semantics

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Move Construct (small) | 10K | 75.08 us |
| MoveOnlyFn Move Construct (small) | 100K | 738.77 us |
| MoveOnlyFn Move Construct (small) | 1M | 7.95 ms |
| MoveOnlyFn Move Construct (large) | 10K | 2.78 ms |
| MoveOnlyFn Move Construct (large) | 100K | 28.35 ms |
| MoveOnlyFn Move Construct (large) | 1M | 280.15 ms |

## Move Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Move Construct | 10K | 12.38 us |
| FnRef Move Construct | 100K | 123.08 us |
| FnRef Move Construct | 1M | 1.23 ms |

## Capture Size

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Bind (0B) | 10K | 68.31 us | 74.54 us | +9.1% |
| Fn Bind (0B) | 100K | 745.92 us | 739.08 us | -0.9% |
| Fn Bind (0B) | 1M | 6.85 ms | 7.48 ms | +9.2% |
| Fn Bind (16B) | 10K | 74.23 us | 92.92 us | +25.2% |
| Fn Bind (16B) | 100K | 788.00 us | 923.38 us | +17.2% |
| Fn Bind (16B) | 1M | 7.49 ms | 9.32 ms | +24.4% |
| Fn Bind (40B, SBO Boundary) | 10K | 86.77 us | 2.82 ms | +3151.2% |
| Fn Bind (40B, SBO Boundary) | 100K | 1.11 ms | 28.02 ms | +2424.4% |
| Fn Bind (40B, SBO Boundary) | 1M | 11.26 ms | 278.92 ms | +2377.7% |
| Fn Bind (64B, Heap) | 10K | 2.69 ms | 2.87 ms | +6.7% |
| Fn Bind (64B, Heap) | 100K | 27.70 ms | 28.43 ms | +2.6% |
| Fn Bind (64B, Heap) | 1M | 278.35 ms | 286.12 ms | +2.8% |
| Fn Bind (256B, Heap) | 10K | 3.18 ms | 3.44 ms | +8.2% |
| Fn Bind (256B, Heap) | 100K | 32.36 ms | 34.80 ms | +7.6% |
| Fn Bind (256B, Heap) | 1M | 322.91 ms | 342.93 ms | +6.2% |

## Invoke By Size

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Invoke (0B) | 10K | 43.69 us | 56.00 us | +28.2% |
| Fn Invoke (0B) | 100K | 556.62 us | 554.46 us | -0.4% |
| Fn Invoke (0B) | 1M | 4.38 ms | 5.59 ms | +27.4% |
| Fn Invoke (16B) | 10K | 43.38 us | 55.92 us | +28.9% |
| Fn Invoke (16B) | 100K | 431.08 us | 602.46 us | +39.8% |
| Fn Invoke (16B) | 1M | 4.35 ms | 5.57 ms | +28.0% |
| Fn Invoke (40B, SBO Boundary) | 10K | 43.46 us | 37.54 us | -13.6% |
| Fn Invoke (40B, SBO Boundary) | 100K | 431.00 us | 369.92 us | -14.2% |
| Fn Invoke (40B, SBO Boundary) | 1M | 4.41 ms | 3.69 ms | -16.3% |
| Fn Invoke (64B, Heap) | 10K | 43.69 us | 37.31 us | -14.6% |
| Fn Invoke (64B, Heap) | 100K | 431.31 us | 369.62 us | -14.3% |
| Fn Invoke (64B, Heap) | 1M | 4.37 ms | 3.73 ms | -14.6% |
| Fn Invoke (256B, Heap) | 10K | 43.62 us | 37.31 us | -14.5% |
| Fn Invoke (256B, Heap) | 100K | 431.15 us | 369.54 us | -14.3% |
| Fn Invoke (256B, Heap) | 1M | 4.36 ms | 3.73 ms | -14.4% |

## Null Compare

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::operator==(nullptr) | 10K | 24.92 us | 25.15 us | +0.9% |
| Fn::operator==(nullptr) | 100K | 246.46 us | 246.15 us | -0.1% |
| Fn::operator==(nullptr) | 1M | 2.52 ms | 2.52 ms | +0.2% |

## Null Compare

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::operator==(nullptr) | 10K | 12.54 us |
| MoveOnlyFn::operator==(nullptr) | 100K | 246.23 us |
| MoveOnlyFn::operator==(nullptr) | 1M | 2.46 ms |

## Null Compare

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef::operator==(nullptr) | 10K | 6.23 us |
| FnRef::operator==(nullptr) | 100K | 61.62 us |
| FnRef::operator==(nullptr) | 1M | 1.28 ms |

## Swap

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::swap() | 10K | 154.69 us | 371.62 us | +140.2% |
| Fn::swap() | 100K | 1.72 ms | 3.53 ms | +105.1% |
| Fn::swap() | 1M | 13.55 ms | 35.15 ms | +159.5% |

## Swap

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::swap() | 10K | 145.31 us |
| MoveOnlyFn::swap() | 100K | 1.60 ms |
| MoveOnlyFn::swap() | 1M | 15.73 ms |
