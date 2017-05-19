// Copyright 2017 Kyle Shannon.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

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

var db *sql.DB

const (
	dbFile = "./ninjavisit.db"
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
	http.HandleFunc("/user", userReport)

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
	// srv.ListenAndServeTLS()
}
