// Copyright 2018 Kyle Shannon.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"bufio"
	"context"
	"crypto/tls"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"math/rand"
	"net/http"
	"net/url"
	"os"
	"path"
	"strings"
	"sync"
	"time"

	"crawshaw.io/sqlite"
	"github.com/google/go-github/github"
	"golang.org/x/crypto/acme/autocert"
)

const (
	message     = "hello, world"
	threddsFile = "thredds.csv"
)

var (
	pool    *sqlite.Pool
	gh      *github.Client
	mu      sync.RWMutex
	version string
)

func current() error {
	r, _, err := gh.Repositories.GetLatestRelease(context.TODO(), "firelab", "windninja")
	if err != nil {
		return err
	}
	mu.Lock()
	version = r.GetTagName()
	mu.Unlock()
	return nil
}

func versionHandler(w http.ResponseWriter, r *http.Request) {
	s := "VERSION="
	mu.RLock()
	s += version
	mu.RUnlock()
	if message != "" {
		s += ";MESSAGE=" + message
	}
	fmt.Fprintf(w, s)
}

var mapKeys []string

func mapkeyHandler(w http.ResponseWriter, r *http.Request) {
	if mapKeys == nil || r.Header.Get("Cache-Control") == "no-cache" {
		mapKeys = []string{}
		fin, err := os.Open("./map_keys")
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		defer fin.Close()
		s := bufio.NewScanner(fin)
		for s.Scan() {
			t := strings.TrimSpace(s.Text())
			if t != "" {
				mapKeys = append(mapKeys, t)
			}
		}
	}
	if len(mapKeys) == 0 {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	n := rand.Intn(len(mapKeys))
	err := json.NewEncoder(w).Encode(struct {
		Key string `json:"key"`
	}{mapKeys[n]})
	if err != nil {
		log.Print(err)
	}
}

func visitHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	db := pool.Get(r.Context().Done())
	defer pool.Put(db)
	if v := r.FormValue("thredds"); v != "" {
		fi, err := os.Stat(threddsFile)
		if os.IsNotExist(err) {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		fin, err := os.Open(threddsFile)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Length", fmt.Sprintf("%d", fi.Size()))
		w.Header().Set("Content-Type", "text/csv")
		w.Header().Set("Content-Dispostion", "attachment")
		io.Copy(w, fin)
		fin.Close()
	}
	ip := strings.Split(r.RemoteAddr, ":")[0]
	log.Printf("visit from ip: %s", ip)
	stmt := db.Prep("INSERT INTO visit(timestamp, ip) VALUES(datetime('now'), ?)")
	stmt.BindText(1, ip)
	_, err = stmt.Step()
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	// Call the geoip server asynchronously
	// TODO(kyle): replace with vendored freegeoip package.
	go func() {
		log.Printf("fetching data for ip: %s", ip)
		stmt := db.Prep("SELECT COUNT() FROM ip WHERE ip=?")
		stmt.BindText(1, ip)
		hasRow, err := stmt.Step()
		if err != nil || hasRow == false {
			log.Print(err)
			return
		}
		n := stmt.ColumnInt64(0)
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
		var ms map[string]string
		for k, v := range m {
			if s, ok := v.(string); ok {
				ms[k] = s
			} else {
				log.Print("invalid type")
			}
		}

		stmt = db.Prep("INSERT INTO ip VALUES(?,?,?,?,?,?)")
		stmt.BindText(1, ms["ip"])
		stmt.BindText(2, ms["country_code"])
		stmt.BindText(3, ms["region_name"])
		stmt.BindText(4, ms["city"])
		stmt.BindText(5, ms["longitude"])
		stmt.BindText(6, ms["latitude"])
		_, err = stmt.Step()
		if err != nil {
			log.Print(err)
		}
	}()
}

func main() {
	flagAddr := flag.String("addr", ":https", "address to listen on (:8888)")
	flagDB := flag.String("db", "", "database file")
	flag.Parse()

	var err error
	pool, err = sqlite.Open(*flagDB, 0, 16)
	if err != nil {
		log.Fatal(err)
	}

	gh = github.NewClient(nil)

	current()
	go func() {
		t := time.NewTicker(time.Hour)
		for {
			select {
			case <-t.C:
				current()
			}
		}
	}()

	mux := &http.ServeMux{}
	mux.HandleFunc("/cgi-bin/ninjavisit", visitHandler)
	mux.HandleFunc("/version/", versionHandler)
	mux.HandleFunc("/mapkey/", mapkeyHandler)
	srv := &http.Server{
		Addr:         *flagAddr,
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 10 * time.Second,
		IdleTimeout:  120 * time.Second,
		Handler:      mux,
	}
	https := map[string]struct{}{":443": struct{}{}, ":8443": struct{}{}, ":https": struct{}{}}
	if _, ok := https[*flagAddr]; !ok {
		log.Fatal(srv.ListenAndServe())
	} else {
		m := &autocert.Manager{
			Cache:      autocert.DirCache("/opt/acme/"),
			Prompt:     autocert.AcceptTOS,
			HostPolicy: autocert.HostWhitelist("windninja.org"),
		}
		go func() {
			log.Fatal(http.ListenAndServe(":http", m.HTTPHandler(nil)))
		}()
		srv.TLSConfig = &tls.Config{GetCertificate: m.GetCertificate}
		log.Fatal(srv.ListenAndServeTLS("", ""))
	}
}
