package bqreservation

import (
	"context"

	"cloud.google.com/go/bigquery/reservation/apiv1/reservationpb"
	"google.golang.org/grpc"
)

// Server implements the shallow BigQuery Reservation API gRPC surface.
type Server struct {
	reservationpb.UnimplementedReservationServiceServer
}

// RegisterGRPC wires ReservationService onto srv.
func RegisterGRPC(srv grpc.ServiceRegistrar) {
	if srv == nil {
		return
	}
	reservationpb.RegisterReservationServiceServer(srv, &Server{})
}

// ListCapacityCommitments returns an empty page.
func (s *Server) ListCapacityCommitments(
	_ context.Context,
	_ *reservationpb.ListCapacityCommitmentsRequest,
) (*reservationpb.ListCapacityCommitmentsResponse, error) {
	return &reservationpb.ListCapacityCommitmentsResponse{
		CapacityCommitments: []*reservationpb.CapacityCommitment{},
	}, nil
}

// ListReservations returns an empty page.
func (s *Server) ListReservations(
	_ context.Context,
	_ *reservationpb.ListReservationsRequest,
) (*reservationpb.ListReservationsResponse, error) {
	return &reservationpb.ListReservationsResponse{
		Reservations: []*reservationpb.Reservation{},
	}, nil
}
