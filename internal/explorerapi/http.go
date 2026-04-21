package explorerapi

import (
	"net/http"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
)

// APIPrefix is the URL path prefix for explorer UI routes merged into the emulator HTTP server.
const APIPrefix = "/api"

// NewHTTPHandler returns the Gin engine for explorer UI routes under /api and a close function for the BigQuery client.
func NewHTTPHandler() (http.Handler, func() error, error) {
	bq, err := NewBigQueryClient()
	if err != nil {
		return nil, nil, err
	}

	gin.SetMode(gin.ReleaseMode)
	router := gin.New()
	router.Use(gin.Recovery())
	router.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"*"},
		AllowMethods:     []string{"GET", "POST", "OPTIONS"},
		AllowHeaders:     []string{"Origin", "Content-Type"},
		AllowCredentials: true,
	}))

	api := router.Group(APIPrefix)
	{
		api.GET("/config", bq.GetConfig)
		api.GET("/projects", bq.GetProjects)
		api.POST("/emulator/projects", bq.CreateEmulatorProject)
		api.GET("/projects/:project_id/datasets", bq.GetDatasets)
		api.GET("/projects/:project_id/datasets/:dataset_id/tables", bq.GetTables)
		api.GET("/projects/:project_id/datasets/:dataset_id/tables/:table_id/schema", bq.GetTableSchema)
		api.POST("/query", bq.RunQuery)
	}

	return router, bq.Close, nil
}
