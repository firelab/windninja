// Copyright 2016 Boise State University.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"

	_ "github.com/mattn/go-sqlite3"
)

var db *sql.DB

const (
	dbFile      = "/srv/www/marblerye.org/data/ninjavisit.db"
	threddsFile = "/srv/www/marblerye.org/data/thredds.csv"
)

// Handle the phone home call from WindNinja.  Log the IP and if requested,
// return the thredds csv file
func visitHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	if t := r.FormValue("thredds"); t == "1" || t == "true" {
		// Read in thredds data for download
		st, err := os.Stat(threddsFile)
		if err != nil || st.Size() < 1 {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		fin, err := os.Open(threddsFile)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Length", fmt.Sprintf("%d", st.Size()))
		w.Header().Set("Content-Type", "text/csv")
		w.Header().Set("Content-Dispostion", "attachment")
		io.Copy(w, fin)
		fin.Close()
	}
	ip := r.RemoteAddr
	_, err = db.Exec("INSERT INTO visit(timestamp, ip) VALUES(datetime('now'), ?)", ip)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
	// Call the geoip server asynchronously
	go func(ip string) {
		rows, err := db.Query("SELECT COUNT() FROM ip WHERE ip=?", ip)
		if err == nil && rows.Next() {
			var n int
			rows.Scan(&n)
			if n < 1 {
				resp, err := http.Get(fmt.Sprintf("https://freegeoip.net/json/%s", ip))
				if err != nil {
					// log error
					fmt.Printf("%s", err)
					return
				}
				defer resp.Body.Close()
				dec := json.NewDecoder(resp.Body)
				var m map[string]interface{}
				err = dec.Decode(&m)
				if err == nil {
					_, err := db.Exec("INSERT INTO ip VALUES(?,?,?,?,?,?)",
						m["ip"], m["country_code"], m["region_name"], m["city"],
						m["longitude"], m["latitude"])
					fmt.Printf("%s", err)
				} else {
					fmt.Println(err.Error())
				}
			}
		}
	}(ip)
}
