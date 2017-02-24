// Copyright 2016 Boise State University.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"encoding/json"
	"fmt"
	"html/template"
	"net/http"
)

func reportHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	rows, err := db.Query(`SELECT country, region, COUNT()
														FROM visit LEFT JOIN ip USING(ip)
														GROUP BY region
														ORDER BY COUNT() DESC`)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
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
		//if err := json.NewEncoder(w).Encode(gs); err != nil {
		//panic(err)
		//}
	} else {
		t, err := template.New("geog").Parse(geogTemplate)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Type", "text/html; charset=UTF-8")
		err = t.Execute(w, gs)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
	}
}
