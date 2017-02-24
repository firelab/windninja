// Copyright 2017 Kyle Shannon.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"encoding/json"
	"fmt"
	"net/http"
)

const reportSQL = `
SELECT country, region, COUNT()
	FROM visit LEFT JOIN ip USING(ip)
	GROUP BY region
	ORDER BY COUNT() DESC`

func reportHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	rows, err := db.Query(reportSQL)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	var g geogData
	var gs []geogData
	for rows.Next() {
		rows.Scan(
			&g.Country,
			&g.Region,
			&g.Visits)
		gs = append(gs, g)
	}
	if r.FormValue("fmt") == "json" {
		var j []byte
		if p := r.FormValue("pretty"); p == "true" || p == "1" {
			j, err = json.MarshalIndent(gs, "", "  ")
		} else {
			j, err = json.Marshal(gs)
		}
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Type", "application/json; charset=UTF-8")
		fmt.Fprintf(w, string(j))
	} else {
		w.Header().Set("Content-Type", "text/html; charset=UTF-8")
		err = templates.ExecuteTemplate(w, "geogreport", gs)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
	}
}
