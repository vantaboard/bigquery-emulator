package server

import (
	"context"
	"github.com/vantaboard/bigquery-emulatorlator/internal/connection"
	"github.com/vantaboard/bigquery-emulatorlator/types"
)

func (s *Server) addProjects(ctx context.Context, projects []*types.Project) error {
	for _, project := range projects {
		if err := s.addProject(ctx, project); err != nil {
			return err
		}
	}
	return nil
}

func (s *Server) addProject(ctx context.Context, project *types.Project) error {
	err := s.connMgr.ExecuteWithTransaction(ctx, func(ctx context.Context, tx *connection.Tx) error {
		tx.SetProjectAndDataset(project.ID, "")
		for _, dataset := range project.Datasets {
			for _, table := range dataset.Tables {
				table.SetupMetadata(project.ID, dataset.ID)
				if err := s.addTableData(ctx, tx, project, dataset, table); err != nil {
					return err
				}
			}
		}
		p, _, _ := s.metaRepo.ProjectFromData(project)
		if err := s.metaRepo.AddProjectIfNotExists(ctx, tx.Tx(), p); err != nil {
			return err
		}
		for _, d := range project.Datasets {
			dataset, tables, _, _ := s.metaRepo.DatasetFromData(p.ID, d)
			if err := s.metaRepo.AddDataset(ctx, tx.Tx(), dataset); err != nil {
				return err
			}
			for _, table := range tables {
				if err := s.metaRepo.AddTable(ctx, tx.Tx(), table); err != nil {
					return err
				}
			}
		}
		return nil
	})
	if err != nil {
		return err
	}

	return nil
}

func (s *Server) addTableData(ctx context.Context, tx *connection.Tx, project *types.Project, dataset *types.Dataset, table *types.Table) error {
	if err := s.contentRepo.CreateOrReplaceTable(ctx, tx, project.ID, dataset.ID, table); err != nil {
		return err
	}
	if err := s.contentRepo.AddTableData(ctx, tx, project.ID, dataset.ID, table, false); err != nil {
		return err
	}
	return nil
}
