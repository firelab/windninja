package main

import (
	"database/sql"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"

	_ "github.com/mattn/go-sqlite3"
)

type geogData struct {
	Country string `json:"country"`
	Region  string `json:"region"`
	Visits  int    `json:"count"`
}

var db *sql.DB

const (
	//dbFile      = "/srv/www/marblerye.org/data/ninjavisit.db"
	dbFile      = "./ninjavisit.db"
	threddsFile = "/srv/www/marblerye.org/data/thredds.csv"
)

func main() {
	log.SetFlags(log.LstdFlags | log.Lshortfile)
	var err error
	dbPath, err := filepath.Abs(dbFile)
	if err != nil {
		panic(err)
	}
	dbURI := fmt.Sprintf("file://%s?cache=shared&mode=rw&_busy_timeout=60000", dbPath)
	log.Printf("opening db file %s", dbURI)
	db, err = sql.Open("sqlite3", dbURI)
	if err != nil {
		panic(err)
	}
	db.Exec("PRAGMA journal_mode=WAL")
	srv := http.Server{
		Addr: ":34333",
	}
	http.HandleFunc("/map", mapHandler)
	http.HandleFunc("/report", reportHandler)
	http.HandleFunc("/visit", visitHandler)

	quit := make(chan os.Signal)
	signal.Notify(quit, os.Interrupt)
	go func() {
		<-quit
		log.Println("Shutting down server...")
		err := db.Close()
		if err != nil {
			log.Println(err)
		}
		os.Exit(0)
	}()
	srv.ListenAndServe()
}
