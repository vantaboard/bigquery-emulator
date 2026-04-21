// Command bq-explorer-api serves the HTTP API used by bigquery-emulator-ui (React).
// Run beside the BigQuery emulator; configure BIGQUERY_EMULATOR_HOST and related env vars.
package main

import (
	"log"
	"os"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"
	"github.com/vantaboard/bigquery-emulator/internal/explorerapi"
)

func main() {
	_ = godotenv.Load()

	bqClient, err := explorerapi.NewBigQueryClient()
	if err != nil {
		log.Fatalf("Failed to create BigQuery client: %v", err)
	}
	defer bqClient.Close()

	router := gin.Default()
	router.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"*"},
		AllowMethods:     []string{"GET", "POST", "OPTIONS"},
		AllowHeaders:     []string{"Origin", "Content-Type"},
		AllowCredentials: true,
	}))

	api := router.Group("/api")
	{
		api.GET("/config", bqClient.GetConfig)
		api.GET("/projects", bqClient.GetProjects)
		api.POST("/emulator/projects", bqClient.CreateEmulatorProject)
		api.GET("/projects/:project_id/datasets", bqClient.GetDatasets)
		api.GET("/projects/:project_id/datasets/:dataset_id/tables", bqClient.GetTables)
		api.GET("/projects/:project_id/datasets/:dataset_id/tables/:table_id/schema", bqClient.GetTableSchema)
		api.POST("/query", bqClient.RunQuery)
	}

	port := os.Getenv("PORT")
	if port == "" {
		port = "8000"
	}
	log.Printf("bq-explorer-api listening on :%s\n", port)
	if err := router.Run(":" + port); err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}
}
