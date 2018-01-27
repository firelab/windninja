package main

import (
	"testing"
	"time"
)

func TestLandfireGet(t *testing.T) {
	err := Get(Envelope{-114, 45, -113.9, 45.1}, time.Time{}, time.Time{}, "test.lcp")
	if err != nil {
		t.Error(err)
	}
}
