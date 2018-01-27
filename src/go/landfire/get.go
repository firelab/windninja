package main

import (
	"log"
	"time"
)

import "C"

type Envelope struct {
	MinX float64
	MinY float64
	MaxX float64
	MaxY float64
}

//export ninja_get
func ninja_get(source *C.char, minx, miny, maxx, maxy C.double, dst *C.char) int {

	e := Envelope{
		float64(minx),
		float64(miny),
		float64(maxx),
		float64(maxy),
	}
	t := time.Time{}
	err := Get(e, t, t, C.GoString(dst))
	//err := Get(Envelope{-114, 45, -113.9, 45.1}, time.Time{}, time.Time{}, "test.lcp")
	if err != nil {
		log.Fatal(err)
	}
	return 0
}

func main() {
}
