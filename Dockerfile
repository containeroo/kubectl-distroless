# Step 1: Build the wrapper
FROM alpine:3.20 as wrapper-builder

RUN apk add --no-cache gcc musl-dev make

COPY wrapper.c /src/wrapper.c

RUN gcc -o /wrapper /src/wrapper.c -static && chmod +x /wrapper

# Step 2: Download kubectl binary
FROM alpine:3.20 as downloader

# renovate: datasource=github-tags depName=kubernetes/kubectl extractVersion=^kubernetes-(?<version>.*)$
ARG KUBECTL_VERSION=1.31.2

RUN apk add --no-cache curl && \
  curl -LO "https://dl.k8s.io/release/v${KUBECTL_VERSION}/bin/linux/amd64/kubectl" && \
  chmod +x kubectl

# Step 3: Create the final minimal image
FROM gcr.io/distroless/static-debian12:nonroot

COPY --from=downloader /kubectl /usr/local/bin/kubectl
COPY --from=wrapper-builder /wrapper /usr/local/bin/wrapper

ENTRYPOINT ["/usr/local/bin/wrapper"]
CMD [ "kubectl" ]
