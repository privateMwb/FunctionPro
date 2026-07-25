# FunctionProBenchmark Results

## Bool Check

| Test | Iteration | FunctionPro | std::function | Δ |
|---|---|---|---|---|
| Fn::operator Bool() | 10K | 14.15 us | 13.92 us | -1.6% |
| Fn::operator Bool() | 100K | 208.23 us | 208.00 us | -0.1% |
| Fn::operator Bool() | 1M | 2.03 ms | 1.96 ms | -3.6% |

## Bool Check

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::operator Bool() | 10K | 13.23 us |
| MoveOnlyFn::operator Bool() | 100K | 195.92 us |
| MoveOnlyFn::operator Bool() | 1M | 1.31 ms |

## Bool Check

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef::operator Bool() | 10K | 6.69 us |
| FnRef::operator Bool() | 100K | 65.46 us |
| FnRef::operator Bool() | 1M | 685.23 us |

## Invoke

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::operator() | 10K | 204.31 us | 83.00 us | -59.4% |
| Fn::operator() | 100K | 2.04 ms | 800.62 us | -60.7% |
| Fn::operator() | 1M | 20.00 ms | 7.89 ms | -60.6% |
| Fn::operator() (empty) | 10K | 287.17 ms | 247.66 ms | -13.8% |
| Fn::operator() (empty) | 100K | 3.18 s | 2.56 s | -19.6% |
| Fn::operator() (empty) | 1M | 55.65 s | 51.63 s | -7.2% |

## Invoke

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::operator() | 10K | 589.23 us |
| MoveOnlyFn::operator() | 100K | 5.54 ms |
| MoveOnlyFn::operator() | 1M | 54.65 ms |
| MoveOnlyFn::operator() (empty) | 10K | 741.28 ms |
| MoveOnlyFn::operator() (empty) | 100K | 6.72 s |
| MoveOnlyFn::operator() (empty) | 1M | 59.67 s |

## Invoke

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef::operator() | 10K | 43.77 us |
| FnRef::operator() | 100K | 431.31 us |
| FnRef::operator() | 1M | 4.37 ms |
| FnRef::operator() (empty) | 10K | 441.99 ms |
| FnRef::operator() (empty) | 100K | 4.48 s |
| FnRef::operator() (empty) | 1M | 45.45 s |
| FnRef::operator() (function Pointer) | 10K | 81.61 us |
| FnRef::operator() (function Pointer) | 100K | 800.92 us |
| FnRef::operator() (function Pointer) | 1M | 8.11 ms |

## Assign

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Copy-assign | 10K | 974.54 us | 2.14 ms | +119.5% |
| Fn Copy-assign | 100K | 8.79 ms | 20.90 ms | +137.7% |
| Fn Copy-assign | 1M | 87.96 ms | 208.83 ms | +137.4% |
| Fn Move-assign | 10K | 377.85 us | 547.08 us | +44.8% |
| Fn Move-assign | 100K | 3.81 ms | 5.51 ms | +44.8% |
| Fn Move-assign | 1M | 37.87 ms | 54.83 ms | +44.8% |

## Assign

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Move-assign | 10K | 378.46 us |
| MoveOnlyFn Move-assign | 100K | 4.28 ms |
| MoveOnlyFn Move-assign | 1M | 42.29 ms |

## Assign

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Assign | 10K | 12.69 us |
| FnRef Assign | 100K | 61.85 us |
| FnRef Assign | 1M | 615.69 us |

## Bind

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Bind (small) | 10K | 173.62 us | 185.77 us | +7.0% |
| Fn Bind (small) | 100K | 1.77 ms | 1.85 ms | +4.1% |
| Fn Bind (small) | 1M | 17.41 ms | 21.07 ms | +21.0% |
| Fn Bind (large) | 10K | 15.91 ms | 22.63 ms | +42.3% |
| Fn Bind (large) | 100K | 180.34 ms | 205.30 ms | +13.8% |
| Fn Bind (large) | 1M | 1.87 s | 1.98 s | +6.0% |

