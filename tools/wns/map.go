// Copyright 2016 Boise State University.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import "net/http"

const mapSQL = `
SELECT country, COUNT()
	FROM visit LEFT JOIN ip USING(ip)
	WHERE country NOT NULL
	GROUP BY country
	ORDER BY country DESC`

func mapHandler(w http.ResponseWriter, r *http.Request) {
	rows, err := db.Query(mapSQL)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	var g geogData
	var gs []geogData
	for rows.Next() {
		rows.Scan(
			&g.Country,
			&g.Visits)
		if g.Country != "" && g.Visits > 0 {
			gs = append(gs, g)
		}
	}
	w.Header().Set("Content-Type", "text/html; charset=UTF-8")
	err = templates.ExecuteTemplate(w, "map", gs)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
}
