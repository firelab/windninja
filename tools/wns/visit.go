// Copyright 2016 Boise State University.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"net/url"
	"os"
	"path"
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
	log.Printf("visit from ip: %s", ip)
	_, err = db.Exec("INSERT INTO visit(timestamp, ip) VALUES(datetime('now'), ?)", ip)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	// Call the geoip server asynchronously
	go func() {
		log.Printf("fetching data for ip: %s", ip)
		row := db.QueryRow("SELECT COUNT() FROM ip WHERE ip=?", ip)
		if err != nil {
			log.Println(err)
			return
		}
		var n int
		err = row.Scan(&n)
		if err != nil {
			log.Println(err)
			return
		}
		if n > 0 {
			// Already logged in our system
			return
		}
		u := url.URL{
			Scheme: "https",
			Host:   "freegeoip.net",
			Path:   "json",
		}
		u.Path = path.Join(u.Path, ip)
		log.Printf("fetching %s", u.String())
		resp, err := http.Get(u.String())
		if err != nil {
			log.Println(err)
			return
		}
		defer resp.Body.Close()
		if resp.StatusCode != http.StatusOK {
			log.Printf("response from geoip: %s", http.StatusText(resp.StatusCode))
			return
		}
		var m map[string]interface{}
		if err := json.NewDecoder(resp.Body).Decode(&m); err != nil {
			log.Println(err)
			return
		}
		_, err = db.Exec("INSERT INTO ip VALUES(?,?,?,?,?,?)",
			m["ip"], m["country_code"], m["region_name"], m["city"],
			m["longitude"], m["latitude"])
		if err != nil {
			log.Println(err)
		}
	}()
}
