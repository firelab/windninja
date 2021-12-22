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
	"io/ioutil"
	"log"
	"math/rand"
	"net/http"
	"net/url"
	"os"
	"strings"
	"sync"
	"time"

	"crawshaw.io/sqlite"
	"github.com/google/go-github/github"
	"golang.org/x/crypto/acme/autocert"
)

type ipStackResp struct {
	City          string  `json:"city"`
	ContinentCode string  `json:"continent_code"`
	ContinentName string  `json:"continent_name"`
	CountryCode   string  `json:"country_code"`
	CountryName   string  `json:"country_name"`
	IP            string  `json:"ip"`
	Latitude      float64 `json:"latitude"`
	Location      struct {
		CallingCode             string `json:"calling_code"`
		Capital                 string `json:"capital"`
		CountryFlag             string `json:"country_flag"`
		CountryFlagEmoji        string `json:"country_flag_emoji"`
		CountryFlagEmojiUnicode string `json:"country_flag_emoji_unicode"`
		GeonameID               int64  `json:"geoname_id"`
		IsEu                    bool   `json:"is_eu"`
		Languages               []struct {
			Code   string `json:"code"`
			Name   string `json:"name"`
			Native string `json:"native"`
		} `json:"languages"`
	} `json:"location"`
	Longitude  float64 `json:"longitude"`
	RegionCode string  `json:"region_code"`
	RegionName string  `json:"region_name"`
	Type       string  `json:"type"`
	Zip        string  `json:"zip"`
}

const (
	message     = ""
	threddsFile = "thredds.csv"
)

var (
	pool         *sqlite.Pool
	gh           *github.Client
	mu           sync.RWMutex
	version      string
	ipstackToken = ""
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
		http.Error(w, "no keys", http.StatusInternalServerError)
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
	stmt := db.Prep("SELECT COUNT() FROM visit WHERE ip=?")
	stmt.BindText(1, ip)
	_, err = stmt.Step()
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	if stmt.ColumnInt64(0) < 0 {
		w.WriteHeader(200)
		return
	}
	stmt.Finalize()
	stmt = db.Prep("INSERT INTO visit(timestamp, ip) VALUES(datetime('now'), ?)")
	stmt.BindText(1, ip)
	_, err = stmt.Step()
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	// Call the geoip server asynchronously
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
			Scheme: "http",
			Host:   "api.ipstack.com",
			Path:   ip,
		}
		if ipstackToken != "" {
			q := url.Values{}
			q.Set("format", "1")
			q.Set("access_key", ipstackToken)
			u.RawQuery = q.Encode()
			log.Print(u.String())
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
			var ips ipStackResp
			if err := json.NewDecoder(resp.Body).Decode(&ips); err != nil {
				log.Println(err)
				return
			}
			stmt = db.Prep("INSERT INTO ip VALUES(?,?,?,?,?,?)")
			stmt.BindText(1, ips.IP)
			stmt.BindText(2, ips.CountryCode)
			stmt.BindText(3, ips.RegionName)
			stmt.BindText(4, ips.City)
			stmt.BindFloat(5, ips.Longitude)
			stmt.BindFloat(6, ips.Longitude)
			_, err = stmt.Step()
			if err != nil {
				log.Print(err)
			}
		}
	}()
}

func reportHandler(w http.ResponseWriter, r *http.Request) {
	db := pool.Get(r.Context().Done())
	stmt := db.Prep(`SELECT COUNT() AS c, city, region, country
		FROM visit JOIN ip USING(ip) GROUP BY city ORDER BY c DESC`)
	type usage struct {
		City    string
		Region  string
		Country string
		Count   int64
	}
	var u usage
	var us []usage
	for {
		if hasRow, err := stmt.Step(); err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		} else if !hasRow {
			break
		}
		u = usage{}
		u.City = stmt.ColumnText(1)
		u.Region = stmt.ColumnText(2)
		u.Country = stmt.ColumnText(3)
		u.Count = stmt.ColumnInt64(0)
		us = append(us, u)
	}
	w.Header().Set("Content-Type", "application/json")
	err := json.NewEncoder(w).Encode(us)
	if err != nil {
		log.Print(err)
	}
}

func main() {
	log.SetFlags(log.LstdFlags | log.Lshortfile)
	flagAddr := flag.String("addr", ":https", "address to listen on (:8888)")
	flagDB := flag.String("db", "", "database file")
	flagDisableIPLookup := flag.Bool("iplookup", false, "lookup ip region")
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

	buf, err := ioutil.ReadFile("ipstack.txt")
	if err != nil {
		log.Fatal(err)
	}
	ipstackToken = strings.TrimSpace(string(buf))
	log.Printf("ipstack token: %s", ipstackToken)
	if *flagDisableIPLookup {
		ipstackToken = ""
	}

	mux := &http.ServeMux{}
	mux.HandleFunc("/cgi-bin/ninjavisit", visitHandler)
	mux.HandleFunc("/version/", versionHandler)
	mux.HandleFunc("/mapkey/", mapkeyHandler)
	mux.HandleFunc("/report/", reportHandler)
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
