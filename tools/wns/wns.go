package main

import (
	"database/sql"
	"fmt"
	"net/http"
)

type geogData struct {
	Country string `json:"country"`
	Region  string `json:"region"`
	Visits  int    `json:"count"`
}

func main() {
	var err error
	db, err = sql.Open("sqlite3", fmt.Sprintf("file://%s?cache=shared&mode=rw&_busy_timeout=60000", dbFile))
	if err != nil {
		panic(err.Error())
	}
	db.Exec("PRAGMA journal_mode=WAL")
	srv := http.Server{
		Address: ":34333",
	}
	http.HandleFunc("/map", mapHandler)
	http.HandleFunc("/report", reportHandler)
	http.HandleFunc("/visit", visitHandler)

	http.ListenAndServe(":34333", router)
}