## Bind

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Bind (small) | 10K | 179.69 us |
| MoveOnlyFn Bind (small) | 100K | 1.86 ms |
| MoveOnlyFn Bind (small) | 1M | 17.25 ms |
| MoveOnlyFn Bind (large) | 10K | 15.14 ms |
| MoveOnlyFn Bind (large) | 100K | 182.88 ms |
| MoveOnlyFn Bind (large) | 1M | 1.86 s |

## Bind

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Bind | 10K | 37.31 us |
| FnRef Bind | 100K | 369.54 us |
| FnRef Bind | 1M | 3.77 ms |

## Reset

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::reset() | 10K | 22.66 ms | 16.08 ms | -29.0% |
| Fn::reset() | 100K | 196.54 ms | 198.67 ms | +1.1% |
| Fn::reset() | 1M | 1.98 s | 2.02 s | +1.8% |

## Reset

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::reset() | 10K | 17.80 ms |
| MoveOnlyFn::reset() | 100K | 197.86 ms |
| MoveOnlyFn::reset() | 1M | 2.03 s |

## Construct Destroy

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| Fn Construct/destroy (empty) | 10K | 389.15 us | 56.08 us | -85.6% |
| Fn Construct/destroy (empty) | 100K | 4.01 ms | 680.69 us | -83.0% |
| Fn Construct/destroy (empty) | 1M | 41.14 ms | 5.63 ms | -86.3% |

## Construct Destroy

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Construct/destroy (empty) | 10K | 416.62 us |
| MoveOnlyFn Construct/destroy (empty) | 100K | 4.01 ms |
| MoveOnlyFn Construct/destroy (empty) | 1M | 40.26 ms |

## Construct Destroy

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Construct/destroy (empty) | 10K | 367.08 us |
| FnRef Construct/destroy (empty) | 100K | 3.75 ms |
| FnRef Construct/destroy (empty) | 1M | 38.28 ms |

## Copy Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Copy Construct (small) | 10K | 1.64 ms | 1.35 ms | -17.6% |
| Fn Copy Construct (small) | 100K | 16.03 ms | 12.22 ms | -23.8% |
| Fn Copy Construct (small) | 1M | 165.25 ms | 121.54 ms | -26.4% |
| Fn Copy Construct (large) | 10K | 30.38 ms | 20.60 ms | -32.2% |
| Fn Copy Construct (large) | 100K | 249.79 ms | 240.39 ms | -3.8% |
| Fn Copy Construct (large) | 1M | 2.19 s | 2.11 s | -3.4% |

## Copy Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Copy Construct | 10K | 37.85 us |
| FnRef Copy Construct | 100K | 389.85 us |
| FnRef Copy Construct | 1M | 4.61 ms |

## Move Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Move Construct (small) | 10K | 1.80 ms | 1.97 ms | +9.2% |
| Fn Move Construct (small) | 100K | 17.18 ms | 17.36 ms | +1.0% |
| Fn Move Construct (small) | 1M | 135.96 ms | 162.67 ms | +19.6% |
| Fn Move Construct (large) | 10K | 26.63 ms | 18.71 ms | -29.8% |
| Fn Move Construct (large) | 100K | 233.61 ms | 247.78 ms | +6.1% |
| Fn Move Construct (large) | 1M | 2.21 s | 2.03 s | -8.2% |

## Move Semantics

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn Move Construct (small) | 10K | 1.33 ms |
| MoveOnlyFn Move Construct (small) | 100K | 13.50 ms |
| MoveOnlyFn Move Construct (small) | 1M | 133.90 ms |
| MoveOnlyFn Move Construct (large) | 10K | 16.93 ms |
| MoveOnlyFn Move Construct (large) | 100K | 201.20 ms |
| MoveOnlyFn Move Construct (large) | 1M | 2.00 s |

## Move Semantics

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef Move Construct | 10K | 37.23 us |
| FnRef Move Construct | 100K | 369.62 us |
| FnRef Move Construct | 1M | 3.77 ms |

