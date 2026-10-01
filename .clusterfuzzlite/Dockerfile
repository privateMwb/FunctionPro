# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:eb77f5af37831bde58f992e15783ff23f04ee81af67962b7e0aaa7e1e26a7662

COPY . $SRC/FunctionPro
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FunctionPro
