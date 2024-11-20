FROM alpine:3.18 as downloader

# renovate: datasource=github-tags depName=kubernetes/kubectl extractVersion=^kubernetes-(?<version>.*)$
ARG KUBECTL_VERSION="1.28.0"

RUN apk add --no-cache curl && \
  curl -LO "https://dl.k8s.io/release/${KUBECTL_VERSION}/bin/linux/amd64/kubectl" && \
  chmod +x kubectl

FROM gcr.io/distroless/static-debian12:nonroot
COPY --from=downloader /kubectl /usr/local/bin/kubectl

ENTRYPOINT ["/usr/local/bin/kubectl"]