## Capture Size

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Bind (0B) | 10K | 167.15 us | 179.08 us | +7.1% |
| Fn Bind (0B) | 100K | 2.08 ms | 1.79 ms | -14.2% |
| Fn Bind (0B) | 1M | 18.13 ms | 18.88 ms | +4.1% |
| Fn Bind (16B) | 10K | 1.20 ms | 785.15 us | -34.7% |
| Fn Bind (16B) | 100K | 5.61 ms | 5.46 ms | -2.6% |
| Fn Bind (16B) | 1M | 56.95 ms | 56.87 ms | -0.1% |
| Fn Bind (40B, SBO Boundary) | 10K | 617.00 us | 17.57 ms | +2747.3% |
| Fn Bind (40B, SBO Boundary) | 100K | 8.79 ms | 203.45 ms | +2213.3% |
| Fn Bind (40B, SBO Boundary) | 1M | 78.33 ms | 1.88 s | +2294.0% |
| Fn Bind (64B, Heap) | 10K | 18.54 ms | 32.35 ms | +74.5% |
| Fn Bind (64B, Heap) | 100K | 209.14 ms | 236.56 ms | +13.1% |
| Fn Bind (64B, Heap) | 1M | 1.83 s | 1.48 s | -19.1% |
| Fn Bind (256B, Heap) | 10K | 13.75 ms | 14.26 ms | +3.7% |
| Fn Bind (256B, Heap) | 100K | 109.78 ms | 124.60 ms | +13.5% |
| Fn Bind (256B, Heap) | 1M | 885.00 ms | 1.09 s | +23.4% |

## Invoke By Size

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn Invoke (0B) | 10K | 86.23 us | 37.31 us | -56.7% |
| Fn Invoke (0B) | 100K | 854.46 us | 368.85 us | -56.8% |
| Fn Invoke (0B) | 1M | 8.52 ms | 4.22 ms | -50.4% |
| Fn Invoke (16B) | 10K | 85.23 us | 44.00 us | -48.4% |
| Fn Invoke (16B) | 100K | 842.77 us | 445.00 us | -47.2% |
| Fn Invoke (16B) | 1M | 8.47 ms | 4.44 ms | -47.5% |
| Fn Invoke (40B, SBO Boundary) | 10K | 84.92 us | 38.69 us | -54.4% |
| Fn Invoke (40B, SBO Boundary) | 100K | 843.77 us | 381.54 us | -54.8% |
| Fn Invoke (40B, SBO Boundary) | 1M | 8.45 ms | 3.83 ms | -54.7% |
| Fn Invoke (64B, Heap) | 10K | 108.08 us | 43.69 us | -59.6% |
| Fn Invoke (64B, Heap) | 100K | 1.05 ms | 431.69 us | -58.9% |
| Fn Invoke (64B, Heap) | 1M | 10.58 ms | 4.36 ms | -58.8% |
| Fn Invoke (256B, Heap) | 10K | 114.23 us | 37.23 us | -67.4% |
| Fn Invoke (256B, Heap) | 100K | 1.16 ms | 368.62 us | -68.1% |
| Fn Invoke (256B, Heap) | 1M | 11.52 ms | 3.70 ms | -67.9% |

## Null Compare

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::operator==(nullptr) | 10K | 15.77 us | 10.69 us | -32.2% |
| Fn::operator==(nullptr) | 100K | 105.31 us | 105.31 us | +0.0% |
| Fn::operator==(nullptr) | 1M | 1.05 ms | 1.10 ms | +4.1% |

## Null Compare

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::operator==(nullptr) | 10K | 10.54 us |
| MoveOnlyFn::operator==(nullptr) | 100K | 105.31 us |
| MoveOnlyFn::operator==(nullptr) | 1M | 1.05 ms |

## Null Compare

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| FnRef::operator==(nullptr) | 10K | 5.46 us |
| FnRef::operator==(nullptr) | 100K | 105.46 us |
| FnRef::operator==(nullptr) | 1M | 1.05 ms |

## Swap

| Test | Iteration | FunctionPro | std::function_ref | Δ |
|---|---|---|---|---|
| Fn::swap() | 10K | 312.00 us | 1.14 ms | +266.1% |
| Fn::swap() | 100K | 3.14 ms | 10.99 ms | +250.1% |
| Fn::swap() | 1M | 31.93 ms | 116.50 ms | +264.9% |

## Swap

| Test | Iteration | FunctionPro | std::move_only_function | Δ |
|---|---|---|---|---|
| MoveOnlyFn::swap() | 10K | 350.46 us |
| MoveOnlyFn::swap() | 100K | 3.23 ms |
| MoveOnlyFn::swap() | 1M | 35.46 ms |
