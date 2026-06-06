package gateway

import (
	"context"
	"log/slog"
	"net/http"
	"os"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/grpcserver"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
)

func (g *Gateway) startStorageGRPC(ctx context.Context, deps handlers.Dependencies) error {
	if g.opts.StorageGRPCAddress == "" {
		return nil
	}
	grpcSrv, err := grpcserver.Start(g.opts.StorageGRPCAddress, g.engineClient, deps)
	if err != nil {
		return err
	}
	g.storageGRPC = grpcSrv
	go func() {
		if serveErr := grpcSrv.Serve(); serveErr != nil {
			g.logger.WarnContext(ctx, "storage grpc server exited", slog.Any("err", serveErr))
		}
	}()
	g.logger.InfoContext(ctx, "storage grpc listening",
		slog.String("addr", g.opts.StorageGRPCAddress))
	return nil
}

func (g *Gateway) waitForShutdown(
	ctx context.Context,
	srv *http.Server,
	errCh <-chan error,
	sigCh <-chan os.Signal,
) error {
	select {
	case err := <-errCh:
		g.stopStorageGRPC()
		g.stopEngine()
		return err
	case sig := <-sigCh:
		g.logger.InfoContext(ctx, "shutting down on signal",
			slog.String("signal", sig.String()))
		shutdownCtx, cancel := context.WithTimeout(ctx, 5*time.Second)
		defer cancel()
		_ = srv.Shutdown(shutdownCtx)
		g.stopStorageGRPC()
		g.stopEngine()
		return nil
	}
}

func (g *Gateway) logStartupExpectations(ctx context.Context) {
	g.logger.InfoContext(ctx, "gateway listening",
		slog.String("addr", g.opts.HTTPAddress))
	switch {
	case g.opts.EngineBinary != "":
		g.logger.InfoContext(ctx, "engine grpc expected",
			slog.String("addr", g.opts.EngineAddress))
		if g.opts.StorageGRPCAddress != "" {
			g.logger.InfoContext(ctx, "public storage grpc expected",
				slog.String("addr", g.opts.StorageGRPCAddress))
		}
	default:
		g.logger.InfoContext(ctx, "engine subprocess disabled; query routes will return Unimplemented")
	}
}
